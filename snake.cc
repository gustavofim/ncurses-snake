#include <ncurses.h>
#include <iostream>
#include <random>
#include <list>

using namespace std;

// getch timeout | "framerate" | difficulty
const int TIMEOUT = 100;
// Board size
int const BOARD_LIN = 32;
int const BOARD_COL = 62;
// Appearance
char const GROUND_CH = '.';
char const SNAKE_CH = 'O';
char const FOOD_CH = '@';

random_device dev;
mt19937 rng(dev());
uniform_int_distribution<std::mt19937::result_type> rand_col(1, BOARD_COL - 2);
uniform_int_distribution<std::mt19937::result_type> rand_lin(1, BOARD_LIN - 2);
uniform_int_distribution<std::mt19937::result_type> dice(1, 100);

struct Point {
	int x;
	int y;
	Point(int x, int y) : x(x), y(y) {}
	Point operator+(Point p) {
		return Point(p.x + x, p.y + y);
	}
	bool operator==(Point p) {
		return p.x == x && p.y == y;
	}
};

enum Dir {
	UP,
	DOWN,
	RIGHT,
	LEFT,
};

list<Point> snake = {
	Point(10, 10),
	Point(11, 10),
	Point(12, 10),
	Point(13, 10),
	Point(14, 10),
	Point(15, 10),
};

Point dirvec[4] {
	Point(0, -1),
	Point(0, 1),
	Point(1, 0),
	Point(-1, 0),
};

WINDOW *board;
Dir dir = RIGHT;

void
gen_food()
{
	mvwaddch(board, rand_lin(rng), rand_col(rng), FOOD_CH | COLOR_PAIR(2) | A_BLINK);
}

void
update_dir()
{
	switch (getch()) {
		case KEY_UP:
		case 'w':
		case 'k':
			if (dir != DOWN)
				dir = UP;
			break;
		case KEY_DOWN:
		case 's':
		case 'j':
			if (dir != UP)
				dir = DOWN;
			break;
		case KEY_RIGHT:
		case 'd':
		case 'l':
			if (dir != LEFT)
				dir = RIGHT;
			break;
		case KEY_LEFT:
		case 'a':
		case 'h':
			if (dir != RIGHT)
				dir = LEFT;
			break;
		default:
			;
	}
}

bool
update_snake()
{
	Point new_head = snake.back() + dirvec[dir];
	// Check collision with itself
	for (auto i : snake) {
		if (new_head == i)
			return true;
	}
	// Check collision with walls
	if (1 > new_head.x || BOARD_COL - 2 < new_head.x || 1 > new_head.y || BOARD_LIN - 2 < new_head.y)
		return true;
	snake.push_back(new_head);		
	if (char(mvwinch(board, new_head.y, new_head.x)) != FOOD_CH) {
		Point p = snake.front();
		mvwaddch(board, p.y, p.x, GROUND_CH | COLOR_PAIR(1));
		snake.pop_front();
		p = snake.back();
		mvwaddch(board, p.y, p.x, SNAKE_CH | COLOR_PAIR(3));
	} else {
		mvwaddch(board, new_head.y, new_head.x, SNAKE_CH | COLOR_PAIR(1));
		gen_food();
	}
	return false;
}

int
main()
{
	initscr();
	noecho();
	curs_set(0);
	start_color();
	init_pair(1, COLOR_GREEN, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
	timeout(TIMEOUT);
	keypad(stdscr, true);
	// Draws title, board and snake
	printw("SNAKE");
	mvchgat(0, 0, BOARD_COL, A_REVERSE, 0, 0);
	board = newwin(BOARD_LIN, BOARD_COL, 1, 0);
	box(board, 0, 0);
	for (int i = 1; i < BOARD_LIN - 1; ++i) {
		for (int j = 1; j < BOARD_COL - 1; ++j) {
			mvwaddch(board, i, j, GROUND_CH | COLOR_PAIR(1));
		}
	}
	for (auto i : snake) {
		mvwaddch(board, i.y, i.x, SNAKE_CH | COLOR_PAIR(3));
	}
	gen_food();
	refresh();
	wrefresh(board);
	// Game loop
	bool over = false;
	while(!over) {
		update_dir();
		over = update_snake();
		Point h = snake.back();
		mvprintw(BOARD_LIN+1, 0, "%d", snake.size());
		wrefresh(board);
	}
	endwin();

	return 0;
}