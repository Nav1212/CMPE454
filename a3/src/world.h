// world.h

#ifndef WORLD_H

#include "headers.h"
#include "sphere.h"
#include "rectangle.h"
#include "seq.h"



#define SPHERE_LEVELS 3  // number of times sphere is refined from original dodecahedron.

#define WORLD_RADIUS 6


class World {

  seq<Sphere> spheres;
  seq<Rectangle> rectangles;

  static const SphereDef    initSpheres[];
  static const RectangleDef initRectangles[];

 public:

  World( char *sphereFilename );

  void updateState( float elapsedTime );

  float updateStateByDeltaT( float deltaT );

  void draw( mat4 WCS_to_VCS, mat4 VCS_to_CCS, vec3 &lightDir );
  quaternion orientationDeriv( quaternion q, vec3 w );
  bool findCollisions( Sphere **collisionSphere, Object **collisionObject );
  void integrate( State *yStart, State *yEnd, float deltaT, bool &collisionAtEnd, Sphere **collsionSphere, Object **collisionObject );
  void resolveCollision( Sphere *collisionSphere, Object *collisionObject );

  void copyState( Sphere *fromSpheres, State *toState ) {
    for (int i=0; i<spheres.size(); i++)
      toState[i] = fromSpheres[i].state;
  }

  void copyState( State *fromState, Sphere *toSpheres ) {
    for (int i=0; i<spheres.size(); i++)
      toSpheres[i].state = fromState[i];
  }

  void copyState( State *fromState, State *toState ) {
    for (int i=0; i<spheres.size(); i++)
      toState[i] = fromState[i];
  }
  
};

#endif
