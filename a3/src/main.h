// main.h

#ifndef SHADER_H
#define SHADER_H

#include "linalg.h"
#include "drawSegs.h"

extern GLuint windowWidth, windowHeight;
extern float fovy;
extern vec3 eyePosition;
extern vec3 lookAt;
extern float worldRadius;
extern bool showAxes;
extern bool showClosest;
extern bool sleeping;
extern Segs *segs;
extern float timeFactor;

void glErrorReport( char *where );
float getTime();

#endif
