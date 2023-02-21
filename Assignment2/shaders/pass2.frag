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

  // discard fragment if it's a background fragement
  float storedDepth = texture(depthBuffer, texCoords).r;
  storedDepth = storedDepth * 2.0 - 1.0;
  float d = gl_FragCoord.z * 0.5 + 0.5;

  if(d <= storedDepth) {
    discard;
  }

  // normal
  vec3 N = texture(normalBuffer, texCoords).xyz; // in [0,1]

  // position in sample space
  vec3 P = texture(positionBuffer, texCoords).xyz; // texture [0,1]x[0,1] of position in VCS

  // building perpendicular x,y
  vec3 X = abs(N.x) > 0.5 ? vec3(0, 1, 0) : vec3(1, 0, 0);
  vec3 Y = cross(N, X);
  X = normalize(cross(Y, N));
  Y = normalize(Y);

  // build rotation matrix
  mat3 R = mat3(X, Y, N);

  // keep track of how many samples are above their respective surfaces
  int samplesAboveSurface = 0;

  // loop over sample offsets
  for(int i = 0; i < numSampleOffsetsToUse; i++) {

    // compute the vcs pos of the offset from the fragment's vcs pos
    vec3 offset = offsetScale * (R * sampleOffsets[i]);

    // move P into depth buffer coord system
    vec3 Pprime3 = P + offset;
    vec4 Pprime = VCS_to_CCS * vec4(Pprime3, 1.0f); // Pprime VCS -> CCS
    Pprime /= Pprime.w; // Pprime CSS -> NDCS

    // Pprim between [0, 1]
    Pprime *= 0.5;
    Pprime += 0.5;

    // depth of Pprime is its 'z' 
    float sampleDepth = Pprime.z;

    // get the value in the depth buffer at Pprime's x and y coords
    float surfaceDepth = texture(depthBuffer, Pprime.xy).r;

    // is this sample above its surface?
    if(sampleDepth < surfaceDepth) {
      samplesAboveSurface++;
    }
  }

  // divide to get occlusion factor for this fragment
  occlusionFactor = float(samplesAboveSurface) / float(numSampleOffsetsToUse);
}
