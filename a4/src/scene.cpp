// scene.cpp

#include "headers.h"
#ifndef WIN32
#include <unistd.h>
#endif

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "arcball.h"
#include "arrow.h"
#include "light.h"
#include "main.h"
#include "material.h"
#include "rtWindow.h"
#include "scene.h"
#include "sphere.h"
#include "strokefont.h"
#include "triangle.h"
#include "wavefrontobj.h"

#ifndef MAXFLOAT
#define MAXFLOAT 9999999
#endif

#define UPDATE_INTERVAL 0.05  // update the screen with each 5% of RT progress

#define INDENT(n)                                  \
    {                                              \
        for (int i = 0; i < (n); i++) cout << " "; \
    }

vec3 backgroundColour(0, 0, 0);
vec3 blackColour(0, 0, 0);

#define MAX_NUM_LIGHTS 4

// Display everything

void Scene::display()

{
    mat4 WCS_to_VCS = win->arcball->V;

    mat4 VCS_to_CCS = perspective(win->fovy, windowWidth / (float)windowHeight,
                                  MAX(0.1, win->arcball->distToCentre - 100), win->arcball->distToCentre + 100);

    draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);
}

// Find the first object intersected

bool Scene::findFirstObjectInt(vec3 rayStart, vec3 rayDir, int thisObjIndex, int thisObjPartIndex, vec3 &P, vec3 &N,
                               vec3 &T, float &param, int &objIndex, int &objPartIndex, Material *&mat, int lightIndex)

{
    if (storingRays) storedRays.add(rayStart);

    bool hit = false;

    float maxParam = MAXFLOAT;

    for (int i = 0; i < objects.size(); i++) {
        WavefrontObj *wfo = dynamic_cast<WavefrontObj *>(objects[i]);

        // don't check for int with the originating object for non-wavefront objects (since such objects are convex)

        if (wfo || i != thisObjIndex) {
            vec3 point, normal, texcoords;
            float t;
            Material *intMat;
            int intPartIndex;

            if (objects[i]->rayInt(rayStart, rayDir, ((i != thisObjIndex) ? -1 : thisObjPartIndex), maxParam, point,
                                   normal, texcoords, t, intMat, intPartIndex)) {
                P = point;
                N = normal;
                T = texcoords;
                param = t;
                objIndex = i;
                objPartIndex = intPartIndex;
                mat = intMat;

                maxParam = t;  // In future, don't intersect any farther than this
                hit = true;
            }
        }
    }

    if (storingRays) {
        if (hit) {
            storedRays.add(P);
            if (lightIndex >= 0) {
                storedRayColours.add(
                    vec3(.843, .710, .278));  // GOLD: shadow ray toward a light that is (perhaps) blocked
            } else
                storedRayColours.add(vec3(.1, .7, .7));  // CYAN: normal ray that hits something
        } else {
            if (lightIndex >= 0) {
                storedRays.add(lights[lightIndex]->position);
                storedRayColours.add(vec3(.843, .710, .278));  // GOLD: shadow ray toward a light that is NOT blocked
            } else {
                storedRays.add(rayStart + sceneScale * 2 * rayDir);
                storedRayColours.add(vec3(.3, .3, .3));  // GREY: normal ray that misses
            }
        }
    }

    return hit;
}

// Raytrace: This is the main raytracing routine which finds the first
// object intersected, performs the lighting calculation, and does
// recursive calls.
//
// This returns the colour received on the ray.

vec3 Scene::raytrace(vec3 &rayStart, vec3 &rayDir, int depth, int thisObjIndex, int thisObjPartIndex)

