// CISC/CMPE 454 Assignment 3
//
// Physical simulation of spheres


#include "headers.h"
#include "linalg.h"
#include "seq.h"
#include "axes.h"
#include "strokefont.h"
#include "main.h"
#include "world.h"


GLuint windowWidth = 1200;
GLuint windowHeight = 900;

GLFWwindow* window;

World *world;
Axes *axes;
StrokeFont *strokeFont;
Segs *segs;

bool sleeping = false;
bool showAxes = false;
bool showClosest = false;

float timeOffset = 0;
float timeFactor = 0.5;	// scale real time by this to get simulation time 

// Viewpoint movement using the mouse

typedef enum { TRANSLATE, ROTATE } ModeType;
typedef enum { LEFT_BUTTON, MIDDLE_BUTTON, RIGHT_BUTTON, NO_BUTTON } ButtonType;

GLFWcursor* rotationCursor;
GLFWcursor* translationCursor;

ModeType mode = ROTATE;		// translate or rotate mode
ButtonType button;		// which button is currently down
vec2 mouse;			// mouse position on button DOWN


// Drawing function

float fovy;
vec3 eyePosition;
vec3 upDir(0,0,1);
vec3 lookAt(0,0,0);

vec3 initEyeDir = vec3(0.7,0.3,0.2).normalize();

const float initEyeDistance = 4.0;

vec3 lightDir = vec3(1,1.5,2).normalize();


void display()

{
  // WCS-to-VCS

  mat4 WCS_to_VCS = lookat( eyePosition, lookAt, upDir );

  // VCS-to-CCS

  float n = (eyePosition - lookAt).length() - WORLD_RADIUS;
  float f = (eyePosition - lookAt).length() + WORLD_RADIUS;

  mat4 VCS_to_CCS = perspective( fovy, windowWidth / (float) windowHeight, n, f );

  // Draw the objects

  world->draw( WCS_to_VCS, VCS_to_CCS, lightDir );

  // Draw the world axes

  if (showAxes) {
    mat4 axesTransform = VCS_to_CCS * WCS_to_VCS * scale( 1, 1, 1 );
    axes->draw( axesTransform, lightDir );
  }

  // Output status message

  char buffer[1000];
  sprintf( buffer, "x %4.2f", timeFactor );
  strokeFont->drawStrokeString( buffer, 0.95, -0.95, 0.04, 0, RIGHT );
}


// Handle windox size changes

void windowSizeCallback( GLFWwindow* window, int width, int height )

{
  windowWidth = width;
  windowHeight = height;
  glViewport(0, 0, width, height);
}



// GLFW Error callback

void GLFWErrorCallback( int error, const char* description )

{
  cerr << "Error " << error << ": " << description << endl;
  exit(1);
}


float getTime()

{
  static time_t initialSeconds = 0;  // subtract this from times to avoid loss of float precision
  
#ifdef _WIN32

  struct timeb thisTime;
  ftime( &thisTime );

  if (initialSeconds == 0)
    initialSeconds = thisTime.time;

  return (thisTime.time-initialSeconds) + thisTime.millitm / 1000.0;

#else

  struct timeval thisTime;
  gettimeofday( &thisTime, NULL );

  if (initialSeconds == 0)
    initialSeconds = thisTime.tv_sec;

  return (thisTime.tv_sec-initialSeconds) + thisTime.tv_usec / 1000000.0;

#endif
}



void toggleSleep()

{
  static float startTime;

  sleeping = !sleeping;

  float thisTime = getTime();

  if (sleeping)
    startTime = thisTime;
  else
    timeOffset += thisTime - startTime;
}


// Handle a key press


void keyCallback( GLFWwindow* window, int key, int scancode, int action, int mods )
  
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (key) {

    case GLFW_KEY_ESCAPE:
      exit(0);

    case 'A':
      showAxes = !showAxes;
      break;

    case 'C':
      showClosest = !showClosest;
      break;

    case 'P':
      toggleSleep();
      break;
      
    case '+':
    case '=':
      timeFactor *= sqrt(2);
      break;
      
    case '-':
    case '_':
      timeFactor /= sqrt(2);
      break;
      
    case ' ':
      if (mode == ROTATE)
	mode = TRANSLATE;
      else
	mode = ROTATE;
      glfwSetCursor( window, (mode == TRANSLATE ? translationCursor : rotationCursor) );
      break;

    case '?':
    case '/':
      cout << "a - toggle axes" << endl;
    }
  }
}




// Find 2d mouse position on 3D arcball

vec3 arcballPos( vec2 pos )

{
  vec3 p(  pos.x/(float)windowWidth * 2 - 1, -(pos.y/(float)windowHeight * 2 - 1), 0 );
  float rr = p * p;
  if (rr <= 1)			// inside arcball
    p.z = sqrt( 1 - rr );
  else
    p = p.normalize();
  return p;
}


mat4 arcballTransform( vec2 pos, vec2 prevPos )

{
  vec3 p1 = arcballPos( pos );
  vec3 p0 = arcballPos( prevPos );

  float dot = p0 * p1;
  if (dot > 1) dot=1;
  float angle = acos( dot );

  vec3 axis = p0 ^ p1;

  return rotate( -1 * angle, axis );
}


