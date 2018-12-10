#include "keybinds.hpp"

// back/middle/front (front is facing screen) up/center/down left/middle/right
#define BUL GLFW_KEY_Q
#define BUU GLFW_KEY_W
#define BUR GLFW_KEY_E
#define BCL GLFW_KEY_A
#define BCC GLFW_KEY_S
#define BCR GLFW_KEY_D
#define BDL GLFW_KEY_Z
#define BDD GLFW_KEY_X
#define BDR GLFW_KEY_C

#define MUL GLFW_KEY_R
#define MUU GLFW_KEY_T
#define MUR GLFW_KEY_Y
#define MCL GLFW_KEY_F
#define MCC GLFW_KEY_G
#define MCR GLFW_KEY_H
#define MDL GLFW_KEY_V
#define MDD GLFW_KEY_B
#define MDR GLFW_KEY_N

#define FUL GLFW_KEY_U
#define FUU GLFW_KEY_I
#define FUR GLFW_KEY_O
#define FCL GLFW_KEY_J
#define FCC GLFW_KEY_K
#define FCR GLFW_KEY_L
#define FDL GLFW_KEY_M
#define FDD GLFW_KEY_COMMA
#define FDR GLFW_KEY_PERIOD

std::unordered_map<int, std::tuple<int, int, int>> keybinds;

void build_keybinds() {
	// build mapping of keys to positions
	keybinds[BUL] = std::make_tuple(0, 0, 0);
	keybinds[BUU] = std::make_tuple(1, 0, 0);
	keybinds[BUR] = std::make_tuple(2, 0, 0);
	keybinds[BCL] = std::make_tuple(0, 1, 0);
	keybinds[BCC] = std::make_tuple(1, 1, 0);
	keybinds[BCR] = std::make_tuple(2, 1, 0);
	keybinds[BDL] = std::make_tuple(0, 2, 0);
	keybinds[BDD] = std::make_tuple(1, 2, 0);
	keybinds[BDR] = std::make_tuple(2, 2, 0);

	keybinds[MUL] = std::make_tuple(0, 0, 1);
	keybinds[MUU] = std::make_tuple(1, 0, 1);
	keybinds[MUR] = std::make_tuple(2, 0, 1);
	keybinds[MCL] = std::make_tuple(0, 1, 1);
	keybinds[MCC] = std::make_tuple(1, 1, 1);
	keybinds[MCR] = std::make_tuple(2, 1, 1);
	keybinds[MDL] = std::make_tuple(0, 2, 1);
	keybinds[MDD] = std::make_tuple(1, 2, 1);
	keybinds[MDR] = std::make_tuple(2, 2, 1);

	keybinds[FUL] = std::make_tuple(0, 0, 2);
	keybinds[FUU] = std::make_tuple(1, 0, 2);
	keybinds[FUR] = std::make_tuple(2, 0, 2);
	keybinds[FCL] = std::make_tuple(0, 1, 2);
	keybinds[FCC] = std::make_tuple(1, 1, 2);
	keybinds[FCR] = std::make_tuple(2, 1, 2);
	keybinds[FDL] = std::make_tuple(0, 2, 2);
	keybinds[FDD] = std::make_tuple(1, 2, 2);
	keybinds[FDR] = std::make_tuple(2, 2, 2);
}

