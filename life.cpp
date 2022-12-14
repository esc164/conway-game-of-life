#pragma GCC diagnostic ignored "-Wnarrowing"

#include <csignal>
#include <fstream>
#include <limits>
#include <random>
#include <utility>
#include <vector>

#include <iostream>

extern "C" {
#include <ncurses.h>
#include <unistd.h>
}

constexpr char empty{' '}, alive{'X'};

using Row = std::vector<char>;
using Board = std::vector<Row>;

void term(int);
int checkNeighbors(const Board&, int, int);
void iterateBoard(Board&);
void printBoard(const Board&);

int main(int argc, char *argv[]) {
	signal(SIGTERM, term);
	Board board;
	initscr();
	noecho();
	cbreak();

	int y{ 0 }, x{ 0 };

	int times{ 0 };
	int arg;
	bool isReadingFile{ false };
	// Parse arguments
	while ((arg = getopt(argc, argv, "t:i")) != -1) {
		switch(arg) {
			case 't':
				for (int i{ 0 }; optarg[i] != '\0'; ++i)
					if ('0' <= optarg[i] && optarg[i] <= '9')
						times = times * 10 + (optarg[i] - '0');
				break;
			case 'i': // short for input
				if (!isReadingFile) {
					int xmax, ymax;
					getmaxyx(stdscr, ymax, xmax);
					board.resize(ymax);
					for (auto& row : board) {
						row.resize(xmax);
						for (auto& cell : row)
							cell = empty;
					}
					int x{ 0 }, y{ 0 };

					char in{};
					curs_set(2);
					do {
						switch (in = getch()) {
							case 'w':
								if (y >= 0)
									--y;
								break;
							case 'a':
								if (x >= 0)
									--x;
								break;
							case 's':
								if (y < ymax)
									++y;
								break;
							case 'd':
								if (x < xmax)
									++x;
								break;
							case ' ':
								if (board[y][x] == alive)
									board[y][x] = empty;
								else
									board[y][x] = alive;
								break;
						}
						printBoard(board);
						move(y, x);
					} while (in != 'z');
					isReadingFile = true;
				}
				break;
		}
	}
	if (times == 0)
		times = 100;

	if (!isReadingFile) {
		getmaxyx(stdscr, y, x);
		if (y == 0 || x == 0) {
			y = 30;
			x = 120;
		}

		board.resize(y);
		for (auto& row : board)
			row.resize(x);

		std::random_device rd;
		std::mt19937 mt{ rd() };
		std::uniform_int_distribution<int> dist{ 0, 1 };
		for (auto& row : board)
			for (auto& cell : row)
				cell = (dist(mt) == 1 ? alive : empty);
	}

	printBoard(board);
	curs_set(0);
	for (int i{ 0 }; i < times; ++i) {
		sleep(1);
		iterateBoard(board);
		printBoard(board);
	}
	getch();
	endwin();
	return 0;
}

void term(int sig) {
	endwin();
	std::exit(1);
}

int countNeighbors(const Board& board, int x, int y) {
	int n{ 0 };
	static constexpr int neighbors[8][2]{
			{-1,-1},{-1,0},{-1,1},
			{ 0,-1},       { 0,1},
			{ 1,-1},{ 1,0},{ 1,1}
	};
	static const auto bounded = [](int x, int lower, int upper) -> bool {
		return lower <= x && x <= upper;
	};
	int ymax = board.size(), xmax = board[0].size();
	for (const auto& neighbor : neighbors) {
		if (bounded(y + neighbor[1], 0, ymax - 1)
				&& bounded(x + neighbor[0], 0, xmax - 1))
			if (board[y + neighbor[1]][x + neighbor[0]] == alive)
				++n;
	}

	return n;
}

void iterateBoard(Board& board) {
	static Board stage;
	int ymax{ board.size() }, xmax{ board[0].size() };

	if (stage.size() != ymax)
		stage.resize(ymax);
	if (stage[0].size() != xmax)
		for (auto& row : stage)
			row.resize(xmax);

	int n;
	for (int y{ 0 }; y < ymax; ++y) {
		for (int x{ 0 }; x < xmax; ++x) {
			n = countNeighbors(board, x, y);
			if (n == 3)
				stage[y][x] = alive;
			else if (n == 2)
				stage[y][x] = board[y][x];
			else
				stage[y][x] = empty;
		}
	}
	std::swap(board, stage);
}

void printBoard(const Board& board) {
	move(0, 0);
	for (int y{ 0 }; y < board.size(); ++y) {
		for (int x{ 0 }; x < board[0].size(); ++x) {
			mvaddch(y, x, board[y][x]);
		}
	}
	refresh();
}
