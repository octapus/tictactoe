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

#define PAN_SPEED 0.003f
#define ZOOM_SPEED 0.1

// brightness values after win. FOCUS_RECOMMEND (LEFT_CTRL) to dim non-win moves.
#define PLACE_BRIGHTNESS 1
#define WIN_BRIGHTNESS 1
#define KEY_BRIGHTNESS 1

// brightness values for possibilites (recommendations) when unfocused/focused with FOCUS_RECOMMEND (LEFT_CTRL)
#define KEY_POSS_BRIGHTNESS 0
#define KEY_POSS_FOCUSED_BRIGHTNESS 1
#define BLOCK_POSS_BRIGHTNESS 0
#define BLOCK_POSS_FOCUSED_BRIGHTNESS 0.8

#define KEY_POSS_1_BRIGHTNESS 0
#define KEY_POSS_1_FOCUSED_BRIGHTNESS 0.5
#define BLOCK_POSS_1_BRIGHTNESS 0
#define BLOCK_POSS_1_FOCUSED_BRIGHTNESS 0.2

// board color += recommendation * BOARD_POSS_ADJUST
#define BOARD_POSS_ADJUST 0.8

// unfocused move color *= UNFOCUSED_MOVE_DIM
#define UNFOCUSED_MOVE_DIM 0

// Switches turns (just in case more players are added)
#define turnCycle() turn = (turn == X) ? O : X;

// updates the rgb vector and sends it to the gpu.
#define update_rgb(r, g, b)	glUniform3f(rgbLoc, r, g, b);

// updates camera light pos and sends it to the gpu.
#define update_camera_light_pos(x, y, z) glUniform3f(cameraLightPosLoc, x, y, z);

Board board;
Turn turn;
std::vector<std::array<int, 4>> moveHistory;
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
unsigned int BOARD_VAO, BOARD_VBO; // board
unsigned int X_VAO, X_VBO; // x
unsigned int O_VAO, O_VBO; // o
GLuint shaderProgram;

glm::mat4 Model, View, Projection, mvp;
glm::mat3 Normal;
unsigned int modelLoc, normalLoc, mvpLoc, rgbLoc, cameraLightPosLoc;

float theta, phi, radius; // theta on xz plane, 0 at +x. phi on y axis, 0 is horizontal (on xz plane)
double mouseStartX, mouseStartY, startTheta, startPhi; // for camera orbit calculations


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
	update_camera_light_pos(cameraX, cameraY, cameraZ);
}

// updates the mvp matrix and related values and sends them to the gpu.
void update_mvp() {
	mvp = Projection * View * Model;
	Normal = glm::transpose(glm::inverse(Model));

	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(Model));
	glUniformMatrix3fv(normalLoc, 1, GL_FALSE, glm::value_ptr(Normal));
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
}

