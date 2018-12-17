#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "shaders/loadShaders.hpp"
#include "polygons.hpp"
#include "keybinds.hpp"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<iostream>
#include<vector>
#include "board.hpp"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800

#define PAN_SPEED 0.0001f
#define ZOOM_SPEED 0.1

#define PLACE_BRIGHTNESS 0.2
#define WIN_BRIGHTNESS 0.7
#define KEY_BRIGHTNESS 1

#define KEY_POSS_BRIGHTNESS 0
#define KEY_POSS_FOCUSED_BRIGHTNESS 1
#define BLOCK_POSS_BRIGHTNESS 0
#define BLOCK_POSS_FOCUSED_BRIGHTNESS 0.8

#define KEY_POSS_1_BRIGHTNESS 0
#define KEY_POSS_1_FOCUSED_BRIGHTNESS 0.5
#define BLOCK_POSS_1_BRIGHTNESS 0
#define BLOCK_POSS_1_FOCUSED_BRIGHTNESS 0.3

// board color += recommendation * BOARD_POSS_ADJUST
#define BOARD_POSS_ADJUST 0.5

// Brightness of unfocused moves = UNFOCUSED_MOVE_DIM
#define UNFOCUSED_MOVE_DIM 0

// Switches turns (just in case more players are added)
#define turnCycle() turn = (turn == X) ? O : X;

// updates the mvp matrix and sends it to the gpu.
#define update_mvp() mvp = Projection * View * Model; glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));

// updates the rgb vector and sends it to the gpu.
#define update_rgb(r, g, b)	glUniform3f(rgbLoc, r, g, b);

Board board;
Turn turn;
std::vector<std::tuple<int, int, int, int>> moveHistory;
int moveHistoryIndex;
bool wDown = false, wUp = false;
int w;
bool quit;
bool won;

bool key_recommend = false, block_recommend = false;
bool key_1_recommend = false, block_1_recommend = false;
bool focus_recommend = false;
float recommendation = 0;

GLFWwindow *window;
unsigned int VAO, VBO, EBO; // board
unsigned int X_VAO, X_VBO, X_EBO; // x
unsigned int O_VAO, O_VBO, O_EBO; // o
GLuint shaderProgram;

glm::mat4 Model, View, Projection, mvp;
unsigned int mvpLoc, rgbLoc;

float theta, phi, radius; // theta on xz plane, 0 at +x. phi on y axis, 0 is horizontal (on xz plane)
double mouseStartX, mouseStartY; // for camera orbit calculations


// updates the View matrix to the current camera position. Does not update mvp.
void update_view() {
	float cameraX = radius * cos(theta) * cos(phi);
	float cameraY = radius * sin(phi);
	float cameraZ = radius * sin(theta) * cos(phi);
	View = glm::lookAt(
			glm::vec3(cameraX, cameraY, cameraZ), // camera pos
			glm::vec3(0, 0, 0), // looking at origin
			glm::vec3(0, 1, 0) // +y is up
		);
}

