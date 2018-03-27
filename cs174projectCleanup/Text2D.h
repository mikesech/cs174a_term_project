#ifndef _TEXT2D
#define _TEXT2D

#ifdef WIN32
#include <windows.h>
#endif

#include "GL/glew.h"
#ifdef __APPLE__
// freeglut doesn't work well on osx. We'll use the Apple-provided original GLUT instead.
#	include "GLUT/glut.h"
#else
#	include "GL/freeglut.h"
#	include "GL/freeglut_ext.h"
#endif
#include "mat.h"
#include "vec.h"
#include "Angel.h"

/** @brief Helper class for 2-D text display. Doesn't work on some computers
*/
class Text2D
{
	public:
		Text2D(void);
		void print_bitmap_string(void* font, const char* s);
		void draw_stuff(const char* s, vec4 color, GLfloat location_x, GLfloat location_y );

};

#endif