{
    // Terminate the ray?

    // Terminate based on depth.  This leads to biased sampling.
    // Russian Roulette would be better.

    depth++;

    if (depth > maxDepth) return blackColour;

    // Find the closest object intersected

    vec3 P, N, texcoords;
    float t;
    int objIndex, objPartIndex;
    Material *mat;

    // Below, 'rayStart' is the ray staring point
    //        'rayDir' is the direction of the ray
    //        'thisObjIndex' is the index of the originating object
    //        'thisObjPartIndex' is the index of the part on the originating object (e.g. the triangle)
    //
    // If a hit is made then at the intersection point:
    //        'P' is the position
    //        'N' is the normal
    //        'texcoords' are the texture coordinates
    //        't' is the ray parameter at intersection
    //        'objIndex' is the index of the object that is hit
    //        'objPartIndex' is the index of the part of object that is hit
    //        'mat' is the material at the intersection point

    bool hit = findFirstObjectInt(rayStart, rayDir, thisObjIndex, thisObjPartIndex, P, N, texcoords, t, objIndex,
                                  objPartIndex, mat, -1);

    // No intersection: Return background colour

    if (!hit) {
        if (depth == 1)
            return backgroundColour;  // no need to weight this output, as "depth == 1" ensures that it is the
                                      // first ray from the eye.  No harm in weighting it, though.
        else
            return blackColour;
    }

    // Find reflection direction & incoming light from that direction

    Object &obj = *objects[objIndex];

    vec3 E = (-1 * rayDir).normalize();
    vec3 R = (2 * (E * N)) * N - E;

    float alpha;
    vec3 colour = obj.textureColour(P, objPartIndex, alpha, texcoords);

    vec3 kd = vec3(colour.x * mat->kd.x, colour.y * mat->kd.y, colour.z * mat->kd.z);

#if 0
  if (debug) { // 'debug' is set when tracing through a pixel that the user right-clicked
    INDENT(2*depth); cout << "texcoords " << texcoords << endl;
    INDENT(2*depth); cout << "   colour " << colour << endl;
    INDENT(2*depth); cout << "       id " << kd << endl;
    INDENT(2*depth); cout << "        P " << P << endl;
    INDENT(2*depth); cout << "        N " << N << endl;
    INDENT(2*depth); cout << "    alpha " << alpha << endl;
  }
#endif

    vec3 Iout = mat->Ie + vec3(mat->ka.x * Ia.x, mat->ka.y * Ia.y, mat->ka.z * Ia.z);

    // Compute glossy reflection

    if (mat->g < 0 || mat->g > 1) {
        cerr << "Material glossiness is outside the range [0,1]" << endl;
        exit(1);
    }

    float g = mat->g;

    if (g == 1 || numRaySamples == 1) {
        vec3 Iin = raytrace(P, R, depth, objIndex, objPartIndex);

        Iout = Iout + calcIout(N, R, E, E, kd, mat->ks, mat->n, Iin);

    } else if (g > 0) {
        // Glossy reflection
        //
        // Cast 'numRaySamples' random rays from P centred on direction R
        // using glossiness 'g' to control the spread of the rays.  For
        // glossiness 'g', the half angle of the cone of rays is
        // arccos(g).  Average the *reflected* colours of the rays and add
        // that average to Iout.  EACH INDIVIDUAL RAY has its own
        // reflected colour according to Phong, which is determined by
        // passing the individual ray through calcIout().
        vec3 TotalGlossyIout = vec3(0, 0, 0);
        float psi = g = 1 - (1 - g) / glossinessFactor;  // glossinessFactor is controlled by pressing 'G' or 'g'

        lastGlossiness = g;  // for showing in the window's status message

        // ---------------- START YOUR CODE HERE ----------------
        float halfangle = acos(g);
        float dist = 1 / tan(halfangle);
        float A, B, AandB;
        vec3 Iin, u, v, temp;
        u = R.perp1();
        v = R.perp2();

        for (int i = 0; i < numRaySamples; i++) {
            // A and B  make sure that they are between 0 and 1 as a sum
            A = 1;
            B = 1;
            AandB = pow(A, 2) + pow(B, 2);
            while (AandB > 1) {
                A = randIn01();
                B = randIn01();
                AandB = pow(A, 2) + pow(B, 2);
            }

            temp = (dist * R + A * u + B * v).normalize();
            Iin = raytrace(P, temp, depth, objIndex, objPartIndex);
            TotalGlossyIout = TotalGlossyIout + calcIout(N, temp, E, R, kd, mat->ks, mat->n, Iin);
        }

        Iout = Iout + (1.0f / numRaySamples) * TotalGlossyIout;

        // ---------------- END YOUR CODE HERE ----------------
    }

    // Add contributions from point lights

    for (int i = 0; i < lights.size(); i++) {
        Light &light = *lights[i];

        vec3 L = light.position - P;  // point light

        if (N * L > 0) {
            float Ldist = L.length();
            L = (1.0 / Ldist) * L;

            vec3 intP, intN, intTexCoords;
            float intT;
            int intObjIndex, intObjPartIndex;
            Material *intMat;

            // Is there an object between P and the light?
            //
            // Note that 'intObjIndex' will return with the index of the
            // object that is hit.  So the hit object is objects[intObjIndex].

            bool found = findFirstObjectInt(P, L, objIndex, objPartIndex, intP, intN, intTexCoords, intT, intObjIndex,
                                            intObjPartIndex, intMat, i);

            if (!found || intT > Ldist) {  // no object: Add contribution from this light
                vec3 Lr = (2 * (L * N)) * N - L;
                Iout = Iout + calcIout(N, L, E, Lr, kd, mat->ks, mat->n, light.colour);
            }
        }
    }

    // Add contributions from emitting triangles

    for (int i = 0; i < objects.size(); i++) {
        if (i != thisObjIndex) {
            Triangle *tri = dynamic_cast<Triangle *>(objects[i]);
            if (tri && tri->mat->Ie.squaredLength() > 0) {
                // Add to Iout the contribution from the emitting triangle, 'tri',
                // which has colour 'tri->mat->Ie'.
                //
                // Apply Phong *separately* to *each* shadow ray that reaches the light.
                //
                // Use 'randIn01()' to generate uniform random floats in [0,1].
                //
                // Send 'numRaySamples' rays toward the emitting triangle.

                // ---------------- BEGIN YOUR CODE HERE ----------------

                // Soft shadows

                vec3 tempIout = vec3(0, 0, 0);
                int raysThatHit = 0;

                for (int i = 0; i < numRaySamples; i++) {
                    // Rejection sampling:
                    // choose A, B in [0, 1]
                    float alpha = 1;
                    float beta = 1;
                    while (alpha + beta > 1) {
                        alpha = randIn01();
                        beta = randIn01();
                    }
                    float gamma = 1 - alpha - beta;

                    // point on tri is P = gamma*v0 + alpha*v1 + beta*v2
                    vec3 gammaV0 = gamma * tri->verts[0].position;
                    vec3 alphaV1 = alpha * tri->verts[1].position;
                    vec3 betaV2 = beta * tri->verts[2].position;
                    vec3 tempP1 = gammaV0 + alphaV1 + betaV2;
                    vec3 tempP = tempP1 - P;
                    float tempPDist = tempP.length();
                    tempP = tempP.normalize();

                    // Next, test t (distance) to see if the ray is blocked
                    vec3 intP, intN, intTexCoords;
                    float intT;
                    int intObjIndex, intObjPartIndex;
                    Material *intMat;

                    // Is there an object between P and the light?
                    //
                    // Note that 'intObjIndex' will return with the index of the
                    // object that is hit.  So the hit object is objects[intObjIndex].

                    bool found = findFirstObjectInt(P, tempP, objIndex, objPartIndex, intP, intN, intTexCoords, intT,
                                                    intObjIndex, intObjPartIndex, intMat, i);

                    // is dist to point on tri == to actual int point + epsilon?
                    float epsilon = 0.0001;

                    if (found && tempPDist < intT + epsilon && tempPDist > intT - epsilon) {
                        // for a ray that hits...
                        raysThatHit += 1;

                        vec3 Lr = (2 * (tempP * N)) * N - tempP;
                        tempIout = tempIout + calcIout(N, tempP, E, Lr, kd, mat->ks, mat->n, tri->mat->Ie);
                    }
                }

                vec3 avgRay = (1.0f / raysThatHit) * tempIout;

                vec3 intensityOfAvgRay =
                    raysThatHit == 0 ? vec3(0, 0, 0) : (1.0f / numRaySamples) * raysThatHit * avgRay;

                Iout = Iout + intensityOfAvgRay;

                // ---------------- END YOUR CODE HERE ----------------
            }
        }
    }

    return Iout;
}

