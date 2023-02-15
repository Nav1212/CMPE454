// Pass 3 fragment shader
//
// Blur the occlusion values

#version 300 es
precision mediump float;


uniform sampler2D occlusionBuffer;
uniform float blurRadius;
uniform vec2 texelSize;

// inputs

in vec2 texCoords;

// outputs

out float blurredOcclusionFactor;


void main()

{
  // Average the occlusion factors in the neighbourhood
  //
  // YOUR CODE HERE

  float dummy = texCoords.y; // REMOVE THIS
  
  blurredOcclusionFactor = 0.75;
}
