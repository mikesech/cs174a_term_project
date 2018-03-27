#ifndef GUARD_GlobalIncludes_h
#define GUARD_GlobalIncludes_h

#include <stdio.h>
#include <stdlib.h>

#include<string>
#include<vector>
#include<iostream>
#include "Exception.h"
#include "GL/glew.h"
#ifdef __APPLE__
// freeglut doesn't work well on osx. We'll use the Apple-provided original GLUT instead.
#	include "GLUT/glut.h"
#else
#	include "GL/freeglut.h"
#	include "GL/freeglut_ext.h"
#endif
#include "IL/il.h"
#include "Angel.h"
#include "obj_loader/objLoader.h"
#endif