// Calculate the outgoing intensity due to light Iin entering from
// direction L and exiting to direction E, with normal N.  Reflection
// direction R is provided, along with the material properties Kd,
// Ks, and n.
//
//       Iout = Iin * ( Kd (N.L) + Ks (R.V)^n )

vec3 Scene::calcIout(vec3 N, vec3 L, vec3 E, vec3 R, vec3 Kd, vec3 Ks, float ns, vec3 In)

{
    // Don't illuminate from the back of the surface

    if (N * L <= 0) return blackColour;

    // Both E and L are in front:

    vec3 Id = (L * N) * In;

    vec3 Is;

    if (R * E < 0)
        Is = blackColour;  // Is = 0 from behind the
    else                   // reflection direction
        Is = pow(R * E, ns) * In;

    return vec3(Is.x * Ks.x + Id.x * Kd.x, Is.y * Ks.y + Id.y * Kd.y, Is.z * Ks.z + Id.z * Kd.z);
}

// Determine the colour of one pixel.  This is where the raytracing
// actually starts.
//
//    (x-0.5,y-0.5) is the lower-left pixel corner.
//
//    (x+0.5,y+0.5) is the upper-right pixel corner.

vec3 Scene::pixelColour(int x, int y)

{
    if (x == debugPixel.x && y == debugPixel.y) {
        debug = true;
        cout << "---------------- start debugging at pixel " << debugPixel << " ----------------" << endl;
    }

    vec3 result;

#if 0

    // This sends a single ray through the pixel centre.  Disable this
    // section of code when your antialiasing code (below) is ready.

    vec3 dir = (llCorner + x * right + y * up).normalize();

    result = raytrace(eye->position, dir, 0, -1, -1);

#else 

    // Antialias through a pixel using ('numPixelSamples' x
    // 'numPixelSamples') rays.  Use a regular pattern in the subpixel
    // centres if 'jitter' is false; use a jittered pattern in the
    // subpixels if 'jitter' is true.

    // ---------------- START YOUR CODE HERE ----------------
    float total = 0.0;
    vec3 totalColor = vec3(0, 0, 0);
    float xOffset, yOffset;
    for (int row = 0; row < numPixelSamples; row++) {
        for (int col = 0; col < numPixelSamples; col++) {
            if (jitter == false) {
                 xOffset = (col + 0.5f) / numPixelSamples;
                 yOffset = (row + 0.5f) / numPixelSamples;
            }
            else {
                 xOffset = (col + 0.5f+randIn01()) / numPixelSamples;
                 yOffset = (row + 0.5f+randIn01()) / numPixelSamples;

            }

            vec3 dir = (llCorner + (x+ xOffset) * right + (y+yOffset) * up).normalize();
            totalColor = totalColor + raytrace(eye->position, dir, 0, -1, -1);
        }
    }

    result = 1/(numPixelSamples*numPixelSamples)*totalColor;

    // ---------------- END YOUR CODE HERE ----------------

#endif

    if (storingRays) storingRays = false;

    if (debug) {
        cout << "---------------- stop debugging ----------------" << endl;
        debug = false;
    }

    return result;
}

