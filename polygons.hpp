#ifndef __POLYGONS__
#define __POLYGONS__
#include<GL/glew.h>

#define LINE_RADIUS 0.01f
#define SCALE 0.33333333333
#define PADDING 0.0625

extern GLfloat board_polygon[];
extern size_t board_polygon_size;

extern GLfloat x_polygon[];
extern size_t x_polygon_size;

extern GLfloat o_polygon[];
extern size_t o_polygon_size;

bool build_polygons();

#endif
