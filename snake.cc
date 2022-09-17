#include <ncurses.h>
#include <iostream>
#include <random>
#include <list>

using namespace std;

// Initial timeout
// timeout = DELAY - DECR * SPD
int const DELAY = 200;
int const MAX_SPD = 15;
int const DECR = 10;
// Appearance
char const SNAKE_BD = 'O';
char const SNAKE_HD = '@';
char const FOOD_CH = 'f';
char const HALVER_CH = '/';
char const DIET_CH = '0';
char const COKE_CH = '+';
char const WALL_CH = '#';
// Map size, including walls
int const MAP_LIN = 32;
int const MAP_COL = 62;
char ground[MAP_COL][MAP_LIN];
char scene[MAP_COL][MAP_LIN];

random_device dev;
mt19937 rng(dev());
uniform_int_distribution<std::mt19937::result_type> rand_col(1, MAP_COL - 2);
uniform_int_distribution<std::mt19937::result_type> rand_lin(1, MAP_LIN - 2);
uniform_int_distribution<std::mt19937::result_type> d100(1, 100);
uniform_int_distribution<std::mt19937::result_type> d10(1, 10);

void gen_food(WINDOW *win);

enum Dir {
	UP,
	DOWN,
	RIGHT,
	LEFT,
};

class Point {
public:
	int x;
	int y;

	Point(int x=0, int y=0) : x(x), y(y) {}

	Point operator+(Point p) {
		return Point(p.x + x, p.y + y);
	}

	bool operator==(Point p) {
		return p.x == x && p.y == y;
	}
};

list<Point> to_update;

Point const dirvec[4] {
	Point(0, -1),
	Point(0, 1),
	Point(1, 0),
	Point(-1, 0),
};

class Snake {
public:
	int speed;
	int food_n;
	int halver_n;
	int diet_n;
	int coke_n;
	int pts;
	bool dead;
	Dir dir;
	list<Point> seg;

	Snake() {
		speed = food_n = halver_n = diet_n = coke_n = pts = 0;
		dead = false;
		dir = RIGHT;
		seg = {
			Point(10, 10),
			Point(11, 10),
			Point(12, 10),
			Point(13, 10),
			Point(14, 10),
			Point(15, 10),
		};
	}

	void print() {
		for (auto i : seg) {
			scene[i.x][i.y] = SNAKE_BD;
			to_update.push_back(i);
		}
		Point head = seg.back();
		scene[head.x][head.y] = SNAKE_HD;
		to_update.push_back(head);
	}

	bool contains(Point p) {
		for (auto i : seg) {
			if (p == i)
				return true;
		}
		return false;
	}

	void add_speed(int n) {
		speed += n;
		if (speed > MAX_SPD) speed = MAX_SPD;
		if (speed < 0) speed = 0;
	}

	void update(char front) {
		Point old_head = seg.back();
		Point new_head = seg.back() + dirvec[dir];

		// Check collision with itself
		if (contains(new_head)) {
			dead = true;
			return;
		}

		// Check collision with walls
		if (front == '#') {
			dead = true;
			return;
		}

		seg.push_back(new_head);		

		if (front == FOOD_CH) {
			add_speed(1);
		} else {
			Point p = seg.front();
			scene[p.x][p.y] = '.';
			to_update.push_back(p);
			seg.pop_front();
		}
		scene[new_head.x][new_head.y] = SNAKE_HD;
		to_update.push_back(new_head);
		scene[old_head.x][old_head.y] = SNAKE_BD;
		to_update.push_back(old_head);
	}
};

Snake snake;

void gen_food(WINDOW *win)
{
	Point new_food;

	do {
		new_food = Point(rand_col(rng), rand_lin(rng));
	} while (snake.contains(new_food));

	mvwaddch(win, new_food.y, new_food.x, FOOD_CH | COLOR_PAIR(2));
}

void update_scene(WINDOW *win)
{
	for (auto i : to_update) {
		if (scene[i.x][i.y] == '.')
			mvwaddch(win, i.y, i.x, ground[i.x][i.y] | COLOR_PAIR(1));
		else
			mvwaddch(win, i.y, i.x, scene[i.x][i.y] | COLOR_PAIR(3));
	}
	to_update.clear();
}

int main()
{
	initscr();
	noecho();
	curs_set(0);
	start_color();
	keypad(stdscr, true);
	timeout(DELAY);

	WINDOW * map_win;
	WINDOW * main_win;
	main_win = newwin(MAP_LIN + 2, MAP_COL + 2, 0, 0);
	map_win = newwin(MAP_LIN, MAP_COL, 1, 1);

	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(3, COLOR_BLACK, COLOR_YELLOW);
	init_pair(4, COLOR_WHITE, COLOR_RED);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);

	box(main_win, 0, 0);
	mvwprintw(main_win, 0, 1, "[SNAKE]");

	bool running = true;
	while (running) {
		for (int i = 0; i < MAP_LIN ; ++i) {
			for (int j = 0; j < MAP_COL; ++j) {
				if (i == 0 || j == 0 || i == MAP_LIN - 1 || j == MAP_COL - 1) {
					ground[j][i] = scene[j][i]  = WALL_CH;
					mvwaddch(map_win, i, j, ground[j][i] | COLOR_PAIR(5));
					continue;
				}

				int roll = d10(rng);
				char tile = ' ';
				if (roll == 1)
					tile = '\'';
				else if (roll == 2)
					tile = '\"';
				else if (roll == 3)
					tile = '`';
				else if (roll == 4)
					tile = '.';
				ground[j][i] = scene[j][i] = tile;
				mvwaddch(map_win, i, j, ground[j][i] | COLOR_PAIR(1));
			}
		}

		snake = Snake();
		snake.print();
		gen_food(map_win);
		update_scene(map_win);

		refresh();
		wrefresh(main_win);
		wrefresh(map_win);

		bool paused = true;
		while(!snake.dead) {
			if (paused) {
				wattron(map_win, COLOR_PAIR(4));
				mvwprintw(map_win, MAP_LIN - 1, (MAP_COL - 2) / 2 - 4, " *PAUSED* ");
				wattroff(map_win, COLOR_PAIR(4));
				wrefresh(map_win);
				while (getch() != ' ') ;
				paused = false;
				wattron(map_win, COLOR_PAIR(5));
				mvwprintw(map_win, MAP_LIN - 1, (MAP_COL - 2) / 2 - 4, "##########");
				wattroff(map_win, COLOR_PAIR(5));
				wrefresh(map_win);
			}

			switch (getch()) {
				case KEY_UP:
				case 'w':
				case 'k':
					if (snake.dir != DOWN)
						snake.dir = UP;
					break;
				case KEY_DOWN:
				case 's':
				case 'j':
					if (snake.dir != UP)
						snake.dir = DOWN;
					break;
				case KEY_RIGHT:
				case 'd':
				case 'l':
					if (snake.dir != LEFT)
						snake.dir = RIGHT;
					break;
				case KEY_LEFT:
				case 'a':
				case 'h':
					if (snake.dir != RIGHT)
						snake.dir = LEFT;
					break;
				case ' ':
					paused = true;
					break;
				default:
					;
			}

			Point new_head = snake.seg.back() + dirvec[snake.dir];
			char front = mvwinch(map_win, new_head.y, new_head.x);
			snake.update(front);
			if (front == FOOD_CH) {
				gen_food(map_win);
			}
			update_scene(map_win);
			timeout(DELAY - DECR * snake.speed);
			wrefresh(map_win);
		}
	}

	endwin();

	return 0;
}