// Pass 1 vertex shader
//
// Output vertex depth in [0,1], position in VCS, normal in VCS

#version 300 es
precision mediump float;

uniform mat4 OCS_to_VCS;
uniform mat4 OCS_to_CCS;

// inputs

layout(location = 0) in vec3 vertPosition; // OCS vertex position from OpenGL
layout(location = 1) in vec3 vertNormal;   // OCS vertex normal

// outputs

out vec3 vcsPosition;
out vec3 vcsNormal;
out float depth;		// depth in [0,1]

void main() {
  // position in CCS

  vec4 ccsPosition = OCS_to_CCS * vec4(vertPosition, 1.0);

  gl_Position = ccsPosition;

  // output the VCS position, VCS normal, and depth in [0,1]
  //
  // YOUR CODE HERE

  vcsPosition = vec3(OCS_to_VCS * vec4(vertPosition, 1.0f));
  vcsNormal = (OCS_to_VCS * vec4(vertNormal, 0.0)).xyz;
  depth = 0.5 * ((ccsPosition.z / ccsPosition.w) + 1.0);
}
