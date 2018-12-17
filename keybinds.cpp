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

std::unordered_map<int, std::array<int, 3>> keybinds;

void build_keybinds() {
	// build mapping of keys to positions
	keybinds[BUL] = {0, 0, 0};
	keybinds[BUU] = {1, 0, 0};
	keybinds[BUR] = {2, 0, 0};
	keybinds[BCL] = {0, 1, 0};
	keybinds[BCC] = {1, 1, 0};
	keybinds[BCR] = {2, 1, 0};
	keybinds[BDL] = {0, 2, 0};
	keybinds[BDD] = {1, 2, 0};
	keybinds[BDR] = {2, 2, 0};

	keybinds[MUL] = {0, 0, 1};
	keybinds[MUU] = {1, 0, 1};
	keybinds[MUR] = {2, 0, 1};
	keybinds[MCL] = {0, 1, 1};
	keybinds[MCC] = {1, 1, 1};
	keybinds[MCR] = {2, 1, 1};
	keybinds[MDL] = {0, 2, 1};
	keybinds[MDD] = {1, 2, 1};
	keybinds[MDR] = {2, 2, 1};

	keybinds[FUL] = {0, 0, 2};
	keybinds[FUU] = {1, 0, 2};
	keybinds[FUR] = {2, 0, 2};
	keybinds[FCL] = {0, 1, 2};
	keybinds[FCC] = {1, 1, 2};
	keybinds[FCR] = {2, 1, 2};
	keybinds[FDL] = {0, 2, 2};
	keybinds[FDD] = {1, 2, 2};
	keybinds[FDR] = {2, 2, 2};
}