bool init() {
	// init opengl
	glewExperimental = true;
	if(!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Create window
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "tic-tac-toe", NULL, NULL);
	if(window == NULL) {
		std::cerr << "Failed to open GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = true;
	if(glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// build model arrays
	std::cout << "generating models..." << std::endl;
	generate_board_bg();
	generate_x_polygon();

	// Generate buffers
	// x buffers
	glGenVertexArrays(1, &X_VAO);
	glBindVertexArray(X_VAO);

	glGenBuffers(1, &X_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, X_VBO);
	glBufferData(GL_ARRAY_BUFFER, x_vertices_size, x_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &X_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, X_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, x_indices_size, x_indices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// o buffers
	glGenVertexArrays(1, &O_VAO);
	glBindVertexArray(O_VAO);

	glGenBuffers(1, &O_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, O_VBO);
	glBufferData(GL_ARRAY_BUFFER, icosahedron_vertices_size, icosahedron_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &O_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, O_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, icosahedron_indicies_size, icosahedron_indicies, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// board buffers
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, board_vertices_size, board_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, board_indices_size, board_indices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);


	// Load Shaders
	shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag", nullptr);
	glUseProgram(shaderProgram);
	mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
	rgbLoc = glGetUniformLocation(shaderProgram, "rgb");

	// Generate mvp and camera
	Projection = glm::perspective(glm::radians(45.0f), float(SCREEN_WIDTH)/float(SCREEN_HEIGHT), 0.1f, 100.0f);
	theta = M_PI/2;
	phi = 0;
	radius = 4;
	mouseStartX = -1;
	mouseStartY = -1;
	update_view();
	Model = glm::mat4(1.0f);
	update_mvp();

	return true;
}

void close() {
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);

	glDeleteBuffers(1, &X_VBO);
	glDeleteBuffers(1, &X_EBO);
	glDeleteVertexArrays(1, &X_VAO);

	glDeleteBuffers(1, &O_VBO);
	glDeleteBuffers(1, &O_EBO);
	glDeleteVertexArrays(1, &O_VAO);

	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
}

// draws the moves of a specific player
void draw_specific_moves(Turn player) {
	// select correct VAO
	switch(player) {
		case X:
			glBindVertexArray(X_VAO);
			break;
		case O:
			glBindVertexArray(O_VAO);
			break;
		default:
			return;
			break;
	}

	// iterate through board with w (colors) last
	for(int lx = 0; lx < 3; ++lx) {
		for(int ly = 0; ly < 3; ++ly) {
			for(int lz = 0; lz < 3; ++lz) {
				// check all w for the move in order to set the right color blend
				float rgb[3];
				for(int c = 0; c < 3; ++c) {
					CellState state = board.get(lx, ly, lz, c);
					if(state.state != CellState::EMPTY && state.turn == player) {
						if(won) {
							switch(state.state) {
								case CellState::PLACE:
									rgb[c] = PLACE_BRIGHTNESS;
									break;
								case CellState::WIN:
									rgb[c] = WIN_BRIGHTNESS;
									break;
								case CellState::KEY:
									rgb[c] = KEY_BRIGHTNESS;
									break;
								default:
									rgb[c] = 0;
									break;
							}
						} else {
							switch(state.state) {
								case CellState::KEY_POSS_1:
									rgb[c] = KEY_POSS_1_BRIGHTNESS;
									break;
								case CellState::BLOCK_POSS_1:
									rgb[c] = BLOCK_POSS_1_BRIGHTNESS;
									break;
								case CellState::KEY_POSS:
									rgb[c] = KEY_POSS_BRIGHTNESS;
									break;
								case CellState::BLOCK_POSS:
									rgb[c] = BLOCK_POSS_BRIGHTNESS;
									break;
								default:
									rgb[c] = 1;
									break;
							}
						}

						// dim unfocused moves
						if(focus_recommend) {
							switch(state.state) {
								case CellState::KEY_POSS:
									rgb[c] = KEY_POSS_FOCUSED_BRIGHTNESS;
									break;
								case CellState::BLOCK_POSS:
									rgb[c] = BLOCK_POSS_FOCUSED_BRIGHTNESS;
									break;
								case CellState::KEY_POSS_1:
									rgb[c] = KEY_POSS_1_FOCUSED_BRIGHTNESS;
									break;
								case CellState::BLOCK_POSS_1:
									rgb[c] = BLOCK_POSS_1_FOCUSED_BRIGHTNESS;
									break;
								case CellState::KEY:
									break;
								case CellState::WIN:
									break;
								default:
									rgb[c] = UNFOCUSED_MOVE_DIM * rgb[c];
									break;
							}
						}
						if(wUp || wDown) {
							if(c != w) rgb[c] = UNFOCUSED_MOVE_DIM * rgb[c];
						}
					} else {
						rgb[c] = 0;
					}
				}

				if(rgb[0] > 0 || rgb[1] > 0 || rgb[2] > 0) {
					update_rgb(rgb[0], rgb[1], rgb[2]);

					Model = glm::translate(glm::mat4(1.0f), glm::vec3(2*SCALE*(lx-1), -2*SCALE*(ly-1), 2*SCALE*(lz-1)));
					update_mvp();

					switch(player) {
						case X:
							glDrawElements(GL_TRIANGLES, 4*36, GL_UNSIGNED_INT, 0);
							break;
						case O:
							glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);
							break;
						default:
							break;
					}
				}
			}
		}
	}

	Model = glm::mat4(1.0f);
	update_mvp();
}

void draw_board_background() {
	glBindVertexArray(VAO);

	// set board color to either color of winning move or color of next move to be placed
	int local_w = won ? std::get<3>(moveHistory.at(moveHistoryIndex)) : w;

	// brighten if key_possible, dim if block_possible
	float mod = 0;
	if(!won && recommendation != 0) {
		mod = recommendation * BOARD_POSS_ADJUST;
	}

	update_rgb((local_w == 0) + mod, (local_w == 1) + mod, (local_w == 2) + mod);

	glDrawElements(GL_TRIANGLES, 12*36, GL_UNSIGNED_INT, 0);
}

float recommend_keys() {
	board.clearRecs();
	float result = 0;

	if(block_1_recommend) {
		if(board.possibleBlocks(turn, true, 1)) {
			result = -0.5;
		}
	}

	if(key_1_recommend) {
		if(board.possibleKeys(turn, true, 1)) {
			result = 0.5;
		}
	}

	if(block_recommend) {
		if(board.possibleBlocks(turn, true, 0)) {
			result = -1;
		}
	}

	if(key_recommend) {
		if(board.possibleKeys(turn, true, 0)) {
			result = 1;
		}
	}

	recommendation = result;
	return result;
}

void undo() {
	if(moveHistoryIndex >= 0) {
		std::tuple<int, int, int, int> priorMove = moveHistory.at(moveHistoryIndex--);
		board.remove(std::get<0>(priorMove), std::get<1>(priorMove), std::get<2>(priorMove), std::get<3>(priorMove));
		turnCycle();
		recommend_keys();
	}
}
void redo() {
	if(moveHistoryIndex < (int) (moveHistory.size()) - 1) {
		std::tuple<int, int, int, int> priorMove = moveHistory.at(++moveHistoryIndex);
		if(board.move(std::get<0>(priorMove), std::get<1>(priorMove), std::get<2>(priorMove), std::get<3>(priorMove), turn)) won = true;
		turnCycle();
		if(!won) recommend_keys();
	}
}
void restart() {
	board.clear();
	turn = X;
	won = false;
	moveHistoryIndex = -1;
	recommendation = 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(action == GLFW_PRESS) {
		switch(key) {
			case QUIT:
				quit = true;
				break;
			case RESTART:
				restart();
				break;
			case RESET_CAMERA:
				theta = M_PI/2;
				phi = 0;
				radius = 4;
				update_view();
				update_mvp();
				break;
			case W_UP:
				wUp = true;
				w = (wUp && wDown) ? 1 : 2;
				break;
			case W_DOWN:
				wDown = true;
				w = (wUp && wDown) ? 1 : 0;
				break;
			case RECOMMEND_KEY:
				key_recommend = !key_recommend;
				recommend_keys();
				break;
			case RECOMMEND_BLOCK:
				block_recommend = !block_recommend;
				recommend_keys();
				break;
			case RECOMMEND_KEY_1:
				key_1_recommend = !key_1_recommend;
				recommend_keys();
				break;
			case RECOMMEND_BLOCK_1:
				block_1_recommend = !block_1_recommend;
				recommend_keys();
				break;
			case FOCUS_RECOMMEND:
				focus_recommend = true;
				break;
			case UNDO:
				if(won) {
					won = false;
					board.clearWins();
				}
				undo();
				break;
			case REDO:
				redo();
				break;
			default:
				if(!won && keybinds.find(key) != keybinds.end()) {
					std::tuple<int, int, int> move = keybinds[key];
					if(board.get(std::get<0>(move), std::get<1>(move), std::get<2>(move), w).state != CellState::PLACE) { // if valid move
						// add to move history
						moveHistory.erase(moveHistory.begin() + moveHistoryIndex + 1, moveHistory.end());
						moveHistory.push_back(std::make_tuple(std::get<0>(move), std::get<1>(move), std::get<2>(move), w));
						moveHistoryIndex = moveHistory.size() - 1;

						// place move and check for a win
						if(board.move(std::get<0>(move), std::get<1>(move), std::get<2>(move), w, turn)) {
							won = true;
							board.clearRecs();
							std::cout << board << std::endl;
						}

						// change turn
						turnCycle();

						// show recommendations
						recommend_keys();
					}
				}
				break;
		}
	} else if(action == GLFW_RELEASE) {
		switch(key) {
			case W_UP:
				wUp = false;
				w = wDown ? 0 : 1;
				break;
			case W_DOWN:
				wDown = false;
				w = wUp ? 2 : 1;
				break;
			case FOCUS_RECOMMEND:
				focus_recommend = false;
				break;
		}
	}
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	if(mouseStartX >= 0 && mouseStartY >= 0) {
		theta += (xpos - mouseStartX) * PAN_SPEED;
		phi += (ypos - mouseStartY) * PAN_SPEED;

		// when phi hits pi everything flips, so stop it from ocurring
		if(phi > M_PI/2.01) phi = M_PI/2.01;
		if(phi < -M_PI/2.01) phi = -M_PI/2.01;

		update_view();
		update_mvp();
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		glfwGetCursorPos(window, &mouseStartX, &mouseStartY);
		return;
	}
	mouseStartX = -1;
	mouseStartY = -1;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	radius += yoffset * ZOOM_SPEED;
	if(radius < 0) radius = 0.01;
	update_view();
	update_mvp();
}

void window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	Projection = glm::perspective(glm::radians(45.0f), float(width)/float(height), 0.1f, 100.0f);
	update_mvp();
}

int main() {
	if(!init()) {
		printf("Failed to initialize");
	} else {
		board = Board();
		build_keybinds();
		moveHistory.clear();
		w = 1;
		restart();

		glfwSetKeyCallback(window, key_callback);
		glfwSetCursorPosCallback(window, cursor_pos_callback);
		glfwSetMouseButtonCallback(window, mouse_button_callback);
		glfwSetScrollCallback(window, scroll_callback);
		glfwSetWindowSizeCallback(window, window_size_callback);

		// Main loop
		quit = false;
		won = false;
		while(!glfwWindowShouldClose(window) && !quit) {
			glfwWaitEvents();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			draw_board_background();
			draw_specific_moves(X);
			draw_specific_moves(O);

			glfwSwapBuffers(window);
		}

		return EXIT_SUCCESS;
	}
}
