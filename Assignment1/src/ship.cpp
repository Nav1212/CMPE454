// ship.cpp


#include "ship.h"


void Ship::rotateCW( float deltaT )

{
  orientation = quaternion( - SHIP_ROTATION_SPEED * deltaT, vec3(0,0,1) ) * orientation;
}


void Ship::rotateCCW( float deltaT )

{
  orientation = quaternion( + SHIP_ROTATION_SPEED * deltaT, vec3(0,0,1) ) * orientation;
}


void Ship::addThrust( float deltaT )

{
  // Thrust is in the ship's +y direction.  Make sure to change the
  // velocity in that direction in the *world* coordinate system,
  // since the object velocity is in the world coordinate system.

  // YOUR CODE HERE
	velocity.x = velocity.x - SHIP_THRUST_ACCEL * sin(orientation.angle()) * deltaT;
	velocity.y = velocity.y + SHIP_THRUST_ACCEL * cos(orientation.angle()) * deltaT;


}


Shell * Ship::fireShell()

{
  // YOUR CODE HERE (below, find the correct position, velocity, and orientation for the shell)

  return new Shell( position, vec3(-SHELL_SPEED*sin(orientation.angle()), SHELL_SPEED * cos(orientation.angle()),1.0f), orientation );
}


// Ship model consisting of line segments
//
// These are in a ARBITRARY coordinate system and get remapped to the
// world coordinate system (with the centre of the ship at (0,0) and
// width SHIP_WIDTH) when the VAO is set up.


float Ship::verts[] = {

   3,0,  0,9,
   0,9, -3,0, 
  -3,0,  0,1,
   0,1,  3,0,

  9999
};