// Read the scene from an input stream

void Scene::read(const char *basename, istream &in)

{
    char command[1000];

    while (in) {
        skipComments(in);
        in >> command;
        if (!in || command[0] == '\0') break;

        skipComments(in);

        if (strcmp(command, "sphere") == 0) {
            Sphere *o = new Sphere();
            in >> *o;
            objects.add(o);

        } else if (strcmp(command, "triangle") == 0) {
            Triangle *o = new Triangle();
            in >> *o;
            objects.add(o);

        } else if (strcmp(command, "material") == 0) {
            Material *m = new Material(basename);
            in >> *m;
            materials.add(m);

        } else if (strcmp(command, "wavefront") == 0) {
            // Rely on the wavefront.cpp code to read this

            string filename;
            in >> filename;

            char pathname[1000];
            sprintf(pathname, "%s/%s", basename, filename.c_str());

            WavefrontObj *o = new WavefrontObj(pathname);
            objects.add(o);

            // Update scene's scale

            if (o->obj->radius / 2 > sceneScale) sceneScale = o->obj->radius / 2;

        } else if (strcmp(command, "light") == 0) {
            Light *o = new Light();
            in >> *o;
            lights.add(o);

        } else if (strcmp(command, "eye") == 0) {
            eye = new Eye();
            in >> *eye;

            win->arcball->setV(eye->position, eye->lookAt, eye->upDir);
            win->fovy = eye->fovy;

        } else {
            cerr << "Command '" << command << "' not recognized" << endl;
            exit(-1);
        }
    }

    if (lights.size() == 0) {
        cerr << "No lights were provided in " << basename << " so the scene would be black." << endl;
        exit(1);
    }
}

