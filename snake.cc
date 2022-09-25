#include <ncurses.h>
#include <iostream>
#include <random>
#include <string>
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
// Window sizes
int const MAP_LIN = 32;
int const MAP_COL = 62;
int const MAIN_LIN = MAP_LIN + 4 + 4;
int const MAIN_COL = MAP_COL + 4 + 24;
// "Tiles"
char ground[MAP_COL][MAP_LIN];
char scene[MAP_COL][MAP_LIN];

random_device dev;
mt19937 rng(dev());
uniform_int_distribution<std::mt19937::result_type> rand_col(1, MAP_COL - 2);
uniform_int_distribution<std::mt19937::result_type> rand_lin(1, MAP_LIN - 2);
uniform_int_distribution<std::mt19937::result_type> d100(1, 100);
uniform_int_distribution<std::mt19937::result_type> d10(1, 10);

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

void gen_food(Snake snake)
{
	Point new_food;

	do {
		new_food = Point(rand_col(rng), rand_lin(rng));
	} while (snake.contains(new_food));

	to_update.push_back(new_food);
	scene[new_food.x][new_food.y] = FOOD_CH;
}

void update_scene(WINDOW *win)
{
	for (auto i : to_update) {
		char tile = scene[i.x][i.y];
		if (tile == '.')
			mvwaddch(win, i.y, i.x, ground[i.x][i.y] | COLOR_PAIR(1));
		else if (tile == FOOD_CH)
			mvwaddch(win, i.y, i.x, scene[i.x][i.y] | COLOR_PAIR(2));
		else
			mvwaddch(win, i.y, i.x, scene[i.x][i.y] | COLOR_PAIR(3));
	}
	to_update.clear();
}

int main(int argc, char **argv)
{
    bool color = false;
    if (argc > 1)
        if (string(argv[1]) == "color")
            color = true;

	initscr();
	noecho();
	curs_set(0);
	start_color();

    if (color) {
        init_color(COLOR_BLACK, 156, 156, 156);
        init_color(COLOR_GREEN, 721, 733, 149);
        init_color(COLOR_MAGENTA, 694, 384, 525);
        init_color(COLOR_YELLOW, 980, 741, 184);
        init_color(COLOR_WHITE, 921, 858, 698);
        init_color(COLOR_RED, 984, 286, 203);
    }

	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_BLACK, COLOR_MAGENTA);
	init_pair(3, COLOR_BLACK, COLOR_YELLOW);
	init_pair(4, COLOR_WHITE, COLOR_RED);
	init_pair(5, COLOR_BLACK, COLOR_WHITE);

	Snake snake;
	WINDOW *main_win = newwin(MAIN_LIN, MAIN_COL, 0, 0);;
	WINDOW *map_win = newwin(MAP_LIN, MAP_COL, 2, 2);
	WINDOW *help_win = newwin(4, MAP_COL, MAP_LIN + 3, 2);

	keypad(map_win, true);

	box(main_win, 0, 0);
    wattron(main_win, A_BOLD);
	mvwprintw(main_win, 0, MAIN_COL / 2 - 3, " Snake ");
    wattroff(main_win, A_BOLD);

	box(help_win, 0, 0);
    wattron(help_win, A_BOLD);
    mvwaddstr(help_win, 0, MAP_COL / 2 - 3, " Help ");
    wattroff(help_win, A_BOLD);
    mvwaddstr(help_win, 1, 2, "Move with ");
    wattron(help_win, COLOR_PAIR(1) | A_BOLD);
    waddstr(help_win, "wasd  hjkl    arrow keys");
    wattroff(help_win, COLOR_PAIR(1) | A_BOLD);
    mvwaddch(help_win, 1, 16, ',');
    mvwaddstr(help_win, 1, 23, "or");
    mvwaddstr(help_win, 1, 36, ". Press     to pause, ");
    wattron(help_win, COLOR_PAIR(1) | A_BOLD);
    waddch(help_win, 'r');
    mvwaddstr(help_win, 1, 44, "Spc");
    wattroff(help_win, COLOR_PAIR(1) | A_BOLD);
    mvwaddstr(help_win, 2, 2, "to restart or   to quit.");
    wattron(help_win, COLOR_PAIR(1) | A_BOLD);
    mvwaddch(help_win, 2, 16, 'Q');
    wattroff(help_win, COLOR_PAIR(1) | A_BOLD);

	refresh();
	wrefresh(main_win);
	wrefresh(help_win);

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

				if (roll == 1) 	    tile = '\'';
				else if (roll == 2) tile = '\"';
				else if (roll == 3) tile = '`';
				else if (roll == 4) tile = '.';

				ground[j][i] = scene[j][i] = tile;
				mvwaddch(map_win, i, j, ground[j][i] | COLOR_PAIR(1));
			}
		}

		snake = Snake();
		snake.print();
		gen_food(snake);

		wtimeout(map_win, DELAY);
		update_scene(map_win);

		wrefresh(map_win);

		bool paused = true;
		while(!snake.dead) {
			if (paused) {
				wattron(map_win, COLOR_PAIR(4) | A_BOLD);
				mvwprintw(map_win, MAP_LIN - 1, (MAP_COL - 2) / 2 - 4, " *PAUSED* ");
				wattroff(map_win, COLOR_PAIR(4) | A_BOLD);
				wrefresh(map_win);
				while (getch() != ' ') ;
				paused = false;
				wattron(map_win, COLOR_PAIR(5));
				mvwprintw(map_win, MAP_LIN - 1, (MAP_COL - 2) / 2 - 4, "##########");
				wattroff(map_win, COLOR_PAIR(5));
				wrefresh(map_win);
			}

			switch (wgetch(map_win)) {
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
				case 'Q':
					snake.dead = true;
					running = false;
					break;
				default:
					;
			}

			Point new_head = snake.seg.back() + dirvec[snake.dir];
			//char front = char(mvwinch(map_win, new_head.y, new_head.x));
            char front = scene[new_head.x][new_head.y];
			snake.update(front);

			if (front == FOOD_CH) {
				gen_food(snake);
			}

			update_scene(map_win);
			timeout(DELAY - DECR * snake.speed);
			wrefresh(map_win);
		}
	}

	delwin(main_win);
	delwin(map_win);
	endwin();

	return 0;
}
