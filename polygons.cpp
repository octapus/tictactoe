#include "polygons.hpp"

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

// A rectangular prism of length/width 2*LINE_RADIUS and height 2 centered on the origin
const GLfloat line_vertices[] = {
	-LINE_RADIUS,  1.0f, LINE_RADIUS, // top left		0
	LINE_RADIUS,  1.0f, LINE_RADIUS, // top right		1
	-LINE_RADIUS, -1.0f, LINE_RADIUS, // bottom left	2
	LINE_RADIUS, -1.0f, LINE_RADIUS, // bottom right	3

	-LINE_RADIUS,  1.0f, -LINE_RADIUS,   // top left	4
	LINE_RADIUS,  1.0f, -LINE_RADIUS, // top right		5
	-LINE_RADIUS, -1.0f, -LINE_RADIUS, // bottom left	6
	LINE_RADIUS, -1.0f, -LINE_RADIUS // bottom right	7
};
const GLuint line_indices[] {
	1, 0, 3, 0, 2, 3, // front
	7, 4, 5, 7, 6, 4, // back
	0, 6, 2, 0, 4, 6, // left
	1, 3, 5, 5, 3, 7, // right
	0, 1, 5, 0, 5, 4, // top
	3, 2, 7, 2, 6, 7 // bottom
};

// Values stolen off the internet
const GLfloat icos_x = .525731112119133606f * (SCALE - 2 *PADDING);
const GLfloat icos_z = .850650808352039932f * (SCALE - 2 * PADDING);
const GLfloat icos_n = 0.f;
GLfloat icosahedron_vertices[]= {
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
GLuint icosahedron_indicies[] = {
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


GLfloat board_vertices[12*24];
GLuint board_indices[12*36];
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

size_t x_vertices_size = sizeof(x_vertices);
size_t x_indices_size = sizeof(x_indices);

size_t icosahedron_vertices_size = sizeof(icosahedron_vertices);
size_t icosahedron_indicies_size = sizeof(icosahedron_indicies);

size_t board_vertices_size = sizeof(board_vertices);
size_t board_indices_size = sizeof(board_indices);
