// Pass 2 fragment shader
//
// Compute the ambient occlusion factor

#version 300 es
precision mediump float;


#define NUM_SAMPLE_OFFSETS 1000  // must match the same renderer.h


// gBuffers

uniform sampler2D positionBuffer;   // texture [0,1]x[0,1] of position in VCS
uniform sampler2D normalBuffer;     // texture [0,1]x[0,1] of normal in VCS
uniform sampler2D depthBuffer;      // texture [0,1]x[0,1] of depth

// Sample offsets

uniform vec3 sampleOffsets[NUM_SAMPLE_OFFSETS]; // random (x,y,z) offsets, all with z > 0 and all of length <= 1
uniform int numSampleOffsetsToUse;              // number of offsets to use when sampling
uniform float offsetScale;                      // scale each offset by this

uniform mat4 VCS_to_CCS;  // projection matrix

// inputs

in vec2 texCoords;

// outputs

out float occlusionFactor;


void main()

{
  // Output the occlusion factor
  //
  // YOUR CODE HERE

  float dummy = texCoords.x;  // REMOVE THIS
  
  occlusionFactor = 0.5;
}
