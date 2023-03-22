// gbuffer.cpp

#include "headers.h"
#include "gbuffer.h"
#include "strokefont.h"
#include "shader.h"


GBuffer::GBuffer( unsigned int width, unsigned int height, 
		  int nTextures, int *textureInternalFormats, int *textureFormats, int *textureTypes, 
		  GLuint baseTexUnit, GLFWwindow *window )

{
  // Set up GPU program for debugging

  gpuProg = new GPUProgram();
  gpuProg->init( vertShader, fragShader, "gbuffer" );

  glGenVertexArrays( 1, &VAO );

  // Copy vars
    
  windowWidth = width;
  windowHeight = height;
  numTextures = nTextures;
  baseTextureUnit = baseTexUnit;
  texFormats = textureFormats;

  strokeFont = new StrokeFont();

  glfwGetFramebufferSize( window, &fbWidth, &fbHeight );

  // Create the FBO

  glGenFramebuffers( 1, &FBO );
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FBO );

  // Create the gbuffer textures

  textureIDs = new GLuint[ numTextures ];
  glGenTextures( numTextures, textureIDs );
  glGenTextures( 1, &depthTexture );

  for (int i = 0 ; i < numTextures; i++) {

    glBindTexture( GL_TEXTURE_2D, textureIDs[i] );

    glTexImage2D( GL_TEXTURE_2D, 0, textureInternalFormats[i], fbWidth, fbHeight, 0, textureFormats[i], textureTypes[i], NULL );

    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

    glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureIDs[i], 0 );
  }

  // depth

  glBindTexture( GL_TEXTURE_2D, depthTexture );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, fbWidth, fbHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL );

  glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0 );

  // Declare the drawBuffers

  GLenum *drawBuffers = new GLenum[numTextures];

  for (int i=0; i<numTextures; i++)
    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;

  glDrawBuffers( numTextures, drawBuffers );

  GLenum status = glCheckFramebufferStatus( GL_FRAMEBUFFER );

  if (status != GL_FRAMEBUFFER_COMPLETE) {

    printf("Framebuffer error: ");

    switch (status) {

    case GL_FRAMEBUFFER_UNDEFINED:
      cout << "GL_FRAMEBUFFER_UNDEFINED";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      cout << "GL_FRAMEBUFFER_UNSUPPORTED";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
      cout << "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
      break;
    }

    printf( "\n" );
    
    exit(1);
  }

  // restore default FBO

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
}


GBuffer::~GBuffer()

{
  glDeleteFramebuffers( 1, &FBO );
  glDeleteTextures( numTextures, textureIDs );
  glDeleteTextures( 1, &depthTexture );
}


void GBuffer::BindForWriting()

{
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, FBO );
}


void GBuffer::BindForReading()

{
  glBindFramebuffer( GL_READ_FRAMEBUFFER, FBO );
}


void GBuffer::BindTexture( int textureNumber, int gbTextureUnit )

{
  glActiveTexture( baseTextureUnit + gbTextureUnit + GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, textureIDs[ textureNumber ] );
}


void GBuffer::UnbindTexture( int textureNumber, int gbTextureUnit )