// Output the whole scene (mainly for debugging the reader)

void Scene::write(ostream &out)

{
    out << *eye << endl;

    for (int i = 0; i < lights.size(); i++) out << *lights[i] << endl;

    for (int i = 0; i < materials.size(); i++) out << *materials[i] << endl;

    for (int i = 0; i < objects.size(); i++) out << *objects[i] << endl;
}

char *Scene::statusMessage()

{
    static char buffer[1000];

    if (lastGlossiness > 0)
        sprintf(buffer, "%dx%d pixel rays, %d sample rays, glossiness %.6g%s", numPixelSamples, numPixelSamples,
                (int)numRaySamples, lastGlossiness, (jitter ? ", jitter" : ""));
    else
        sprintf(buffer, "%dx%d pixel rays, %d sample rays%s", numPixelSamples, numPixelSamples, (int)numRaySamples,
                (jitter ? ", jitter" : ""));

    return buffer;
}

// Draw the scene.  This sets things up and simply
// calls pixelColour() for each pixel.

void Scene::renderRT(bool restart)

{
    static int nextx, nexty;
    static float lastDisplayTime = 0;

    mat4 WCS_to_VCS = win->arcball->V;

    mat4 VCS_to_CCS = perspective(win->fovy, windowWidth / (float)windowHeight, 1, 1000);

    if (restart) {
        srand(754376105);

        // Copy the window eye into the scene eye

        eye->position = win->arcball->eyePosition();
        eye->lookAt = win->arcball->lookAt();
        eye->upDir = win->arcball->upDirection();
        eye->fovy = win->fovy;

        vec3 rightDir = ((eye->lookAt - eye->position) ^ eye->upDir).normalize();

        // Compute the image plane coordinate system

        up = (2.0 * tan(eye->fovy / 2.0)) * eye->upDir.normalize();

        right = (2.0 * tan(eye->fovy / 2.0) * windowWidth / (float)windowHeight) * rightDir.normalize();

        llCorner = (eye->lookAt - eye->position).normalize() - 0.5 * up - 0.5 * right;

        up = (1.0 / (float)(windowHeight - 1)) * up;
        right = (1.0 / (float)(windowWidth - 1)) * right;

        nextx = 0;
        nexty = 0;

        stop = false;

        // Clear the RT image

        if (rtImage != NULL) delete[] rtImage;

        rtImage = NULL;
    }

    // Set up a new RT image

    if (rtImage == NULL) {
        rtImage = new vec4[(int)(windowWidth / pixelScale * windowHeight / pixelScale)];
        for (int i = 0; i < windowWidth / pixelScale * windowHeight / pixelScale; i++)
            rtImage[i] = vec4(0, 0, 0, 0);  // transparent
    }

    if (stop) return;

    // Draw the next pixel

    vec3 colour = pixelColour((nextx + 0.5) * pixelScale, (nexty + 0.5) * pixelScale);

    rtImage[nextx + nexty * (int)(windowWidth / pixelScale)] = vec4(colour.x, colour.y, colour.z, 1);  // opaque

    // Move (nextx,nexty) to the next pixel

    nexty++;
    if (nexty >= windowHeight / pixelScale) {
        nexty = 0;
        nextx++;

        if (nextx >= windowWidth / pixelScale) {  // finished
            draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);
            nextx = 0;
            stop = true;
            cout << "\r           \r";
            cout.flush();
        }
    } else {
        float thisTime = getTime();
        if (thisTime > lastDisplayTime + DISPLAY_INTERVAL) {
            draw_RT_and_GL(WCS_to_VCS, VCS_to_CCS);
            lastDisplayTime = thisTime;
        }
    }
}

