// object.h

#ifndef OBJECT_H
#define OBJECT_H


#include "headers.h"
#include "linalg.h"


class State {

public:

  vec3       x;  // position
  quaternion q;  // orientation
  vec3       v;  // velocity
  vec3       w;  // angVelocity ("omega")
};


std::ostream& operator << ( std::ostream& stream, State const & state );
std::istream& operator >> ( std::istream& stream, State & state );


class Object {

public:

  State state;

  Object() {}
  
  Object( vec3 x, quaternion q, vec3 v, vec3 w ) {

    state.x = x;
    state.q = q;
    state.v = v;
    state.w = w;
  }

  virtual void draw( mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, vec3 &colour ) = 0;

  virtual float mass() = 0;

  mat4 OCS_to_WCS() {
    return translate(state.x) * state.q.toMatrix();
  }
};

#endif

