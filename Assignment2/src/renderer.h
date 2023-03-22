// G-buffer renderer

#ifndef RENDER_H
#define RENDER_H


#include "wavefront.h"
#include "gpuProgram.h"
#include "gbuffer.h"
#include "shader.h"


#define BASE_GBUFFER_TEXTURE_UNIT 2 // texture units 0 and 1 used elsewhere

#define GPU_NUM_PASSES 4

#define NUM_SAMPLE_OFFSETS 1000


class Renderer {

  enum { POSITION_GBUFFER  = 0,
         NORMAL_GBUFFER    = 1,
	 DEPTH_GBUFFER     = 2,
	 OCCLUSION_GBUFFER = 3,
	 BLURRED_GBUFFER   = 4,
	 NUM_GBUFFERS      = 5 };

  char *gbufferNames[5] = { "VCS position", "VCS normal", "0-1 depth", "0-1 occlusion factor", "0-1 blurred occlusion factor" };

  //                                           position    normal      depth      occlusion  blurred occlusion

  int textureInternalFormats[NUM_GBUFFERS] = { GL_RGB16F,  GL_RGB16F,  GL_R16F,   GL_R16F,   GL_R16F   };
  int textureFormats[NUM_GBUFFERS]         = { GL_RGB,     GL_RGB,     GL_RED,    GL_RED,    GL_RED    };
  int textureTypes[NUM_GBUFFERS]           = { GL_FLOAT,   GL_FLOAT,   GL_FLOAT,  GL_FLOAT,  GL_FLOAT  };

  // GL_UNSIGNED_BYTE, 

  GPUProgram *pass1Prog, *pass2Prog, *pass3Prog, *pass4Prog;
  GBuffer    *gbuffer;

 public:

  int debug;

  Renderer( int windowWidth, int windowHeight, const char* shaderDir, GLFWwindow *window  ) {

    gbuffer = new GBuffer( windowWidth, windowHeight, NUM_GBUFFERS, textureInternalFormats, textureFormats, textureTypes, BASE_GBUFFER_TEXTURE_UNIT, window );

    if (chdir( shaderDir ) != 0) {
      char path[PATH_MAX];
      getcwd( path, PATH_MAX );
      std::cerr << "Failed to change directory to " << shaderDir << ".  Current working directory is " << path << std::endl;
      exit(1);
    }

    pass1Prog = new GPUProgram( "pass1.vert", "pass1.frag", "pass 1" );
    pass2Prog = new GPUProgram( "pass2.vert", "pass2.frag", "pass 2" );
    pass3Prog = new GPUProgram( "pass3.vert", "pass3.frag", "pass 3" );
    pass4Prog = new GPUProgram( "pass4.vert", "pass4.frag", "pass 4" );

    debug = 0;
  }

  ~Renderer() {
    delete gbuffer;
    delete pass2Prog;
    delete pass1Prog;
  }

  void reshape( int windowWidth, int windowHeight, GLFWwindow *window ) {
    delete gbuffer;
    gbuffer = new GBuffer( windowWidth, windowHeight, NUM_GBUFFERS, textureInternalFormats, textureFormats, textureTypes, BASE_GBUFFER_TEXTURE_UNIT, window );
  }

  void render( seq<wfModel *> &objs, mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, float worldRadius, GLFWwindow *window ); 

  void incDebug() {
    debug = (debug+1) % GPU_NUM_PASSES;
  }

  bool debugOn() {
    return (debug > 0);
  }

  void makeStatusMessage( char *buffer ) {
    sprintf( buffer, "After pass %d: %d samples, %.1f sample distance, %d blur radius", debug+1, numSampleOffsetsToUse, offsetScale, (int) blurRadius );
  }
};

#endif
