// Pass 3 fragment shader
//
// Blur the occlusion values

#version 300 es
precision mediump float;

uniform sampler2D depthBuffer;      // texture [0,1]x[0,1] of depth
uniform sampler2D occlusionBuffer;
uniform float blurRadius;
uniform vec2 texelSize;

// inputs

in vec2 texCoords;

// outputs

out float blurredOcclusionFactor;

void main() {
  // Average the occlusion factors in the neighbourhood
  //
  // YOUR CODE HERE

  // float dum = texCoords.x;

  // blurredOcclusionFactor = 0.75;
  // return;

  // discard fragment if it's a background fragement
  float storedDepth = texture(depthBuffer, texCoords).r;
  storedDepth = storedDepth * 2.0 - 1.0;
  float d = gl_FragCoord.z * 0.5 + 0.5;

  if(d <= storedDepth) {
    discard;
  }

  // square neighbourhood dimensions
  int squareSize = int(2.0 * blurRadius + 1.0);

  float occlustionSum = 0.0;
  float numFactors = 0.0;

  // x
  for(int x = -squareSize; x <= squareSize; ++x) {
    vec2 offset = vec2(float(x), 0.0) * texelSize;
    occlustionSum += texture(occlusionBuffer, texCoords + offset).r;
    numFactors++;
  }

  // y
  for(int y = -squareSize; y <= squareSize; ++y) {
    vec2 offset = vec2(0.0, float(y)) * texelSize;
    occlustionSum += texture(occlusionBuffer, texCoords + offset).r;
    numFactors++;
  }

  // Compute the average occlusion factor for the window
  // float avgOcclusion = weight * occlustionSum;
  // float avgOcclusion = occlustionSum / numFactors;

  // return average occlusion factor
  // blurredOcclusionFactor = occlustionSum / numFactors;
  blurredOcclusionFactor = occlustionSum / float(squareSize * squareSize);
  // blurredOcclusionFactor = weight * occlustionSum;

  // blurredOcclusionFactor = 0.75;
}
