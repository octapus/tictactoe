#ifndef __BOARD__
#define __BOARD__
#include<iostream>
#include<vector>
#include<array>

enum Turn {X, O};

struct CellState {
	Turn turn;
	enum State {EMPTY, PLACE, WIN, KEY, KEY_POSS, BLOCK_POSS, KEY_POSS_1, BLOCK_POSS_1} state;
};

class Board {
	private:
		CellState board[3][3][3][3];
		friend std::ostream& operator<<(std::ostream&, const Board&);
		bool check(int x, int y, int z, int w, int dx, int dy, int dz, int dw, Turn turn);
		int getWins(int x, int y, int z, int w, Turn turn, std::vector<std::array<int, 4>> *directions);
		int possibleKeysMark(Turn turn, bool mark, Turn markTurn, CellState::State markState, int layer);
	public:
		Board();
		bool move(int x, int y, int z, int w, Turn turn);
		bool move(std::array<int, 4> &move, Turn turn);
		void remove(int x, int y, int z, int w);
		void remove(std::array<int, 4> &move);
		CellState get(int x, int y, int z, int w) { return board[x][y][z][w]; }
		int possibleKeys(Turn turn, bool mark, int layer);
		int possibleBlocks(Turn turn, bool mark, int layer);
		void clear();
		void clear(CellState::State state);
		void clearRecs();
		void clearWins();
};

#endif
