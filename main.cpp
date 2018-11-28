#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "shaders/loadShaders.hpp"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

#include<iostream>
#include<unordered_map>
#include<vector>
#include "board.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 800
#define LINE_RADIUS 0.01f
#define SCALE 0.33333333333
#define PADDING 0.0625

#define PAN_SPEED 0.0001f
#define ZOOM_SPEED 0.1

#define QUIT GLFW_KEY_ESCAPE
#define RESTART GLFW_KEY_DELETE
#define RESET_CAMERA GLFW_KEY_ENTER

#define UNDO GLFW_KEY_LEFT
#define REDO GLFW_KEY_RIGHT

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

#define W_UP GLFW_KEY_SPACE
#define W_DOWN GLFW_KEY_LEFT_SHIFT

Board board;
CellState turn;
std::vector<std::tuple<int, int, int, int>> moveHistory;
int moveHistoryIndex;
int w;
bool quit;
bool won;
std::unordered_map<int, std::tuple<int, int, int>> keybinds;

GLFWwindow *window;
unsigned int VAO, VBO, EBO; // board
unsigned int X_VAO, X_VBO, X_EBO; // x
unsigned int O_VAO, O_VBO, O_EBO; // o
unsigned int CROSS_VAO, CROSS_VBO, CROSS_EBO; // cross
unsigned int TET_VAO, TET_VBO, TET_EBO; // tetrahedron
GLuint shaderProgram;

glm::mat4 Model, View, Projection, mvp;
unsigned int mvpLoc, rgbLoc;

float theta, phi, radius; // theta on xz plane, 0 at +x. phi on y axis, 0 is horizontal (on xz plane)
double mouseStartX, mouseStartY; // for camera orbit calculations

// A rectangular prism of length/width 2*LINE_RADIUS and height 2 centered on the origin
GLfloat board_vertices[12*24];
GLuint board_indices[12*36];
static const GLfloat line_vertices[] = {
	-LINE_RADIUS,  1.0f, LINE_RADIUS, // top left		0
	LINE_RADIUS,  1.0f, LINE_RADIUS, // top right		1
	-LINE_RADIUS, -1.0f, LINE_RADIUS, // bottom left	2
	LINE_RADIUS, -1.0f, LINE_RADIUS, // bottom right	3

	-LINE_RADIUS,  1.0f, -LINE_RADIUS,   // top left	4
	LINE_RADIUS,  1.0f, -LINE_RADIUS, // top right		5
	-LINE_RADIUS, -1.0f, -LINE_RADIUS, // bottom left	6
	LINE_RADIUS, -1.0f, -LINE_RADIUS // bottom right	7
};
static const GLuint line_indices[] {
	1, 0, 3, 0, 2, 3, // front
	7, 4, 5, 7, 6, 4, // back
	0, 6, 2, 0, 4, 6, // left
	1, 3, 5, 5, 3, 7, // right
	0, 1, 5, 0, 5, 4, // top
	3, 2, 7, 2, 6, 7 // bottom
};

// from wikipedia
static const GLfloat tet_a = 1 * (SCALE - PADDING) - PADDING;
static const GLfloat tet_b = 1/3.0 * (SCALE - PADDING) + PADDING;
static const GLfloat tet_c = sqrt(2/3.0) * (SCALE - PADDING);
static const GLfloat tet_d = sqrt(2/9.0) * (SCALE - PADDING);
static const GLfloat tet_e = sqrt(8/9.0) * (SCALE - PADDING);
static const GLfloat tetrahedron_vertices[] = {
	0, tet_a, 0, // top 					0
	0, -tet_b, tet_e, // front				1
	tet_c, -tet_b, -tet_d, // back right	2
	-tet_c, -tet_b, -tet_d // back left		3
};
// triangle strip
static const GLuint tetrahedron_indices[] = {
	0, 1, 2, 3, 0, 1
};