// Render the scene with OpenGL

void Scene::renderGL(mat4 &WCS_to_VCS, mat4 &VCS_to_CCS)

{
    mat4 WCS_to_CCS = VCS_to_CCS * WCS_to_VCS;

    // create axes and segs here so that they are not created before
    // the window is initialized

    if (axes == NULL) axes = new Axes();

    if (segs == NULL) segs = new Segs();

    vec3 lightDir = vec3(1, 1, 1).normalize();

    // Set up the framebuffer

    glEnable(GL_DEPTH_TEST);
    glClearColor(backgroundColour.x, backgroundColour.y, backgroundColour.z, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Copy the window eye into the scene eye

    eye->position = win->arcball->eyePosition();
    eye->lookAt = win->arcball->lookAt();
    eye->upDir = win->arcball->upDirection();
    eye->fovy = win->fovy;

    // Draw the objects

    wavefrontGPU->activate();

    // Need "headlights" to illuminate non-scene items

    vec3 lightPos[MAX_NUM_LIGHTS] = {vec3(0, 0, 0)};  // light is at eye position
    vec3 lightIin[MAX_NUM_LIGHTS] = {vec3(1, 1, 1)};

    wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
    wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

    wavefrontGPU->setInt("numLights", 1);

    // Ambient lighting

    wavefrontGPU->setVec3("Ia", vec3(1, 1, 1));

    // Show the lights

    for (int i = 0; i < lights.size(); i++) lights[i]->draw(wavefrontGPU, WCS_to_VCS, VCS_to_CCS, lightDir);

    // Draw any stored rays (for debugging)

    drawStoredRays(wavefrontGPU, WCS_to_VCS, VCS_to_CCS);

    wavefrontGPU->deactivate();

    // Draw axes

    if (showAxes) axes->draw(WCS_to_CCS);

    // First define scene lights

    int numLights = (lights.size() < MAX_NUM_LIGHTS ? lights.size() : MAX_NUM_LIGHTS);

    for (int i = 0; i < numLights; i++) {
        lightPos[i] = (WCS_to_VCS * vec4(lights[i]->position, 1.0)).toVec3();
        lightIin[i] = lights[i]->colour;
    }

    wavefrontGPU->activate();

    wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
    wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

    wavefrontGPU->setInt("numLights", numLights);

    // Now draw everything

    if (showObjects)
        for (int i = 0; i < objects.size(); i++) objects[i]->renderGL(wavefrontGPU, WCS_to_VCS, VCS_to_CCS);

    // Show status message

    strokeFont->drawStrokeString(statusMessage(), -0.95, -0.95, TEXT_SIZE, 0, LEFT, vec3(1, 1, 1));

    // Done

    wavefrontGPU->deactivate();
}

// Draw any stored rays (for debugging)

#define BASE_ARROW_RADIUS 0.01

void Scene::drawStoredRays(GPUProgram *gpuProg, mat4 &WCS_to_VCS, mat4 &VCS_to_CCS)

{
    if (storedRays.size() > 0) {
        if (arrow == NULL) arrow = new Arrow();

        for (int i = 0; i < storedRays.size(); i += 2) {
            vec3 dir = storedRays[i] - storedRays[i + 1];
            mat4 OCS_to_WCS = translate(storedRays[i + 1]) * rotate(vec3(0, 0, 1), dir);

            arrow->mat->kd = storedRayColours[i / 2];

            arrow->draw(gpuProg, OCS_to_WCS, WCS_to_VCS, VCS_to_CCS,
                        (i == 0 ? 0.25 : 1.0) * dir.length(),  // first ray (going back to eye) is shorter
                        sceneScale * BASE_ARROW_RADIUS);
        }
    }
}

// Draw the scene, then draw the RT image on top of it.  Transparent
// pixels in the RT image are those that have not been calculated yet.

void Scene::draw_RT_and_GL(mat4 &WCS_to_VCS, mat4 &VCS_to_CCS)

{
    renderGL(WCS_to_VCS, VCS_to_CCS);  // GL rendering

    drawRTImage();  // RT image on top

    // Redraw the stored rays and lights over top of the RT image

    wavefrontGPU->activate();

    // Need "headlights" to illuminate non-scene items

    vec3 lightPos[MAX_NUM_LIGHTS] = {vec3(0, 0, 0)};  // light is at eye position
    vec3 lightIin[MAX_NUM_LIGHTS] = {vec3(1, 1, 1)};

    wavefrontGPU->setVec3("lightIin", lightIin, MAX_NUM_LIGHTS);
    wavefrontGPU->setVec3("lightPos", lightPos, MAX_NUM_LIGHTS);

    wavefrontGPU->setInt("numLights", 1);

    vec3 lightDir = vec3(1, 1, 1).normalize();  // only for OpenGL rendering

    mat4 VP = translate(0, 0, -0.1) * VCS_to_CCS;  // move a little forward in the CCS so as to overdraw the
                                                   // previously-drawn arrow at this location (depth-buffer issue)
    for (int i = 0; i < lights.size(); i++) lights[i]->draw(wavefrontGPU, WCS_to_VCS, VP, lightDir);

    drawStoredRays(wavefrontGPU, WCS_to_VCS, VP);

    wavefrontGPU->deactivate();

    // Perhaps show the pixel zoom

    if (buttonDown == GLFW_MOUSE_BUTTON_LEFT && (buttonMods & GLFW_MOD_CONTROL)) {
        pixelZoom->zoom(win->window, mouse, vec2(windowWidth, windowHeight));
        win->draggingWasDone = true;
    }

    // Status message

    glDisable(GL_DEPTH_TEST);
    strokeFont->drawStrokeString(statusMessage(), -0.95, -0.95, TEXT_SIZE, 0, LEFT, vec3(1, 1, 1));
    glEnable(GL_DEPTH_TEST);

    // Done

    glfwSwapBuffers(win->window);
}

void Scene::drawRTImage()

{
    if (rtImage == NULL) return;

    if (gpu == NULL) {
        gpu = new GPUProgram();
        gpu->init(rtTextureVertShader, rtTextureFragShader, "in Scene::drawRTImage");
    }

    // Send texture to GPU

    if (rtImageTexID == 0) glGenTextures(1, &rtImageTexID);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, rtImageTexID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth / pixelScale, windowHeight / pixelScale, 0, GL_RGBA, GL_FLOAT,
                 rtImage);

    // Draw texture on a full-screen quad

    vec2 verts[8] = {
        vec2(-1, -1), vec2(-1, 1), vec2(1, -1), vec2(1, 1),  // positions
        vec2(0, 0),   vec2(0, 1),  vec2(1, 0),  vec2(1, 1)   // texture coordinates
    };

    GLuint VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(vec2) * 4));

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gpu->activate();
    gpu->setInt("texUnitID", 1);
    gpu->setInt("texturing", 1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gpu->deactivate();

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glBindTexture(GL_TEXTURE_2D, 0);
}

