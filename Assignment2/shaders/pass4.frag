// Pass 4 fragment shader
//
// Apply diffuse lighting to fragment.  Later do Phong lighting.
//
// Determine whether fragment is in shadow.  If so, reduce intensity to 50%.

#version 300 es
precision mediump float;


uniform sampler2D positionBuffer;   // texture [0,1]x[0,1] of position in VCS
uniform sampler2D normalBuffer;     // texture [0,1]x[0,1] of normal in VCS
uniform sampler2D depthBuffer;      // texture [0,1]x[0,1] of depth
uniform sampler2D blurredBuffer;    // texture [0,1]x[0,1] of blurred occlusion factors

uniform vec3 kd;   // material properties
uniform vec3 ks;
uniform vec3 Ia;
uniform float shininess;

uniform vec3 Iin;  // light colour
uniform vec3 L;    // direction to light in WCS

uniform int showOcclusion;

// inputs

in vec2 texCoords;

// outputs

out vec4 fragColour;   // fragment's final colour


void main()

{
  // YOUR CODE HERE

  float dummy = texCoords.x;  // REMOVE THIS

  fragColour = vec4( 0.8, 0.5, 0.1, 1.0 );
}
