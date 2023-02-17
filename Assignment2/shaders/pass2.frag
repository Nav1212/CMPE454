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

  // float dummy = texCoords.x;  // REMOVE THIS

  // normal
  vec3 N = texture(normalBuffer, texCoords).xyz;

  // position in sample space
  vec3 Pss = texture(positionBuffer, texCoords).xyz;

  // for example
  // Compute orthonormal basis with Z = surface normal
  // vec3 z = normalize(normal);
  // vec3 x = normalize(cross(vec3(0.0, 1.0, 0.0), z));
  // vec3 y = cross(z, x);

  // building perpendicular x,y
  vec3 X = abs(N.x) > 0.5 ? vec3(0, 1, 0) : vec3(1, 0, 0);
  vec3 Y = normalize(cross(N, X));
  X = normalize(cross(Y, N));

  // build rotation matrix
  mat3 R = mat3(X, Y, N);

  // vec3 Pvcs = R * Pss;

  // accumulators
  float occ = 0.0;

  // loop over sample offsets
  for(int i = 0; i < numSampleOffsetsToUse; i++) {
    // move P into depth buffer coord system
    // vec4 Pdb = VCS_to_CCS * Pvcs;
    // // vec3 ndcs = 

    // vec3 offsetVcs = normalize(sampleOffsets[i]) * offsetScale;
    // vec4 offsetCcs = VCS_to_CCS * vec4(offsetVcs, 0.0);
    // vec3 offsetNdcs = 

    // vec3 offsetVcs = R * vec3(sampleOffsets[i] * offsetScale, 0.0);

    // calculate sample
    vec3 offsetVcs = R * (sampleOffsets[i] * offsetScale);
    vec3 samplePos = Pss + offsetVcs;
    // hold up
    vec4 offsetCcs = VCS_to_CCS * vec4(samplePos, 0.0);
    vec3 sampleNdcs = offsetCcs.xyz / offsetCcs.w;
    vec2 sampleDBCoords = vec2((sampleNdcs.x + 1.0) / 2.0, (sampleNdcs.y + 1.0) / 2.0);

    // which is right? neither?
    float sampleDepth = (sampleNdcs.z + 1.0) / 2.0;
    // float sampleDepth = texture(depthBuffer, sampleDBCoords).r;

    // calculate surface point
    float surfaceDepth = texture(depthBuffer, texCoords).r;

    if(surfaceDepth - sampleDepth < 0) {
      occ += 1.0;
    }

  }

  // occlusionFactor = 0.5;
  occlusionFactor = occ / float(numSampleOffsetsToUse);
}
