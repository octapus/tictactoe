#include "board.hpp"

Board::Board() {
	clear();
}

/**
 * Checks if there is a win in a specific direction.
 * If the direction is off the board, it returns false.
 * Otherwise it will reverse directions if necessary when checking the second in a row.
 */
bool Board::check(int x, int y, int z, int w, int dx, int dy, int dz, int dw, Turn turn) {
	int x1 = x+dx;
	int y1 = y+dy;
	int z1 = z+dz;
	int w1 = w+dw;

	if(x1 > 2 || y1 > 2 || z1 > 2 || w1 > 2)
		return false;
	if(x1 < 0 || y1 < 0 || z1 < 0 || w1 < 0)
		return false;

	if(board[x1][y1][z1][w1].state != CellState::PLACE || board[x1][y1][z1][w1].turn != turn)
		return false;

	int x2 = x-dx;
	int y2 = y-dy;
	int z2 = z-dz;
	int w2 = w-dw;
	if(x2 < 0 || x2 > 2 || y2 < 0 || y2 > 2 || z2 < 0 || z2 > 2 || w2 < 0 || w2 > 2) {
		x2 = x+2*dx;
		y2 = y+2*dy;
		z2 = z+2*dz;
		w2 = w+2*dw;
	}
	if(x2 < 0 || x2 > 2 || y2 < 0 || y2 > 2 || z2 < 0 || z2 > 2 || w2 < 0 || w2 > 2) {
		return false;
	}

	if(board[x2][y2][z2][w2].state != CellState::PLACE || board[x2][y2][z2][w2].turn != turn)
		return false;

	return true;
}

/**
 * Returns a list of the directions of wins (3 in a rows) for a specific move
 */
int Board::getWins(int x, int y, int z, int w, Turn turn, std::vector<std::array<int, 4>> *directions) {
	// If no directions array if provided, one must be created to check for duplicates
	bool tmpDirections = false;
	if(directions == nullptr) {
		directions = new std::vector<std::array<int, 4>>();
		tmpDirections = true;
	}

	for(int dw = -1; dw <= 1; ++dw) {
		for(int dz = -1; dz <= 1; ++dz) {
			for(int dy = -1; dy <= 1; ++dy) {
				for(int dx = -1; dx <= 1; ++dx) {
					// Skip if all directions are 0
					if(dx == 0 && dy == 0 && dz == 0 && dw == 0)
						continue;

					// Check if direction contains 3 in a row
					if(check(x, y, z, w, dx, dy, dz, dw, turn)) {
						// Confirm it is not a duplicate of another win (in the opposite direction)
						bool duplicate = false;
						for(const auto &win : *directions) {
							if(dx == -win[0] && dy == -win[1] && dz == -win[2] && dw == -win[3]) {
								duplicate = true;
								break;
							}
						}

						if(!duplicate) directions->push_back({dx, dy, dz, dw});
					}
				}
			}
		}
	}

	int result = directions->size();

	// If a temporary directions array was created, clean up
	if(tmpDirections) {
		delete directions;
	}

	return result;
}

/**
 * Moves in the specified spot if it is not already occupied.
 */
bool Board::move(int x, int y, int z, int w, Turn turn) {
	clearRecs();
	if(board[x][y][z][w].state != CellState::EMPTY) return false;

	board[x][y][z][w].turn = turn;
	board[x][y][z][w].state = CellState::PLACE;

	std::vector<std::array<int, 4>> winDirections;
	getWins(x, y, z, w, turn, &winDirections);

	// mark wins
	if(winDirections.size() >= 2) {
		for(const auto &win : winDirections) {
			int x1 = x+win[0];
			int y1 = y+win[1];
			int z1 = z+win[2];
			int w1 = w+win[3];
			board[x1][y1][z1][w1].state = CellState::WIN;

			int x2 = x-win[0];
			int y2 = y-win[1];
			int z2 = z-win[2];
			int w2 = w-win[3];
			if(x2 < 0 || x2 > 2 || y2 < 0 || y2 > 2 || z2 < 0 || z2 > 2 || w2 < 0 || w2 > 2) {
				x2 = x+2*win[0];
				y2 = y+2*win[1];
				z2 = z+2*win[2];
				w2 = w+2*win[3];
			}
			board[x2][y2][z2][w2].state = CellState::WIN;

			printf("%d %d %d %d\n", x, y, z, w);
			printf("%d %d %d %d\n", x1, y1, z1, w1);
			printf("%d %d %d %d\n", x2, y2, z2, w2);
			std::cout << std::endl;
		}

		board[x][y][z][w].state = CellState::KEY;

		return true;
	}

	return false;
}

void Board::remove(int x, int y, int z, int w) {
	clearRecs();
	board[x][y][z][w].state = CellState::EMPTY;
}

int Board::possibleKeys(Turn turn, bool mark) {
	int result = 0;

	for(int x = 0; x < 3; ++x) {
		for(int y = 0; y < 3; ++y) {
			for(int z = 0; z < 3; ++z) {
				for(int w = 0; w < 3; ++w) {
					if(board[x][y][z][w].state == CellState::EMPTY) {
						if(getWins(x, y, z, w, turn, nullptr) >= 2) {
							if(mark) {
								board[x][y][z][w].state = CellState::KEY_POSS;
								board[x][y][z][w].turn = turn;
							}
							++result;
						}
					}
				}
			}
		}
	}

	return result;
}

void Board::clear() {
	for(int x = 0; x < 3; ++x) {
		for(int y = 0; y < 3; ++y) {
			for(int z = 0; z < 3; ++z) {
				for(int w = 0; w < 3; ++w) {
					board[x][y][z][w].state = CellState::EMPTY;
				}
			}
		}
	}
}

void Board::clearRecs() {
	for(int x = 0; x < 3; ++x) {
		for(int y = 0; y < 3; ++y) {
			for(int z = 0; z < 3; ++z) {
				for(int w = 0; w < 3; ++w) {
					if(board[x][y][z][w].state == CellState::KEY_POSS) board[x][y][z][w].state = CellState::EMPTY;
				}
			}
		}
	}
}

void Board::clearState() {
	for(int x = 0; x < 3; ++x) {
		for(int y = 0; y < 3; ++y) {
			for(int z = 0; z < 3; ++z) {
				for(int w = 0; w < 3; ++w) {
					if(board[x][y][z][w].state == CellState::KEY_POSS) board[x][y][z][w].state = CellState::EMPTY;
					if(board[x][y][z][w].state != CellState::EMPTY) board[x][y][z][w].state = CellState::PLACE;
				}
			}
		}
	}
}

std::ostream& operator<<(std::ostream &strm, const Board &b) {
	for(int w = 0; w < 3; ++w) {
		for(int y = 0; y < 3; ++y) {
			for(int z = 0; z < 3; ++z) {
				for(int x = 0; x < 3; ++x) {
					if(b.board[x][y][z][w].state == CellState::EMPTY) {
						strm << '.';
					} else {
						switch(b.board[x][y][z][w].turn) {
							case X:
								strm << ((b.board[x][y][z][w].state == CellState::KEY_POSS) ? 'x' : 'X');
								break;
							case O:
								strm << ((b.board[x][y][z][w].state == CellState::KEY_POSS) ? 'o' : 'O');
								break;
							default:
								strm << '?';
								break;
						}
					}
					strm << " ";
				}
				strm << " ";
			}
			strm << std::endl;
		}
		strm << std::endl;
	}
	return strm;
}
