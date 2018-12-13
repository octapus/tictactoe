#ifndef __BOARD__
#define __BOARD__
#include<iostream>

enum Turn {X, O};

struct CellState {
	Turn turn;
	enum State {EMPTY, PLACE, WIN, KEY} state;
};

class Board {
	private:
		CellState board[3][3][3][3];
		friend std::ostream& operator<<(std::ostream&, const Board&);
		bool check(int x, int y, int z, int w, int dx, int dy, int dz, int dw, Turn turn);
	public:
		Board();
		bool move(int x, int y, int z, int w, Turn turn);
		void remove(int x, int y, int z, int w);
		CellState get(int x, int y, int z, int w) { return board[x][y][z][w]; }
		void clear();
};

#endif
