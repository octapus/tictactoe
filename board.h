#ifndef __BOARD__
#define __BOARD__
#include<iostream>

enum CellState { EMPTY, X, O, WIN, KEY };

class Board {
	private:
		CellState board[3][3][3][3];
		friend std::ostream& operator<<(std::ostream&, const Board&);
		bool check(int x, int y, int z, int w, int dx, int dy, int dz, int dw, CellState state);
	public:
		Board();
		bool move(int x, int y, int z, int w, CellState state);
		CellState get(int x, int y, int z, int w) { return board[x][y][z][w]; }
		void clear();
};

#endif