// Values stolen off the internet
static const GLfloat icos_x = .525731112119133606f * (SCALE - 2 *PADDING);
static const GLfloat icos_z = .850650808352039932f * (SCALE - 2 * PADDING);
static const GLfloat icos_n = 0.f;
static const GLfloat icosahedron_vertices[]= {
	icos_n, -icos_x, icos_z,             // vertices[0]
	icos_z, icos_n, icos_x,              // vertices[1]
	icos_z, icos_n, -icos_x,             // vertices[2]
	-icos_z, icos_n, -icos_x,            // vertices[3]
	-icos_z, icos_n, icos_x,             // vertices[4]
	-icos_x, icos_z, icos_n,             // vertices[5]
	icos_x, icos_z, icos_n,              // vertices[6]
	icos_x, -icos_z, icos_n,             // vertices[7]
	-icos_x, -icos_z, icos_n,            // vertices[8]
	icos_n, -icos_x, -icos_z,            // vertices[9]
	icos_n, icos_x, -icos_z,             // vertices[10]
	icos_n, icos_x, icos_z              // vertices[11]
};
static const GLuint icosahedron_indicies[] = {
	1, 2, 6,
	1, 7, 2,
	3, 4, 5,
	4, 3, 8,
	6, 5, 11,
	5, 6, 10,
	9, 10, 2,
	10, 9, 3,
	7, 8, 9,
	8, 7, 0,
	11, 0, 1,
	0, 11, 4,
	6, 2, 10,
	1, 6, 11,
	3, 5, 10,
	5, 4, 11,
	2, 7, 9,
	7, 1, 0,
	3, 9, 8,
	4, 8, 0
};


// use 4 translations of the line to make a section of the board. Takes in rx, ry, and rz for rotation.
void generate_line_group(int offset, int rx, int ry, int rz) {
	int currLine = offset * 4;
	for(int dx = -1; dx <= 1; dx += 2) {
		for(int dz = -1; dz <= 1; dz += 2) {
			for(int i = 0; i < 24; i += 3) {
				board_vertices[currLine * 24 + i + rx] = line_vertices[i] + SCALE * dx;
				board_vertices[currLine * 24 + i + ry] = line_vertices[i + 1];
				board_vertices[currLine * 24 + i + rz] = line_vertices[i + 2] + SCALE * dz;
			}

			for(int i = 0; i < 36; ++i) {
				board_indices[currLine * 36 + i] = line_indices[i] + 8 * currLine;
			}
			++currLine;
		}
	}
}
void generate_board_bg() {
	generate_line_group(0, 0, 1, 2);
	generate_line_group(1, 1, 0, 2);
	generate_line_group(2, 1, 2, 0);
}

GLfloat cross_vertices[3*24];
GLuint cross_indices[3*36];
// use 3 rotations of the line to create a cross
void generate_cross_polygon() {
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1, SCALE - PADDING, 1));
	glm::mat4 rotations[3];
	rotations[0] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
	rotations[1] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 1, 0));
	rotations[2] = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1, 0, 0));

	// generate vertices
	for(int i = 0; i < 24; i += 3) {
		glm::vec4 in = scale * glm::vec4(line_vertices[i], line_vertices[i + 1], line_vertices[i + 2], 1.0);
		for(int bar = 0; bar < 3; ++bar) {
			glm::vec4 out = rotations[bar] * in;
			cross_vertices[bar * 24 + i + 0] = out.x;
			cross_vertices[bar * 24 + i + 1] = out.y;
			cross_vertices[bar * 24 + i + 2] = out.z;
		}
	}

	// generate indices
	for(int currLine = 0; currLine < 3; ++currLine) {
		for(int i = 0; i < 36; ++i) {
			cross_indices[currLine * 36 + i] = line_indices[i] + 8 * currLine;
		}
	}
}

