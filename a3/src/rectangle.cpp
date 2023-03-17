// rectangle.cpp

#include "rectangle.h"


void Rectangle::setupVAO()

{
  // Create a VAO

  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  // store vertices

  GLuint vertexBufferID;
  glGenBuffers( 1, &vertexBufferID );
  glBindBuffer( GL_ARRAY_BUFFER, vertexBufferID );

  glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(vec3), (void*) &verts[0], GL_STATIC_DRAW );

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

  // store normals

  GLuint normalBufferID;
  glGenBuffers( 1, &normalBufferID );
  glBindBuffer( GL_ARRAY_BUFFER, normalBufferID );

  glBufferData( GL_ARRAY_BUFFER, 4 * sizeof(vec3), (void*) &norms[0], GL_STATIC_DRAW );

  glEnableVertexAttribArray( 1 );
  glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, 0 );

  // Done
  
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindVertexArray( 0 );
}


void Rectangle::draw( mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, vec3 &colour )

{
  mat4 MV  = WCS_to_VCS * OCS_to_WCS() * scale( xDim, yDim, 1 );
  mat4 MVP = VCS_to_CCS * MV;

  gpu->activate();

  gpu->setMat4( "MV", MV );
  gpu->setMat4( "MVP", MVP );
  gpu->setVec3( "colour", colour );
  gpu->setVec3( "lightDir", lightDir );
  
  glBindVertexArray( VAO );
  glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
  glBindVertexArray( 0 );

  gpu->deactivate();
}


const vec3 Rectangle::verts[4] = { // unit square centred at (0,0,0) with normal (0,0,1)
  vec3(-0.5,-0.5,0), 
  vec3( 0.5,-0.5,0), 
  vec3( 0.5, 0.5,0), 
  vec3(-0.5, 0.5,0) 
};

const vec3 Rectangle::norms[4] = {
  vec3(0,0,1),
  vec3(0,0,1),
  vec3(0,0,1),
  vec3(0,0,1)
};

const char *Rectangle::vertShader = R"XX(

  #version 300 es

  precision mediump float;

  uniform mat4 MVP;
  uniform mat4 MV;

  layout (location = 0) in vec3 vertPosition;
  layout (location = 1) in vec3 vertNormal;

  smooth out vec3 normal;

  void main() {

    gl_Position = MVP * vec4( vertPosition, 1.0 );
    normal = vec3( MV * vec4( vertNormal, 0.0 ) );
  }
)XX";


const char *Rectangle::fragShader = R"XX(

  #version 300 es

  precision mediump float;

  uniform vec3 colour;
  uniform vec3 lightDir;

  smooth in vec3 normal;
  out vec4 outputColour;

  void main() {

    float NdotL = dot( normalize(normal), lightDir );

    if (NdotL < 0.0)
      NdotL = 0.3;
    else
      NdotL = 0.3 + 0.7 * NdotL;

    outputColour = vec4( NdotL * colour, 1.0 );
  }
)XX";
