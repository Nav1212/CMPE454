/* sphere.C
 */

#include "sphere.h"

// Return the distance between 'this' sphere and 'otherSphere'.

float Sphere::distToSphere(Sphere &otherSphere)

{
    return (otherSphere.state.x - this->state.x).length() - this->radius - otherSphere.radius;
}

// Return the distance between 'this' sphere and 'rectangle'.  Also
// set 'closestPoint' to the point on the rectangle that is closest to
// the sphere.

float Sphere::distToRectangle(Rectangle &rectangle, vec3 *closestPoint)

{
    // Find transform that moves rectangle back to [-xDim/2,+xDim/2] x [-yDim/2,+yDim/2] x [0] centred at origin with
    // normal = z

    mat4 Minv = rectangle.OCS_to_WCS().inverse();

    // Apply same to sphere centre.  Now the sphere centre is in the
    // coordinate system of the rectangle, which has the dimensions
    // rectangle.xDim x rectangle.yDim, centre at (0,0,0), and normal
    // (0,0,1).

    vec3 sphereCentre = (Minv * vec4(this->state.x, 1.0)).toVec3();

    // If the z projection of the sphere is inside the xy rectangle,
    // find the closestPoint and the distance.

    // [YOUR CODE HERE: REPLACE THE CODE BELOW]
        
    if (sphereCentre.x+this->radius > -rectangle.xDim / 2.0f && sphereCentre.x - this->radius< rectangle.xDim / 2.0f &&
        sphereCentre.y+this->radius > -rectangle.yDim / 2.0f && sphereCentre.y - this->radius < rectangle.yDim / 2.0f) {
        float xClamp = std::max(-rectangle.xDim / 2.0f, std::min(sphereCentre.x, rectangle.xDim / 2.0f));
        float yClamp = std::max(-rectangle.yDim / 2.0f, std::min(sphereCentre.y, rectangle.yDim / 2.0f));
        vec3 closestPointInRectangleCoords(xClamp, yClamp, 0);

        *closestPoint = (rectangle.OCS_to_WCS() * vec4(closestPointInRectangleCoords, 1.0)).toVec3();
        float distance = (sphereCentre - closestPointInRectangleCoords).length() - this->radius;

        return distance;
    }

    // [END OF YOUR CODE ABOVE]

    // Determine the distance to each edge, named Xplus, Xminus, Yplus, Yminus below.
    //
    // Xplus has y = xDim/2, Xminus has y = -xDim/2, Yplus has x = yDim/2, Yminus has x = -yDim/2.
    //
    // For example, the Xplus edge goes from (xDim/2, -yDim/2, 0) to (xDim/2, yDim/2, 0).
    //
    // Call pointToEdgeDistance(...) in linalg.cpp to compute the
    // distances to each edge, and the closest point on each edge.

    vec3 pointXplus, pointYplus, pointXminus, pointYminus;  // closest points

    float distXplus, distYplus, distXminus, distYminus;  // closest distances

    // [YOUR CODE HERE]

    vec3 XplusH = vec3(rectangle.xDim / 2.0f, -rectangle.yDim / 2.0f, 0);
    vec3 XplusT = vec3(rectangle.xDim / 2.0f, rectangle.yDim / 2.0f, 0);

    vec3 XminusH = vec3(-rectangle.xDim / 2.0f, -rectangle.yDim / 2.0f, 0);
    vec3 XminusT = vec3(-rectangle.xDim / 2.0f, rectangle.yDim / 2.0f, 0);

    vec3 YplusH = vec3(-rectangle.xDim / 2.0f, rectangle.yDim / 2.0f, 0);
    vec3 YplusT = vec3(rectangle.xDim / 2.0f, rectangle.yDim / 2.0f, 0);

    vec3 YminusH = vec3(-rectangle.xDim / 2.0f, -rectangle.yDim / 2.0f, 0);
    vec3 YminusT = vec3(rectangle.xDim / 2.0f, -rectangle.yDim / 2.0f, 0);

    distXplus = pointToEdgeDistance(sphereCentre, XplusH, XplusT, &pointXplus);
    distYplus = pointToEdgeDistance(sphereCentre, YplusH, YplusT, &pointYplus);
    distXminus = pointToEdgeDistance(sphereCentre, XminusH, XminusT, &pointXminus);
    distYminus = pointToEdgeDistance(sphereCentre, YminusH, YminusT, &pointYminus);

    // Pick the minimum of the edge distances

    float min = distXplus;
    vec3 pt = pointXplus;

    if (distYplus < min) {
        min = distYplus;
        pt = pointYplus;
    }

    if (distXminus < min) {
        min = distXminus;
        pt = pointXminus;
    }

    if (distYminus < min) {
        min = distYminus;
        pt = pointYminus;
    }

    // Move 'pt', which is in the rectangle's coordinate system, back to
    // the WCS and store it as 'closestPoint'.  Return the distance from
    // the edge to the sphere surface.

    // [YOUR CODE HERE: REPLACE THE CODE BELOW]

    *closestPoint = (rectangle.OCS_to_WCS() * vec4(pt, 1.0)).toVec3();
    float distance = (sphereCentre - pt).length() - this->radius;

    return distance;
}