void reset_camera() {
	theta = M_PI/2;
	phi = 0;
	radius = 4;

	mouseStartX = -1;
	mouseStartY = -1;
	startPhi = phi;
	startTheta = theta;
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
	if(!build_polygons()) {
		std::cerr << "Failed to build polygons" << std::endl;
		return false;
	}

	// Generate buffers
	// x buffers
	glGenVertexArrays(1, &X_VAO);
	glBindVertexArray(X_VAO);

	glGenBuffers(1, &X_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, X_VBO);
	glBufferData(GL_ARRAY_BUFFER, x_polygon_size, x_polygon, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// o buffers
	glGenVertexArrays(1, &O_VAO);
	glBindVertexArray(O_VAO);

	glGenBuffers(1, &O_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, O_VBO);
	glBufferData(GL_ARRAY_BUFFER, o_polygon_size, o_polygon, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// board buffers
	glGenVertexArrays(1, &BOARD_VAO);
	glBindVertexArray(BOARD_VAO);

	glGenBuffers(1, &BOARD_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, BOARD_VBO);
	glBufferData(GL_ARRAY_BUFFER, board_polygon_size, board_polygon, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*) (3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_CULL_FACE);


	// Load Shaders
	shaderProgram = LoadShaders("shaders/shader.vert", "shaders/shader.frag", nullptr);
	glUseProgram(shaderProgram);
	mvpLoc = glGetUniformLocation(shaderProgram, "mvp");
	modelLoc = glGetUniformLocation(shaderProgram, "model");
	normalLoc = glGetUniformLocation(shaderProgram, "normal");
	rgbLoc = glGetUniformLocation(shaderProgram, "rgb");
	cameraLightPosLoc = glGetUniformLocation(shaderProgram, "cameraLightPos");

	// Generate mvp and camera
	Projection = glm::perspective(glm::radians(45.0f), float(SCREEN_WIDTH)/float(SCREEN_HEIGHT), 0.1f, 100.0f);

	reset_camera();

	update_view();
	Model = glm::mat4(1.0f);
	update_mvp();

	return true;
}

void close() {
	glDeleteBuffers(1, &BOARD_VBO);
	glDeleteVertexArrays(1, &BOARD_VAO);

	glDeleteBuffers(1, &X_VBO);
	glDeleteVertexArrays(1, &X_VAO);

	glDeleteBuffers(1, &O_VBO);
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
					if(player == O) Model = glm::rotate(Model, 0.3f * lx + 0.5f * ly + 0.7f * lz, glm::vec3(0.9f * (lx - 1.1) + 1.1f * (ly - 1) + 1.3f * (lz - 1), 1.1f * (lx - 1) + 1.3f * (ly - 1) + 0.9f * (lz - 1), 1.3f * (lx - 1) + 0.9f * (ly - 1) + 1.1f * (lz - 1))); // rotate O model randomly based on location
					update_mvp();

					switch(player) {
						case X:
							glDrawArrays(GL_TRIANGLES, 0, 4*36);
							break;
						case O:
							glDrawArrays(GL_TRIANGLES, 0, 60);
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
	glBindVertexArray(BOARD_VAO);

	// set board color to either color of winning move or color of next move to be placed
	int local_w = won ? std::get<3>(moveHistory.at(moveHistoryIndex)) : w;

	// brighten if key_possible, dim if block_possible
	float mod = 0;
	if(!won && recommendation != 0) {
		mod = recommendation * BOARD_POSS_ADJUST;
	}

	update_rgb((local_w == 0) + mod, (local_w == 1) + mod, (local_w == 2) + mod);

	//glDrawArrays(GL_TRIANGLES, 0, 12 * 36);
	glDrawArrays(GL_TRIANGLES, 0, 12 * 36);
}

float recommend_keys() {
	board.clearRecs();
	if(won) return 0;

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
std::array<int, 4> automove() {
	board.clearRecs();

	if(board.possibleKeys(turn, true, 0)
			|| board.possibleBlocks(turn, true, 0)
			|| board.possibleKeys(turn, true, 1)
			|| board.possibleBlocks(turn, true, 1)) {
		for(int x = 0; x < 3; ++x) {
			for(int y = 0; y < 3; ++y) {
				for(int z = 0; z < 3; ++z) {
					for(int w = 0; w < 3; ++w) {
						if(board.get(x, y, z, w).state == CellState::KEY_POSS
								|| board.get(x, y, z, w).state == CellState::BLOCK_POSS
								|| board.get(x, y, z, w).state == CellState::KEY_POSS_1
								|| board.get(x, y, z, w).state == CellState::BLOCK_POSS_1) {
							recommend_keys();
							return { x, y, z, w };
						}
					}
				}
			}
		}
	}

	return { 0, 0, 0, 0 };
}

void undo() {
	if(moveHistoryIndex >= 0) {
		std::array<int, 4> priorMove = moveHistory.at(moveHistoryIndex--);
		board.remove(priorMove[0], priorMove[1], priorMove[2], priorMove[3]);
		turnCycle();
		recommend_keys();
	}
}
void redo() {
	if(moveHistoryIndex < (int) (moveHistory.size()) - 1) {
		std::array<int, 4> priorMove = moveHistory.at(++moveHistoryIndex);
		if(board.move(priorMove[0], priorMove[1], priorMove[2], priorMove[3], turn)) won = true;
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
int place_move(int x, int y, int z, int w) {
	if(board.get(x, y, z, w).state != CellState::PLACE) { // if valid move
		// add to move history
		moveHistory.erase(moveHistory.begin() + moveHistoryIndex + 1, moveHistory.end());
		moveHistory.push_back({x, y, z, w});
		moveHistoryIndex = moveHistory.size() - 1;

		// place move and check for a win
		if(board.move(x, y, z, w, turn)) {
			won = true;
			board.clearRecs();
			std::cout << board << std::endl;
		}

		// change turn
		turnCycle();

		// show recommendations
		recommend_keys();

		return 1;
	}

	return 0;
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if(action == GLFW_PRESS) {
		switch(key) {
			case QUIT:
				quit = true;
				std::cout << "quitting..." << std::endl;
				break;
			case RESTART:
				restart();
				std::cout << "restarting..." << std::endl << std::endl;
				break;
			case RESET_CAMERA:
				reset_camera();
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
				std::cout << "recommend key 0: " << key_recommend << std::endl;
				break;
			case RECOMMEND_BLOCK:
				block_recommend = !block_recommend;
				recommend_keys();
				std::cout << "recommend block 0: " << block_recommend << std::endl;
				break;
			case RECOMMEND_KEY_1:
				key_1_recommend = !key_1_recommend;
				recommend_keys();
				std::cout << "recommend key 1: " << key_1_recommend << std::endl;
				break;
			case RECOMMEND_BLOCK_1:
				block_1_recommend = !block_1_recommend;
				recommend_keys();
				std::cout << "recommend block 1: " << block_1_recommend << std::endl;
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
				if(!won) {
					if(key == AUTOMOVE_SINGLE) {
						std::array<int, 4> move = automove();
						place_move(move[0], move[1], move[2], move[3]);
					} else if(keybinds.find(key) != keybinds.end()) {
						std::array<int, 3> move = keybinds[key];
						place_move(move[0], move[1], move[2], w);
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
		theta = (xpos - mouseStartX) * PAN_SPEED + startTheta;
		phi = (ypos - mouseStartY) * PAN_SPEED + startPhi;

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
	startTheta = theta;
	startPhi = phi;
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
