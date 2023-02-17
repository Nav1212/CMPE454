// G-buffer renderer


#include "headers.h"
#include "renderer.h"
#include "shader.h"


void drawFullscreenQuad(); // below



// Render the scene in three passes.


void Renderer::render( seq<wfModel *> &objs, mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, float worldRadius, GLFWwindow *window )

{
  int activeDrawBuffers[NUM_GBUFFERS];

  int bufferIDs[NUM_GBUFFERS];
  char * bufferNames[NUM_GBUFFERS];

  gbuffer->BindForWriting();


  // Pass 1
  //
  // This fragment shader outputs position (VCS), normal (VCS), and depth [0,1] into drawBuffers 0, 1, and 2


  pass1Prog->activate();

  // Set up fragment shader outputs to three gbuffers: position, normal, depth
  
  gbuffer->BindTexture( POSITION_GBUFFER, 0 ); // position texture goes on gb texture unit 0
  gbuffer->BindTexture( NORMAL_GBUFFER,   1 ); // normal texture   goes on gb texture unit 1
  gbuffer->BindTexture( DEPTH_GBUFFER,    2 ); // depth texture    goes on gb texture unit 2

  activeDrawBuffers[0] = 0; // fragment shader has outputs to the three drawBuffers listed above
  activeDrawBuffers[1] = 1;
  activeDrawBuffers[2] = 2;
  
  gbuffer->setDrawBuffers( 3, activeDrawBuffers );

  // Clear buffers

  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); // OpenGL's depth buffer, not the fragment shader's 'depthBuffer' texture!
  glEnable( GL_DEPTH_TEST );

  // CHANGED: Set everything in the fragment shader's depth buffer to
  // 1.0.  This buffer is different than OpenGL's depth buffer, so
  // does not get cleared otherwise.  Not clearing the fragment
  // shader's depth buffer might lead to depth values in the
  // background pixels that are different than 1.0.  (But this is
  // likely implementation-dependant.)
  
  float maxDepth = 1.0;
  glClearBufferfv( GL_COLOR, 2, &maxDepth ); // 2 = 'depthBuffer'

  // Draw objects

  for (int i=0; i<objs.size(); i++) {

    mat4 OCS_to_VCS = WCS_to_VCS * objs[i]->objToWorldTransform;
    mat4 OCS_to_CCS = VCS_to_CCS * OCS_to_VCS;

    pass1Prog->setMat4( "OCS_to_VCS", OCS_to_VCS );
    pass1Prog->setMat4( "OCS_to_CCS", OCS_to_CCS );

    objs[i]->draw( pass1Prog );
  }

  pass1Prog->deactivate();

  if (debug == 0) {
    bufferIDs[0] = POSITION_GBUFFER;
    bufferIDs[1] = NORMAL_GBUFFER;
    bufferIDs[2] = DEPTH_GBUFFER;
    bufferNames[0] = gbufferNames[POSITION_GBUFFER];
    bufferNames[1] = gbufferNames[NORMAL_GBUFFER];
    bufferNames[2] = gbufferNames[DEPTH_GBUFFER];
    gbuffer->DrawGBuffers( window, 3, bufferIDs, gbufferNames );
    return;
  }


  // Pass 2
  //
  // Fragment shader computes raw ambient occlusion and stores it in
  // the OCCLUSION_GBUFFER.


  pass2Prog->activate();


  // Create a set of sample offset in a hemisphere, with denser
  // sampling near the centre.  Note that the denser sampling near the
  // centre is the result of uniform sampling in radius, so nothing
  // special needs to be done to achieve this.

  static vec3 sampleOffsets[ NUM_SAMPLE_OFFSETS ];

  static bool firstTime = true;

  if (firstTime) {

    // YOUR CODE HERE
    //
    // Generate NUM_SAMPLE_OFFSETS sample offsets and store them in
    // the sampleOffsets array.  Each should be a random vector in the
    // +z hemisphere, with a uniform distribution of directions and a
    // distribution of lengths in [0.1,1] such that there are more offsets
    // closer to the origin than farther from the origin.
    //
    // CHANGED: Comment above reflects correct distribution of lengths

    for (int i = 0; i < NUM_SAMPLE_OFFSETS; i++) {
      // idea summary:
      // generate coords in the range 
      //       x in [-1, +1]
      //       y in [-1, +1]
      //       z in [0,  +1]
      // if (x^2 + y^2 + z^2) > 1: reject!

      // random direction in +z hemisphere
      // vec3 dir = vec3(
      //   2.0 * rand(vec2(float(i), 0.0)) - 1.0,
      //   2.0 * rand(vec2(float(i), 1.0)) - 1.0,
      //   abs(2.0 * rand(vec2(float(i), 2.0)) - 1.0)
      // )

      // random direction in +z hemisphere
      vec3 dir = vec3(
        2.0f * randIn01() - 1.0f,
        2.0f * randIn01() - 1.0f,
        abs(2.0f * randIn01() - 1.0f)
      );
      dir = dir.normalize();

      // random length in range [0.1, 1]
      float len;
      vec2 sample;

      do {
        sample = vec2(randIn01(), randIn01());
        len = 0.1f + (1.0f - 0.1f) * sample.x;
      } while (sample.y > len);

      // generate offset
      vec3 offset = vec3(dir.x * len, dir.y * len, dir.z * len);

      // TODO: is this generating the correct ranges of values? 
      //     ie.
      //       x in [-1, +1]
      //       y in [-1, +1]
      //       z in [0,  +1]
      
      // cout << offset.x << ", " << offset.y << ", " << offset.z << endl;

      // append to array
      sampleOffsets[i] = offset;
    }

    firstTime = false;
  }

  pass2Prog->setVec3( "sampleOffsets", sampleOffsets, NUM_SAMPLE_OFFSETS );

  // Set up fragment shader output to one gbuffer: occlusion

  gbuffer->BindTexture( OCCLUSION_GBUFFER, 3 );

  activeDrawBuffers[0] = 3; // fragment shader has outputs to the 'occlusion' drawBuffers listed above

  gbuffer->setDrawBuffers( 1, activeDrawBuffers );

  // Tell fragment shader of input textures

  pass2Prog->setInt( "positionBuffer", BASE_GBUFFER_TEXTURE_UNIT + 0 ); // same units as in Pass 1
  pass2Prog->setInt( "normalBuffer",   BASE_GBUFFER_TEXTURE_UNIT + 1 );
  pass2Prog->setInt( "depthBuffer",    BASE_GBUFFER_TEXTURE_UNIT + 2 );

  // Other uniforms

  pass2Prog->setInt( "numSampleOffsetsToUse", numSampleOffsetsToUse );

  pass2Prog->setFloat( "offsetScale", offsetScale ); // sample within 1% of world radius

  pass2Prog->setMat4( "VCS_to_CCS", VCS_to_CCS );

  // Draw quad
  
  glClear( GL_COLOR_BUFFER_BIT );
  glDisable( GL_DEPTH_TEST ); // not needed as we're drawing the fullscreen quad

  drawFullscreenQuad();

  pass2Prog->deactivate();

  if (debug == 1) {
    bufferIDs[0] = DEPTH_GBUFFER;
    bufferIDs[1] = -1;
    bufferIDs[2] = OCCLUSION_GBUFFER;
    bufferNames[0] = gbufferNames[DEPTH_GBUFFER];
    bufferNames[2] = gbufferNames[OCCLUSION_GBUFFER];
    gbuffer->DrawGBuffers( window, 3, bufferIDs, bufferNames );
    return;
  }


  // Pass 3
  // 
  // Fragment shader blurs the raw ambient occlusions from
  // OCCLUSION_GBUFFER and puts results in BLURRED_GBUFFER.


  pass3Prog->activate();

  // Set up fragment shader output to one gbuffer: blurred occlusion

  gbuffer->BindTexture( BLURRED_GBUFFER, 4 );

  activeDrawBuffers[0] = 4; // fragment shader has outputs to the 'blurred' drawBuffers listed above

  gbuffer->setDrawBuffers( 1, activeDrawBuffers );

  // Set fragment shader uniforms

  pass3Prog->setInt(   "occlusionBuffer", BASE_GBUFFER_TEXTURE_UNIT + 3 ); // sampler2D of occlusion factors
  pass3Prog->setInt(   "depthBuffer",     BASE_GBUFFER_TEXTURE_UNIT + 2 ); // CHANGED: ADDED TO ALLOW ACCESS TO DEPTH BUFFER IN pass3.frag
  pass3Prog->setFloat( "blurRadius",      blurRadius );                    // radius of blurring kernel
  pass3Prog->setVec2(  "texelSize",       vec2( 1.0/(float)gbuffer->fbWidth, 1.0/(float)gbuffer->fbHeight ) ); // distance between [0,1]x[0,1] texCoords of adjacent texels

  // Draw quad
  
  glClear( GL_COLOR_BUFFER_BIT );

  drawFullscreenQuad();

  pass3Prog->deactivate();

  if (debug == 2) {
    bufferIDs[0] = DEPTH_GBUFFER;
    bufferIDs[1] = -1;
    bufferIDs[2] = OCCLUSION_GBUFFER;
    bufferIDs[3] = BLURRED_GBUFFER;
    bufferNames[0] = gbufferNames[DEPTH_GBUFFER];
    bufferNames[2] = gbufferNames[OCCLUSION_GBUFFER];
    bufferNames[3] = gbufferNames[BLURRED_GBUFFER];
    gbuffer->DrawGBuffers( window, 4, bufferIDs, bufferNames );
    return;
  }


  // Pass 4
  //
  // Fragment shader uses blurred occlusion factors along with position
  // and normal to compute Phong shading with ambient occlusion.
  
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ); // fragment shader renders to main framebuffer now (= 0)

  pass4Prog->activate();

  pass4Prog->setVec3(  "L",         vec3( 1, 1, 3 ).normalize() );  // direction to light in VCS
  pass4Prog->setVec3(  "Iin",       vec3( 1, 1, 1 ) );              // light colour

  pass4Prog->setVec3(  "kd",        vec3( 0.3, 0.8, 0.7 ) );  // surface diffuse coefficient
  pass4Prog->setVec3(  "ks",        vec3( 0.5, 0.5, 0.5 ) );  // surface specular coefficient
  pass4Prog->setVec3(  "Ia",        vec3( 0.1, 0.1, 0.1 ) );  // surface ambient lighting
  pass4Prog->setFloat( "shininess", 200 );                    // surface shiniess

  pass4Prog->setInt( "positionBuffer", BASE_GBUFFER_TEXTURE_UNIT + 0 ); // same units as in Pass 1
  pass4Prog->setInt( "normalBuffer",   BASE_GBUFFER_TEXTURE_UNIT + 1 );
  pass4Prog->setInt( "depthBuffer",    BASE_GBUFFER_TEXTURE_UNIT + 2 );
  pass4Prog->setInt( "blurredBuffer",  BASE_GBUFFER_TEXTURE_UNIT + 4 ); // blurred occlusion factor

  pass4Prog->setInt( "showOcclusion",  (showOcclusion ? 1 : 0) );


  drawFullscreenQuad();

  pass4Prog->deactivate();
}



// Draw a quad over the full screen.  This generates a fragment for
// each pixel on the screen, allowing the fragment shader to run on
// each fragment.


void drawFullscreenQuad()

{
  vec2 verts[4] = { vec2( -1, -1 ), vec2( -1, 1 ), vec2( 1, -1 ), vec2( 1, 1 ) };
    
  GLuint VAO, VBO;

  glGenVertexArrays( 1, &VAO );
  glBindVertexArray( VAO );

  glGenBuffers( 1, &VBO );
  glBindBuffer( GL_ARRAY_BUFFER, VBO );

  glBufferData( GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW );

  glEnableVertexAttribArray( 0 );
  glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, 0 );

  glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

  glDeleteBuffers( 1, &VBO );
  glDeleteVertexArrays( 1, &VAO );
}


