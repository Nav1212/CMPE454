// object.cpp
//
// Set up and move an object.


#include "headers.h"
#include "object.h"
#include "world.h"
#include "gpuProgram.h"
#include "main.h"


// Set up an object by creating a VAO and rewriting the object
// vertices so that it is centred at (0,0).

void Object::setupVAO( float objectVerts[], float objectWidth )

{
  // ---- Rewrite the object vertices ----

  // Find the bounding box of the object

  vec3 min = vec3( objectVerts[0], objectVerts[1], 0 );
  vec3 max = vec3( objectVerts[0], objectVerts[1], 0 );

  int i;
  for (i=0; objectVerts[i] != 9999; i+=2) {
    vec3 v( objectVerts[i], objectVerts[i+1], 0 );
    if (v.x < min.x) min.x = v.x;
    if (v.x > max.x) max.x = v.x;
    if (v.y < min.y) min.y = v.y;
    if (v.y > max.y) max.y = v.y;
  }

  // Rewrite the model vertices so that the object is centred at (0,0)
  // and has width objectWidth

  float s = objectWidth / (max.x - min.x);
 
  mat4 modelToOriginTransform
    = scale( s, s, 1 )
    * translate( -0.5*(min.x+max.x), -0.5*(min.y+max.y), 0 );

  for (int i=0; objectVerts[i] != 9999; i+=2) {
    vec4 newV = modelToOriginTransform * vec4( objectVerts[i], objectVerts[i+1], 0.0, 1.0 );
    objectVerts[i]   = newV.x / newV.w;
    objectVerts[i+1] = newV.y / newV.w;
  }

  // Store segments in the object model for later

  for (int i=0; objectVerts[i] != 9999; i+=4)
    segments.push_back( Segment( vec3( objectVerts[i],   objectVerts[i+1], 0 ),
				 vec3( objectVerts[i+2], objectVerts[i+3], 0 ) ) );

  // ---- Create a VAO for this object ----

  // YOUR CODE HERE
  
}


// Draw the object


void Object::draw( mat4 &worldToViewTransform, GPUProgram *gpuProg )

{
  // YOUR CODE HERE (set the transform)                                                                      

  mat4 modelToViewTransform;

  // Tell the shaders about the model-to-view transform.  (See MVP in asteroids.vert.)                       

  gpuProg->setMat4( "MVP", modelToViewTransform );

  // YOUR CODE HERE (call OpenGL to draw the VAO of this object)  

}


mat4 Object::modelToWorldTransform() const

{
  mat4 M;

  // YOUR CODE HERE

  return M;
}



// Update the pose (position and orientation)


void Object::updatePose( float deltaT )

{
  // Update position

  position = position + deltaT * velocity;

  // Update orientation

  float angularSpeed = angularVelocity.length();
  vec3 rotationAxis;

  if (angularSpeed > 0.0001)
    rotationAxis = angularVelocity.normalize();
  else
    rotationAxis = vec3(0,0,1);

  quaternion q = quaternion( deltaT * angularSpeed, rotationAxis );
  orientation =  q * orientation;

  // wrap around screen

  if (position.x > world->worldMax.x)
    position.x -= (world->worldMax.x - world->worldMin.x);
  if (position.x < world->worldMin.x)
    position.x += (world->worldMax.x - world->worldMin.x);
  if (position.y > world->worldMax.y)
    position.y -= (world->worldMax.y- world->worldMin.y);
  if (position.y < world->worldMin.y)
    position.y += (world->worldMax.y - world->worldMin.y);
}



bool Object::intersects( Object const& obj ) const

{
  mat4 M = modelToWorldTransform();

  for (unsigned int i=0; i<segments.size(); i++) {

    vec3 t = (M * vec4( segments[i].tail )).toVec3();
    vec3 h = (M * vec4( segments[i].head )).toVec3();
    Segment seg(t,h);

    if (obj.intersects( seg ))
	return true;
  }

  return false;
}


bool Object::intersects( Segment const &seg ) const

{
  mat4 M = modelToWorldTransform();

  for (unsigned int i=0; i<segments.size(); i++) {

    vec3 t = (M * vec4( segments[i].tail )).toVec3();
    vec3 h = (M * vec4( segments[i].head )).toVec3();
    Segment s(t,h);

    if (s.intersects( seg ))
	return true;
  }

  return false;
}

