// Pass 1 fragment shader
//
// Store fragment depth in depth gbuffer

#version 300 es
precision mediump float;


// inputs

in vec3  vcsPosition;
in vec3  vcsNormal;
in float depth;			// depth in [0,1]


// outputs

layout (location=0) out vec3  positionTexture; // position output to texture 0 (must match 0 in calling OpenGL code)
layout (location=1) out vec3  normalTexture;   // normal   output to texture 1 (must match 1 in calling OpenGL code)
layout (location=2) out float depthTexture;    // depth    output to texture 2 (must match 2 in calling OpenGL code)


void main()

{
  // Copy this fragment's data into the three gbuffers

  positionTexture = vcsPosition;
  normalTexture   = vcsNormal;
  depthTexture    = depth;
}
