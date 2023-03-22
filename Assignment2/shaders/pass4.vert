// Pass 4 vertex shader
//
// This is called by drawing the fullscreen quad in [-1,1]x[-1,1]
//
// Pass the [0x1]x[0,1] fragment's texture position to the fragment shader.

#version 300 es
precision mediump float;

// inputs

layout(location = 0) in vec3 screenPos;   // position on screen in [-1,1] x [-1,1]

// outputs

out vec2 texCoords;  // coordinates in texture in [0,1] x [0,1]

void main() {
  gl_Position = vec4(screenPos.x, screenPos.y, 0.0, 1.0);

  // YOUR CODE HERE
  //
  // Compute texture coords

  texCoords = screenPos.xy * 0.5 + 0.5;
}
