// gbuffer.h

#ifndef GBUFFER_H
#define	GBUFFER_H

#include "strokefont.h"
#include "gpuProgram.h"


class GBuffer

{
  GLuint FBO;
  GLuint VAO;
  GLuint depthTexture;

  unsigned int windowWidth, windowHeight;

  int baseTextureUnit;
  int numTextures;
  int *texFormats;

  StrokeFont *strokeFont;

  GPUProgram *gpuProg;
  static char *vertShader;
  static char *fragShader;

 public:

  GLuint *textureIDs;

  int fbWidth, fbHeight;

  GBuffer( unsigned int width, unsigned int height, int nTextures, int *textureInternalFormats, int *textureFormats, int *textureTypes, GLuint baseTexUnit, GLFWwindow *window );

  ~GBuffer();

  void BindForWriting();
  void BindForReading();  
  void BindTexture( int textureNumber, int gbTextureUnit );
  void UnbindTexture( int textureNumber, int gbTextureUnit );
    
  void SetReadBuffer( int textureNumber );

  void setDrawBuffers( int numDrawBuffers, int *bufferIDs );

  void DrawGBuffers( GLFWwindow *window, int numDrawBuffers, int *bufferIDs, char **bufferNames );
};

#endif	/* SHADOWMAPFBO_H */

