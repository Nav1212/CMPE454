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

void main() {
  // Output the occlusion factor
  //
  // YOUR CODE HERE

  // float dum = texCoords.x;

  // occlusionFactor = 0.5;
  // return;

  // discard fragment if it's a background fragement
  float storedDepth = texture(depthBuffer, texCoords).r;
  storedDepth = storedDepth * 2.0 - 1.0;
  float d = gl_FragCoord.z * 0.5 + 0.5;

  if(d <= storedDepth) {
    discard;
  }

  // normal
  // vec3 N = normalize(texture(normalBuffer, texCoords).xyz * 2.0 - 1.0);
  vec3 N = texture(normalBuffer, texCoords).xyz;

  // position in sample space
  vec3 Pss = texture(positionBuffer, texCoords).xyz;

  // building perpendicular x,y
  vec3 X = abs(N.x) > 0.5 ? vec3(0, 1, 0) : vec3(1, 0, 0);
  vec3 Y = cross(N, X);
  X = normalize(cross(Y, N));
  Y = normalize(Y);

  // build rotation matrix
  mat3 R = mat3(X, Y, N);

  // accumulator
  float occlusion = 0.0;

  // loop over sample offsets
  for(int i = 0; i < numSampleOffsetsToUse; i++) {

    // compute the vcs pos of the offset from the fragment's vcs pos

    // calculate sample
    vec3 offset = offsetScale * (R * sampleOffsets[i]);
    // vec3 offset = offsetScale * (R * vec3(sampleOffsets[i].xy, 0.0));

    // move P into depth buffer coord system
    vec4 Pprime = VCS_to_CCS * vec4(Pss + offset, 1.0f);
    Pprime /= Pprime.w;

    // calculate surface depth
    // float surfaceDepth = texture(depthBuffer, texCoords).r;
    float surfaceDepth = texture(depthBuffer, texCoords + offset.xy).r;

    // put sample depth in the range [0, 1]
    float sampleDepth = (Pprime.z + 1) / 2;

    // sample above surface (+delta), or sample below surface (-delta)
    float delta = sampleDepth - surfaceDepth;

    // update occlusion accumulator
    occlusion += delta < 0.0 ? 1.0 : 0.0;

  }

  // occlusionFactor = 0.5;
  occlusionFactor = occlusion / float(numSampleOffsetsToUse);
}
