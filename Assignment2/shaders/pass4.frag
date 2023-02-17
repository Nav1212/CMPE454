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
uniform vec3 L;    // direction to light in VCS

uniform int showOcclusion;

// inputs

in vec2 texCoords;

// outputs

out vec4 fragColour;   // fragment's final colour

void main() {
  // YOUR CODE HERE

  // discard fragment if it's a background fragement
  float depth = texture(depthBuffer, texCoords).z;

  if(gl_FragCoord.z <= depth) {
    discard;
  }

  // computing Phong illumination

  vec3 N = normalize(texture(normalBuffer, texCoords).xyz * 2.0 - 1.0);

  // diffuse
  float NdotL = max(dot(N, L), 0.0);
  vec3 diffuse = kd * NdotL * Iin;

  // specular
  vec3 R = 2 * NdotL * N - L;
  // vec3 Vpos = texture(positionBuffer, texCoords).xyz * 2.0 - 1.0
  vec3 P = texture(positionBuffer, texCoords).xyz * 2.0 - 1.0;
  vec3 V = normalize(-P);
  float RdotV = max(dot(R, V), 0.0);
  float n = shininess;
  vec3 specular = ks * pow(RdotV, n) * Iin;

  // ambient 
  float ka = 1.0; // TODO: what is this value?
  vec3 ambient = ka * Ia;

  // sum the lighting components
  vec3 phongColour = ambient + diffuse + specular;

  // occlusion factor
  float occlusionFactor = showOcclusion == 1 ? texture(blurredBuffer, texCoords).r : 1.0;

  // output
  fragColour = vec4(phongColour * occlusionFactor, 1.0);

}
