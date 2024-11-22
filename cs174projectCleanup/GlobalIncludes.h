#ifndef GUARD_GlobalIncludes_h
#define GUARD_GlobalIncludes_h

#include <stdio.h>
#include <stdlib.h>

#include<string>
#include<vector>
#include<iostream>
#include "Exception.h"
#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#include "GL/glew.h"
#endif
#include "Angel.h"
#include "obj_loader/objLoader.h"
#endif