// Mouse motion callback
//
// Only enabled when mouse button is down (which is done in mouseButtonCallback())

void mousePositionCallback( GLFWwindow* window, double x, double y )

{
  vec3 xdir, ydir, zdir;

  ydir = upDir.normalize();
  zdir = (eyePosition - lookAt).normalize();
  xdir = (ydir ^ zdir).normalize();

  if (mode == TRANSLATE) {

    if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS) { // pan in x and y

      lookAt = lookAt + fovy * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);
      eyePosition = eyePosition + fovy * ((mouse.x-x) * xdir + (y-mouse.y) * ydir);

    } else if (button == RIGHT_BUTTON) { // move in z

      lookAt = lookAt + (mouse.y-y)*0.2 * zdir;
      eyePosition = eyePosition + (mouse.y-y)*0.2 * zdir;
    }

  } else { // mode == ROTATE

    if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_RIGHT ) == GLFW_PRESS) { // zoom

      fovy *= 1.0 + 0.001*(mouse.y-y);
      if (fovy > 135)
	fovy = 135;
      if (fovy < 0.001)
	fovy = 0.001;

    } else if (glfwGetMouseButton( window, GLFW_MOUSE_BUTTON_LEFT ) == GLFW_PRESS) { // rotate

      if (vec2(x,y) != mouse) { // avoid NaN results                                                             

	mat4 WCS_to_VCS = lookat( eyePosition, lookAt, upDir );
	mat4 VCS_to_WCS = WCS_to_VCS.inverse();
	mat4 T = VCS_to_WCS * arcballTransform( vec2(x,y), mouse ) * WCS_to_VCS;

	upDir = (T * vec4( upDir, 0 )).toVec3();
	vec3 eyeDir = eyePosition - lookAt;
	eyePosition = (T * vec4( eyeDir, 0)).toVec3() + lookAt;
      }
    }
  }

  mouse.x = x;
  mouse.y = y;
}


// Mouse button callback

void mouseButtonCallback( GLFWwindow* window, int button, int action, int mods )

{
  if (action == GLFW_PRESS) {

    // get and store initial mouse position
      
    double x, y;
    glfwGetCursorPos(window, &x, &y );
    mouse.x = x;
    mouse.y = y;

    // enable mouse movement events
      
    glfwSetCursorPosCallback( window, mousePositionCallback );
      
  } else // (action == GLFW_RELEASE)

    // disable mouse movement events
      
    glfwSetCursorPosCallback( window, NULL );
}



// Main program


int main( int argc, char **argv )

{
  // Set up GLFW

  glfwSetErrorCallback( GLFWErrorCallback );
  
  if (!glfwInit())
    return 1;

#ifdef MACOS
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 2 );
  glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE );
  glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );
#else
  glfwWindowHint( GLFW_CLIENT_API, GLFW_OPENGL_ES_API );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 0 );
#endif

  // Set up the window

  window = glfwCreateWindow( windowWidth, windowHeight, "Physical simulation", NULL, NULL);
  
  if (!window) {
    glfwTerminate();
    cerr << "GLFW failed to create a window" << endl;

#ifdef MACOS
    const char *descrip;
    int code = glfwGetError( &descrip );
    cerr << "GLFW code:  " << code << endl;
    cerr << "GLFW error: " << descrip << endl;
#endif
    return 1;
  }

  glfwMakeContextCurrent( window );
  glfwSwapInterval( 1 );
  gladLoadGLLoader( (GLADloadproc) glfwGetProcAddress );

  glfwSetKeyCallback( window, keyCallback );
  glfwSetMouseButtonCallback( window, mouseButtonCallback );
  glfwSetWindowSizeCallback( window, windowSizeCallback );

  rotationCursor = glfwCreateStandardCursor( GLFW_ARROW_CURSOR );
  translationCursor = glfwCreateStandardCursor( GLFW_CROSSHAIR_CURSOR );

  glfwSetWindowPos( window, 7, 30 );

  // Set up objects

  world      = new World( argc > 1 ? argv[1] : NULL );
  axes       = new Axes();
  strokeFont = new StrokeFont();
  segs       = new Segs();
  
  // Point camera to the model

  eyePosition = (initEyeDistance * WORLD_RADIUS) * initEyeDir + lookAt;
  fovy = 0.8 * atan2( 1, initEyeDistance );

  vec3 t = upDir ^ initEyeDir;
  upDir = (initEyeDir ^ t).normalize();

  // Main loop

  float prevTime, thisTime; // record the last rendering time
  prevTime = getTime();

  glEnable( GL_DEPTH_TEST );

  while (!glfwWindowShouldClose( window )) {

    // Find elapsed time since last render

    thisTime = getTime();
    float elapsedSeconds = thisTime - prevTime;
    prevTime = thisTime;

    // Update the world state

    if (!sleeping)
      world->updateState( elapsedSeconds );

    // Clear, display, and check for events

    glClearColor( 1, 1, 1, 1 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // also clear depth buffer

    display();

    glfwSwapBuffers( window );
    glfwPollEvents();
  }

  // Clean up

  glfwDestroyWindow( window );
  glfwTerminate();

  return 0;
}