GLfloat x_vertices[4*24];
GLuint x_indices[4*36];
// uses 4 rotations of the line to create an x
void generate_x_polygon() {
	glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(1, SCALE - PADDING, 1));
	glm::mat4 rotations[4];

	rotations[0] = glm::rotate(glm::mat4(1.0f), glm::radians(55.0f), glm::vec3(1, 0, 1));
	rotations[1] = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(1, 0, 1));
	rotations[2] = glm::rotate(glm::mat4(1.0f), glm::radians(55.0f), glm::vec3(-1, 0, 1));
	rotations[3] = glm::rotate(glm::mat4(1.0f), glm::radians(-55.0f), glm::vec3(-1, 0, 1));

	// generate vertices
	for(int i = 0; i < 24; i += 3) {
		glm::vec4 in = scale * glm::vec4(line_vertices[i], line_vertices[i + 1], line_vertices[i + 2], 1.0);
		for(int bar = 0; bar < 4; ++bar) {
			glm::vec4 out = rotations[bar] * in;
			x_vertices[bar * 24 + i + 0] = out.x;
			x_vertices[bar * 24 + i + 1] = out.y;
			x_vertices[bar * 24 + i + 2] = out.z;
		}
	}

	// generate indices
	for(int currLine = 0; currLine < 4; ++currLine) {
		for(int i = 0; i < 36; ++i) {
			x_indices[currLine * 36 + i] = line_indices[i] + 8 * currLine;
		}
	}
}

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

// updates the mvp matrix and sends it to the gpu.
void update_mvp() {
	mvp = Projection * View * Model;
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp));
}

// updates the rgb vector and sends it to the gpu.
void update_rgb(float r, float g, float b) {
	glUniform3f(rgbLoc, r, g, b);
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
	generate_cross_polygon();
	generate_x_polygon();

	// Generate buffers
	// x buffers
	glGenVertexArrays(1, &X_VAO);
	glBindVertexArray(X_VAO);

	glGenBuffers(1, &X_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, X_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(x_vertices), x_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &X_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, X_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(x_indices), x_indices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// o buffers
	glGenVertexArrays(1, &O_VAO);
	glBindVertexArray(O_VAO);

	glGenBuffers(1, &O_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, O_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(icosahedron_vertices), icosahedron_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &O_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, O_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(icosahedron_indicies), icosahedron_indicies, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// cross buffers
	glGenVertexArrays(1, &CROSS_VAO);
	glBindVertexArray(CROSS_VAO);

	glGenBuffers(1, &CROSS_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, CROSS_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cross_vertices), cross_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &CROSS_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CROSS_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cross_indices), cross_indices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// tetrahedron buffers
	glGenVertexArrays(1, &TET_VAO);
	glBindVertexArray(TET_VAO);

	glGenBuffers(1, &TET_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, TET_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tetrahedron_vertices), tetrahedron_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &TET_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TET_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tetrahedron_indices), tetrahedron_indices, GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0);

	// board buffers
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(board_vertices), board_vertices, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*) 0);

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(board_indices), board_indices, GL_DYNAMIC_DRAW);

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

	glDeleteBuffers(1, &CROSS_VBO);
	glDeleteBuffers(1, &CROSS_EBO);
	glDeleteVertexArrays(1, &CROSS_VAO);

	glDeleteBuffers(1, &TET_VBO);
	glDeleteBuffers(1, &TET_EBO);
	glDeleteVertexArrays(1, &TET_VAO);

	glDeleteProgram(shaderProgram);

	glfwDestroyWindow(window);
	glfwTerminate();
}

