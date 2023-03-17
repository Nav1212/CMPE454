/* rectangle.h
 */


#ifndef RECTANGLE_H
#define RECTANGLE_H


#include "linalg.h"
#include "seq.h"
#include "object.h"
#include "gpuProgram.h"


typedef struct {
  float xDim, yDim;
  vec3 normal, centre;
} RectangleDef;


class Rectangle : public Object {

 public:

  float xDim;
  float yDim;
  vec3  normal;
  vec3  centre;

  Rectangle( float xDim, float yDim, vec3 normal, vec3 centre, quaternion orientation, vec3 velocity, vec3 angVelocity )

    : Object( centre, orientation, velocity, angVelocity ) 

    {
      this->xDim = xDim;
      this->yDim = yDim;
      this->centre = centre;
      this->normal = normal.normalize(); // (just in case)

      // Find orientation
      
      vec3 axis = vec3(0,0,1) ^ normal;
      float angle; 

      if (axis.length() < 0.0001) {
	axis = vec3(1,0,0);
	angle = 0;
      } else
	angle = asin( axis.length() );
      
      this->state.q = quaternion( angle, axis );

      // Set up shaders

      gpu = new GPUProgram();
      gpu->init( vertShader, fragShader, "in rectangle.cpp" );

      setupVAO();
    };

  Rectangle() {}

  ~Rectangle() {}

  void draw( mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, vec3 &colour );

  float mass() {
    return 99999; // hack for an immovable object
  }

 private:

  GLuint          VAO;

  GPUProgram      *gpu;

  static const vec3 verts[4];
  static const vec3 norms[4];

  static const char *vertShader;
  static const char *fragShader;

  void setupVAO();
};

#endif
