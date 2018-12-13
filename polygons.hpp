#ifndef __POLYGONS__
#define __POLYGONS__
#include<GL/glew.h>

#define LINE_RADIUS 0.01f
#define SCALE 0.33333333333
#define PADDING 0.0625

extern GLfloat x_vertices[];
extern size_t x_vertices_size;
extern GLuint x_indices[];
extern size_t x_indices_size;

extern GLfloat icosahedron_vertices[];
extern size_t icosahedron_vertices_size;
extern GLuint icosahedron_indicies[];
extern size_t icosahedron_indicies_size;

extern GLfloat board_vertices[];
extern size_t board_vertices_size;
extern GLuint board_indices[];
extern size_t board_indices_size;

void generate_board_bg();
void generate_x_polygon();

#endif