// icosahedron vertices (taken from Jon Leech http://www.cs.unc.edu/~jon)

vec3 Sphere::icosahedronVerts[NUM_VERTS] = {vec3(tau, one, 0),   vec3(-tau, one, 0),  vec3(-tau, -one, 0),
                                            vec3(tau, -one, 0),  vec3(one, 0, tau),   vec3(one, 0, -tau),
                                            vec3(-one, 0, -tau), vec3(-one, 0, tau),  vec3(0, tau, one),
                                            vec3(0, -tau, one),  vec3(0, -tau, -one), vec3(0, tau, -one)};

// icosahedron faces (taken from Jon Leech http://www.cs.unc.edu/~jon)

int Sphere::icosahedronFaces[NUM_FACES][3] = {
    {4, 8, 7}, {4, 7, 9},  {5, 6, 11}, {5, 10, 6}, {0, 4, 3},  {0, 3, 5},  {2, 7, 1},
    {2, 1, 6}, {8, 0, 11}, {8, 11, 1}, {9, 10, 3}, {9, 2, 10}, {8, 4, 0},  {11, 0, 5},
    {4, 9, 3}, {5, 3, 10}, {7, 8, 1},  {6, 1, 11}, {7, 2, 9},  {6, 10, 2},
};

// Add a level to the sphere

void Sphere::refine()

{
    int n = faces.size();

    for (int i = 0; i < n; i++) {
        SphereFace f = faces[i];

        verts.add((verts[f.v[0]] + verts[f.v[1]]).normalize());
        verts.add((verts[f.v[1]] + verts[f.v[2]]).normalize());
        verts.add((verts[f.v[2]] + verts[f.v[0]]).normalize());

        int v01 = verts.size() - 3;
        int v12 = verts.size() - 2;
        int v20 = verts.size() - 1;

        faces.add(SphereFace(f.v[0], v01, v20));
        faces.add(SphereFace(f.v[1], v12, v01));
        faces.add(SphereFace(f.v[2], v20, v12));

        faces[i].v[0] = v01;
        faces[i].v[1] = v12;
        faces[i].v[2] = v20;
    }
}

void Sphere::setupVAO()

{
    // Create a VAO

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // store vertices (i.e. one triple of floats per vertex)

    GLuint vertexBufferID;
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(vec3), (void *)&verts[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Set up face indices.  These are collected from the sphere's
    // 'seq<vec3> verts' and 'seq<SphereFace> faces' structures.

    GLuint *indexBuffer = new GLuint[faces.size() * 3];

    for (int i = 0; i < faces.size(); i++) {
        // Determine whether vertices are CW or CCW

        vec3 normal = 1 / 3.0 * (verts[faces[i].v[0]] + verts[faces[i].v[1]] + verts[faces[i].v[2]]);
        vec3 cross = (verts[faces[i].v[1]] - verts[faces[i].v[0]]) ^ (verts[faces[i].v[2]] - verts[faces[i].v[0]]);

        if (normal * cross > 0)  // CW
            for (int j = 0; j < 3; j++) indexBuffer[3 * i + j] = faces[i].v[j];
        else  // CCW
            for (int j = 2; j >= 0; j--) indexBuffer[3 * i + j] = faces[i].v[j];
    }

    // store faces (i.e. one triple of vertex indices per face)

    GLuint indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * 3 * sizeof(GLuint), indexBuffer, GL_STATIC_DRAW);

    // Clean up

    delete[] indexBuffer;

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Sphere::draw(mat4 &WCS_to_VCS, mat4 &VCS_to_CCS, vec3 &lightDir, vec3 &colour)

{
    mat4 MV = WCS_to_VCS * OCS_to_WCS() * scale(radius, radius, radius);
    mat4 MVP = VCS_to_CCS * MV;

    gpu->activate();

    gpu->setMat4("MV", MV);
    gpu->setMat4("MVP", MVP);
    gpu->setVec3("colour", colour);
    gpu->setVec3("lightDir", lightDir);

    // Draw using element array

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, faces.size() * 3, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    gpu->deactivate();
}

const char *Sphere::vertShader = R"XX(

  #version 300 es

  precision mediump float;

  uniform mat4 MVP;
  uniform mat4 MV;

  layout (location = 0) in vec3 vertPosition;

  smooth out vec3 normal;

  void main() {

    gl_Position = MVP * vec4( vertPosition, 1.0 );

    normal = vec3( MV * vec4( vertPosition, 0.0 ) );  // positions are on unit sphere, so positions == normals
  }
)XX";

const char *Sphere::fragShader = R"XX(

  #version 300 es

  precision mediump float;

  uniform vec3 colour;
  uniform vec3 lightDir;

  smooth in vec3 normal;
  out vec4 outputColour;

  void main() {

    float NdotL = dot( normalize(normal), lightDir );

    if (NdotL < 0.1)
      NdotL = 0.1; // some ambient

    outputColour = vec4( NdotL * colour, 1.0 );
  }
)XX";
