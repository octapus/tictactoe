#ifndef __KEYBINDS__
#define __KEYBINDS__
#include<GLFW/glfw3.h>
#include<unordered_map>

#define QUIT GLFW_KEY_ESCAPE
#define RESTART GLFW_KEY_DELETE
#define RESET_CAMERA GLFW_KEY_ENTER

#define UNDO GLFW_KEY_LEFT
#define REDO GLFW_KEY_RIGHT

#define W_UP GLFW_KEY_SPACE
#define W_DOWN GLFW_KEY_LEFT_SHIFT

#define RECOMMEND_KEY GLFW_KEY_1
#define RECOMMEND_BLOCK GLFW_KEY_2

#define RECOMMEND_KEY_1 GLFW_KEY_3
#define RECOMMEND_BLOCK_1 GLFW_KEY_4

#define FOCUS_RECOMMEND GLFW_KEY_LEFT_CONTROL

extern std::unordered_map<int, std::tuple<int, int, int>> keybinds;
void build_keybinds();

#endif
