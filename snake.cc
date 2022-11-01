#include <ncurses.h>
#include <iostream>
#include <random>
#include <cstring>
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
char const GRASS_CH = '.';
// Window sizes
int const MAP_LIN = 32;
int const MAP_COL = 62;
int const HELP_LIN = 4;
int const HELP_COL = MAP_COL;
int const MAIN_LIN = MAP_LIN + HELP_LIN + 4;
int const MAIN_COL = MAP_COL + 4;
// "Tiles"
char ground[MAP_COL][MAP_LIN]; // Randomly generated grass
char scene[MAP_COL][MAP_LIN];  // What should be on the screen
// Randomness
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

	list<Point> get_points() {
        return seg;
	}

	void add_speed(int n) {
		speed += n;
		if (speed > MAX_SPD) speed = MAX_SPD;
		if (speed < 0) speed = 0;
	}

	Point move() {
        Point new_head = seg.back() + dirvec[dir];
        //char front = scene[new_head.x][new_head.y];

		//// Check collision
		//if (front == WALL_CH || front == SNAKE_BD) {
		//	dead = true;
		//	return new_head;
		//}

		seg.push_back(new_head);		

        return new_head;
	}

    void eat(char food) {
		if (food == FOOD_CH) {
			add_speed(1);
		} else if (food == WALL_CH || food == SNAKE_BD) {
			dead = true;
		} else {
			Point p = seg.front();
			seg.pop_front();
		}
    }
};

Point gen_food(Snake snake)
{
	Point new_food;

	do {
		new_food = Point(rand_col(rng), rand_lin(rng));
	} while (scene[new_food.x][new_food.y] != GRASS_CH);

	scene[new_food.x][new_food.y] = FOOD_CH;
    
    return new_food;
}
 
void print_on_wall(WINDOW *map, int color, const char *text)
{
    if (text == NULL) {
        wattron(map, COLOR_PAIR(5));
        mvwhline(map, MAP_LIN - 1, 0, ACS_HLINE, MAP_COL);
        wattroff(map, COLOR_PAIR(5));
        wrefresh(map);
        return;
    }

    wattron(map, COLOR_PAIR(color) | A_BOLD);
    mvwprintw(map, MAP_LIN - 1, (MAP_COL - 2) / 2 - (strlen(text) / 2), text);
    wattroff(map, COLOR_PAIR(color) | A_BOLD);
    wrefresh(map);
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
	init_pair(6, COLOR_WHITE, COLOR_BLUE);

	WINDOW *main_win = newwin(MAIN_LIN, MAIN_COL, 0, 0);;
	WINDOW *map_win = newwin(MAP_LIN, MAP_COL, 2, 2);
	WINDOW *help_win = newwin(HELP_LIN, HELP_COL, MAP_LIN + 3, 2);

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

    char head[4] = { '^', 'v', '>', '<' };
	Snake snake;
    list<Point> new_snake;
    list<Point> old_snake;
    Point food_pos;
    Point head_pos;
	bool running = true;
	while (running) {
        wattron(map_win, A_REVERSE);
        box(map_win, 0, 0);
        wattroff(map_win, A_REVERSE);
		for (int i = 0; i < MAP_LIN ; ++i) {
			for (int j = 0; j < MAP_COL; ++j) {
				if (i == 0 || j == 0 || i == MAP_LIN - 1 || j == MAP_COL - 1) {
					ground[j][i] = scene[j][i]  = WALL_CH;
					continue;
				}

				int roll = d10(rng);
				char tile = ' ';

				if (roll == 1) 	    tile = '\'';
				else if (roll == 2) tile = '\"';
				else if (roll == 3) tile = '`';
				else if (roll == 4) tile = '.';

				ground[j][i] = tile;
                scene[j][i] = GRASS_CH;
				mvwaddch(map_win, i, j, ground[j][i] | COLOR_PAIR(1));
			}
		}

		snake = Snake();
		new_snake = old_snake = snake.get_points();
        head_pos = new_snake.back();
		food_pos = gen_food(snake);

        for (auto i : new_snake) {
            mvwaddch(map_win, i.y, i.x, SNAKE_BD | COLOR_PAIR(3));
        }
        mvwaddch(map_win, head_pos.y, head_pos.x, head[snake.dir] | COLOR_PAIR(3));
        mvwaddch(map_win, food_pos.y, food_pos.x, FOOD_CH | COLOR_PAIR(2));

		wtimeout(map_win, DELAY);

		wrefresh(map_win);

		bool paused = false;

        print_on_wall(map_win, 6, " *PRESS ANY KEY TO START* ");
        getch();
        print_on_wall(map_win, 6, NULL);

		while(!snake.dead) {
            for (auto i : old_snake) {
                mvwaddch(map_win, i.y, i.x, ground[i.x][i.y] | COLOR_PAIR(1));
                scene[i.x][i.y] = GRASS_CH;
            }

            for (auto i : new_snake) {
                mvwaddch(map_win, i.y, i.x, SNAKE_BD | COLOR_PAIR(3));
                scene[i.x][i.y] = SNAKE_BD;
            }
            mvwaddch(map_win, head_pos.y, head_pos.x, head[snake.dir] | COLOR_PAIR(3));

            old_snake = new_snake;

			switch (wgetch(map_win)) {
				case KEY_UP:
				case 'w':
				case 'k':
					if (snake.dir != DOWN && !paused)
						snake.dir = UP;
					break;
				case KEY_DOWN:
				case 's':
				case 'j':
					if (snake.dir != UP && !paused)
						snake.dir = DOWN;
					break;
				case KEY_RIGHT:
				case 'd':
				case 'l':
					if (snake.dir != LEFT && !paused)
						snake.dir = RIGHT;
					break;
				case KEY_LEFT:
				case 'a':
				case 'h':
					if (snake.dir != RIGHT && !paused)
						snake.dir = LEFT;
					break;
				case ' ':
					paused = !paused;
                    if (paused) print_on_wall(map_win, 4, " *PAUSED* ");
                    else print_on_wall(map_win, 4, NULL);
                    wrefresh(map_win);
					break;
				case 'Q':
					snake.dead = true;
					running = false;
					break;
				default:
					;
			}

            if (paused) continue;

			head_pos = snake.move();
            snake.eat(scene[head_pos.x][head_pos.y]);

			if (head_pos == food_pos) {
				food_pos = gen_food(snake);
                mvwaddch(map_win, food_pos.y, food_pos.x, FOOD_CH | COLOR_PAIR(2));
			}

            new_snake = snake.get_points();

			wtimeout(map_win, DELAY - DECR * snake.speed);
			wrefresh(map_win);
		}
	}

	delwin(main_win);
	delwin(map_win);
	endwin();

	return 0;
}