// draws the moves of a specific player
void draw_specific_moves(CellState player) {
	// select correct VAO
	switch(player) {
		case X:
			glBindVertexArray(X_VAO);
			break;
		case O:
			glBindVertexArray(O_VAO);
			break;
		case WIN:
			glBindVertexArray(TET_VAO);
			break;
		case KEY:
			glBindVertexArray(CROSS_VAO);
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
				bool r = (board.get(lx, ly, lz, 0) == player);
				bool g = (board.get(lx, ly, lz, 1) == player);
				bool b = (board.get(lx, ly, lz, 2) == player);

				if(r || g || b) {
					update_rgb(r, g, b);

					Model = glm::translate(glm::mat4(1.0f), glm::vec3(2*SCALE*(lx-1), -2*SCALE*(ly-1), 2*SCALE*(lz-1)));
					update_mvp();

					switch(player) {
						case X:
							glDrawElements(GL_TRIANGLES, 4*36, GL_UNSIGNED_INT, 0);
							break;
						case O:
							glDrawElements(GL_TRIANGLES, 60, GL_UNSIGNED_INT, 0);
							break;
						case WIN:
							glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, 0);
							break;
						case KEY:
							glDrawElements(GL_TRIANGLES, 3*36, GL_UNSIGNED_INT, 0);
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

void draw_moves() {
	draw_specific_moves(X);
	draw_specific_moves(O);

	if(won) {
		draw_specific_moves(WIN);
		draw_specific_moves(KEY);
	}
}

void draw_board_background() {
	glBindVertexArray(VAO);
	switch(w) {
		case 0:
			update_rgb(1, 0, 0);
			break;
		case 1:
			update_rgb(0, 1, 0);
			break;
		case 2:
			update_rgb(0, 0, 1);
			break;
	}
	glDrawElements(GL_TRIANGLES, 12*36, GL_UNSIGNED_INT, 0);
}

void draw_board() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw_board_background();
	draw_moves();
	glfwSwapBuffers(window);
}

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

void turnCycle() {
	if(turn == X) turn = O;
	else turn = X;
}
void undo() {
	if(moveHistoryIndex >= 0) {
		std::tuple<int, int, int, int> priorMove = moveHistory.at(moveHistoryIndex--);
		board.remove(std::get<0>(priorMove), std::get<1>(priorMove), std::get<2>(priorMove), std::get<3>(priorMove));
		turnCycle();
	}
}
void redo() {
	if(moveHistoryIndex < (int) (moveHistory.size()) - 1) {
		std::tuple<int, int, int, int> priorMove = moveHistory.at(++moveHistoryIndex);
		if(board.move(std::get<0>(priorMove), std::get<1>(priorMove), std::get<2>(priorMove), std::get<3>(priorMove), turn)) won = true;
		turnCycle();
	}
}
void restart() {
	board.clear();
	turn = X;
	won = false;
	moveHistoryIndex = -1;
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
				w = 2;
				break;
			case W_DOWN:
				w = 0;
				break;
			case UNDO:
				if(!won) {
					undo();
				} else {
					// replay game to reset KEY/WIN
					restart();
					while(moveHistoryIndex < (int) (moveHistory.size()) - 2) {
						redo();
					}
				}
				break;
			case REDO:
				redo();
				break;
			default:
				if(!won && keybinds.find(key) != keybinds.end()) {
					std::tuple<int, int, int> move = keybinds[key];
					if(board.get(std::get<0>(move), std::get<1>(move), std::get<2>(move), w) == EMPTY) { // if valid move
						// add to move history
						moveHistory.erase(moveHistory.begin() + moveHistoryIndex + 1, moveHistory.end());
						moveHistory.push_back(std::make_tuple(std::get<0>(move), std::get<1>(move), std::get<2>(move), w));
						moveHistoryIndex = moveHistory.size() - 1;

						// place move and check for a win
						if(board.move(std::get<0>(move), std::get<1>(move), std::get<2>(move), w, turn)) {
							won = true;
							std::cout << turn << std::endl;
							std::cout << board << std::endl;
							break;
						}

						// change turn
						turnCycle();
					}
				}
				break;
		}
	} else if(action == GLFW_RELEASE) {
		if(key == W_UP || key == W_DOWN) w = 1;
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
			draw_board();
		}

		return EXIT_SUCCESS;
	}
}