const char *Scene::rtTextureVertShader = R"(

  #version 300 es

  layout (location = 0) in mediump vec2 posIn;
  layout (location = 1) in mediump vec2 texCoordsIn;

  //smooth out mediump vec2 pos;
  smooth out mediump vec2 texCoords;

  void main() {
    gl_Position = vec4( posIn, 0, 1 );
    //pos = posIn;
    texCoords = texCoordsIn;
  }
)";

const char *Scene::rtTextureFragShader = R"(

  #version 300 es

  uniform sampler2D texUnitID;
  uniform bool texturing;

  //smooth in mediump vec2 pos;
  smooth in mediump vec2 texCoords;
  out mediump vec4 outputColour;

  void main() {

    if (texturing) {
      mediump vec4 c = texture( texUnitID, texCoords );
      outputColour = c;
    } else
      outputColour = vec4(0.5,0.5,0.5,1);
  }
)";

const char *Scene::wavefrontVertexShader = R"(
  // phong vertex shader with textures

  #version 300 es

  uniform mat4 MVP;		// OCS-to-CCS
  uniform mat4 MV;		// OCS-to-VCS

  layout (location = 0) in vec3 vertPosition;
  layout (location = 1) in vec3 vertNormal;
  layout (location = 2) in vec3 vertTexCoords; // just another vertex attribute

  smooth out mediump vec3 position;
  smooth out mediump vec3 normal;
  smooth out mediump vec3 texCoords;
  //smooth out mediump float depth;

  void main()

  {
    // calc vertex position in CCS

    gl_Position = MVP * vec4( vertPosition, 1.0 );

    // vertex position in VCS

    position = (MV * vec4( vertPosition, 1.0 )).xyz;

    // calc normal in VCS
    //
    // To do this, apply the non-translational parts of MV to the model
    // normal.  The 0.0 as the fourth component of the normal ensures
    // that no translational component is added.

    normal = vec3( MV * vec4( vertNormal, 0.0 ) );

    // Texture coordinates

    texCoords = vertTexCoords;

    // Depth

    //vec4 ccs = MVP * vec4( vertPosition, 1.0 );
    //depth = ((ccs.z / ccs.w) + 1.0) * 0.5;
  }
)";