{
  glActiveTexture( baseTextureUnit + gbTextureUnit + GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


void GBuffer::SetReadBuffer( int textureNumber )

{
  glReadBuffer( GL_COLOR_ATTACHMENT0 + textureNumber );
}


void GBuffer::setDrawBuffers( int numDrawBuffers, int *bufferIDs )

{
  GLenum *drawBuffers = new GLenum[numDrawBuffers];

  for (int i=0; i<numDrawBuffers; i++)
    drawBuffers[i] = GL_COLOR_ATTACHMENT0 + bufferIDs[i];

  glDrawBuffers( numDrawBuffers, drawBuffers );

  delete [] drawBuffers;
}


// Debugging output

void GBuffer::DrawGBuffers( GLFWwindow *window, int numDrawBuffers, int *bufferIDs, char **bufferNames )

{
  // Clear window

  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Blit textures onto window

  glBindFramebuffer( GL_READ_FRAMEBUFFER, FBO );

  int dim = (int) ceil(sqrt(numDrawBuffers)); // use a dim x dim grid

  int i = 0;

  glDisable( GL_DEPTH_TEST );
	
  for (int r=0; r<dim; r++)
    for (int c=0; c<dim; c++)
      if (i < numDrawBuffers) {
	if (bufferIDs[i] >= 0) {

	  gpuProg->activate();

	  // Get vertex positions and texcoords

	  vec2 attributes[8] = {
	    vec2(     c/(float)dim*2-1, (dim-(r+1))/(float)dim*2-1 ), // positions in [-1,1]x[-1,1]
	    vec2( (c+1)/(float)dim*2-1, (dim-(r+1))/(float)dim*2-1 ),
	    vec2( (c+1)/(float)dim*2-1, (dim-r)    /(float)dim*2-1 ),
	    vec2(     c/(float)dim*2-1, (dim-r)    /(float)dim*2-1 ),
	    vec2( 0, 0 ),		// texcoords in [0,1]x[0,1]
	    vec2( 1, 0 ),
	    vec2( 1, 1 ),
	    vec2( 0, 1 )
	  };

	  // Set up a VAO
	  //
	  // This shouldn't be necessary, but MacOS complains otherwise
	  
	  glBindVertexArray( VAO );

	  // Set up a VBO

	  GLuint VBO;
	  glGenBuffers( 1, &VBO );
	  glBindBuffer( GL_ARRAY_BUFFER, VBO );
	  glBufferData( GL_ARRAY_BUFFER, sizeof(attributes), attributes, GL_STATIC_DRAW );

	  // Set up the attributes

	  glEnableVertexAttribArray( 0 ); // positions
	  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );

	  glEnableVertexAttribArray( 1 ); // texcoords
	  glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (const void*) (4*sizeof(vec2)) );

	  // Set up texture
	
	  glActiveTexture( baseTextureUnit + bufferIDs[i] + GL_TEXTURE0 );
	  gpuProg->setInt( "texUnitID", baseTextureUnit + bufferIDs[i] );

	  // Set flag to copy red channel to green and blue channels

	  if (texFormats[ bufferIDs[i] ] == GL_RED)
	    gpuProg->setInt( "copyRed", 1 );
	  else {
	    gpuProg->setInt( "copyRed", 0 );
	    gpuProg->setVec3( "scale", 1*vec3(1,1,1) );
	    gpuProg->setVec3( "bias",  0*vec3(1,1,1) );
	  }

	  // Draw

	  glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

	  // Free everything

	  glDisableVertexAttribArray( 0 );
	  glDisableVertexAttribArray( 1 );
	  glDeleteBuffers( 1, &VBO );
	  glBindVertexArray( 0 );

	  gpuProg->deactivate();

	  // Draw message

	  strokeFont->drawStrokeString( bufferNames[i], (c+0.5)/(0.5*dim)-1, (dim-r)/(0.5*dim)-1.1, 0.03, 0, CENTRE );
	}

	i++;
      }
}


char *GBuffer::vertShader = R"XX(

  #version 300 es
  precision mediump float;

  layout (location = 0) in vec2 vertPosition;
  layout (location = 1) in vec2 vertTexCoord;

  out vec2 texCoord;

  void main()

  {
     gl_Position = vec4( vertPosition, 0.0, 1.0 ); // already in [-1,1]x[-1,1]
     texCoord = vertTexCoord;
  }

)XX";



char *GBuffer::fragShader = R"XX(

  #version 300 es
  precision mediump float;

  uniform sampler2D texUnitID;

  uniform int copyRed;
  uniform vec3 scale;
  uniform vec3 bias;

  in vec2 texCoord;

  out vec4 fragColour;

  void main()

  {
    vec3 val = texture( texUnitID, texCoord ).rgb;

    if (copyRed == 1) {
      val.g = val.r;
      val.b = val.r;
    } else
      val = scale * val + bias;

    fragColour = vec4( val, 1.0 );
  }

)XX";
