/* sphere.h
 */


#ifndef SPHERE_H
#define SPHERE_H


#include "linalg.h"
#include "seq.h"
#include "object.h"
#include "rectangle.h"
#include "gpuProgram.h"


// icosahedron vertices (taken from Jon Leech http://www.cs.unc.edu/~jon)

#define tau 0.8506508084      /* t=(1+sqrt(5))/2, tau=t/sqrt(1+t^2)  */
#define one 0.5257311121      /* one=1/sqrt(1+t^2) , unit sphere     */

#define NUM_VERTS 12
#define NUM_FACES 20

#define SPHERE_DENSITY 1  // 1 kg/m^3


class SphereFace {
 public:
  unsigned int v[3];
  SphereFace() {}
  SphereFace( int v0, int v1, int v2 ) {
    v[0] = v0; v[1] = v1; v[2] = v2;
  }
};



typedef struct {
  float radius;
  vec3 centre;
} SphereDef;


class Sphere : public Object {

 public:

  float radius;

  float minDist;      // min distance to another object
  vec3  contactPoint; // when a collision occurs: contact point on another object
  
  seq<Rectangle*> constraintRectangles;  // rectangles on which the sphere is constrained to remain
  
 Sphere( int numLevels, float radius, vec3 position, quaternion orientation, vec3 velocity, vec3 angVelocity )

    : Object( position, orientation, velocity, angVelocity ) 

    {
      this->radius = radius;
      this->minDist = FLT_MAX;
      
      for (int i=0; i<NUM_VERTS; i++)
	verts.add( icosahedronVerts[i] );

      for (int i=0; i<verts.size(); i++)
	verts[i] = verts[i].normalize();

      for (int i=0; i<NUM_FACES; i++)
	faces.add( SphereFace( icosahedronFaces[i][0],
			       icosahedronFaces[i][1],
			       icosahedronFaces[i][2] ) );

      for (int i=0; i<numLevels; i++)
	refine();

      gpu = new GPUProgram();
      gpu->init( vertShader, fragShader, "in sphere.cpp" );

      setupVAO();
    };

  Sphere() {}

  ~Sphere() {}

  void draw( mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, vec3 &colour );

  float distToSphere( Sphere &otherSphere );

  float distToRectangle( Rectangle &rectangle, vec3 *closestPoint );

  float mass() {
    return SPHERE_DENSITY * (4.0/3.0) * 3.14159 * radius * radius * radius;
  }

 private:

  seq<vec3>       verts;
  seq<SphereFace> faces;
  GLuint          VAO; 

  GPUProgram      *gpu;

  static const char *vertShader;
  static const char *fragShader;

  void refine();
  void setupVAO();

  static vec3 icosahedronVerts[NUM_VERTS];
  static int icosahedronFaces[NUM_FACES][3];
};

#endif