const char *Scene::wavefrontFragmentShader = R"(
  // Phong fragment shader with textures
  //
  // Phong shading with diffuse and specular components

  #version 300 es

  uniform sampler2D objTexture;
  uniform bool      texturing;        // =1 if object's texture is provided

  uniform mediump vec3 lightPos[4];   // array size must match MAX_NUM_LIGHTS (= 4) in scene.cpp
  uniform mediump vec3 lightIin[4];   // array size must match MAX_NUM_LIGHTS (= 4) in scene.cpp
  uniform int numLights;
  uniform bool useHeadlights;

  uniform mediump vec3 kd;
  uniform mediump vec3 ks;
  uniform mediump vec3 ka;
  uniform mediump vec3 Ia;
  uniform mediump vec3 Ie;
  uniform mediump float shininess;

  smooth in mediump vec3 position;
  smooth in mediump vec3 normal;
  smooth in mediump vec3 texCoords;
  //smooth in mediump float depth;

  out mediump vec4 outputColour;

  void main()

  {
    mediump vec3 N = normalize( normal );

    // Compute the outgoing colour

    mediump vec3 Iout = vec3(0,0,0);

    for (int i=0; i<numLights; i++) { // add contribution from each (positional) light

      mediump vec3 L = normalize( lightPos[i] - position );      

      mediump float NdotL = dot( N, L );

      if (NdotL > 0.0) {

	// diffuse component

	mediump vec3 colour;

	if (texturing)
	  colour = texture( objTexture, texCoords.st ).rgb * kd;
	else
	  colour = kd;

	Iout += NdotL * (colour * lightIin[i]);

	Iout += (ka * Ia) * colour;

	// specular component

	mediump vec3 R = (2.0 * NdotL) * N - L;
	mediump vec3 V = vec3(0.0,0.0,1.0);	// direction toward eye in the VCS

	mediump float RdotV = dot( R, V );

	if (RdotV > 0.0)
	  Iout += pow( RdotV, shininess ) * (ks * lightIin[i]);
      }
    }

    outputColour = vec4( Iout, 1.0 );
  }
)";
