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

  // discard fragment if it's a background fragement
  float storedDepth = texture(depthBuffer, texCoords).r;
  storedDepth = storedDepth * 2.0 - 1.0;
  float d = gl_FragCoord.z * 0.5 + 0.5;

  if(d <= storedDepth) {
    discard;
  }

  int blurRadiusInt = int(floor(blurRadius));
  float occlustionSum = 0.0;

  // here, we need to loop through the texels that surround our fragment
  // 
  // i.e. blurRadiusInt = 1
  // 
  //  [ ] [ ] [ ]  
  //  [ ] [x] [ ]  3
  //  [ ] [ ] [ ]
  //       3
  // we loop through each texel ([ ])
  // starting with top left, which is at (-blurRadiusInt, -blurRadiusInt) * texelSize
  // and so...
  //   

  // row (x)
  for(int row = -blurRadiusInt; row <= blurRadiusInt; row++) {
    // col (y)
    for(int col = -blurRadiusInt; col <= blurRadiusInt; col++) {
      vec2 offset = vec2(float(row), float(col)) * texelSize;

      // add occlusion factor at this texel
      occlustionSum += texture(occlusionBuffer, texCoords + offset).r;
    }
  }

  // the number of texels we grab occ. values from is defined as:
  int blurDimension = int(2.0 * blurRadius + 1.0);
  float numTexelsTotal = float(blurDimension * blurDimension);

  // return average occlusion factor
  blurredOcclusionFactor = occlustionSum / numTexelsTotal;
}
