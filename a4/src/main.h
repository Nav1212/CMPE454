/* main.h
 */


#ifndef MAIN_H
#define MAIN_H


#include "seq.h"
#include "scene.h"
#include "rtWindow.h"
#include "gpuProgram.h"
#include "pixelZoom.h"
#include "strokefont.h"


void skipComments( istream &in );
float getTime();

extern int lineNum;
extern Scene *scene;
extern RTwindow *win;
extern PixelZoom *pixelZoom;
extern StrokeFont *strokeFont;

extern int windowWidth;
extern int windowHeight;

#endif
