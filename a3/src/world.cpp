// world.cpp

#include "world.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>

#include "main.h"

#define ZERO_ORIENTATION quaternion(0, vec3(1, 0, 0))
#define ZERO_VELOCITY vec3(0, 0, 0)
#define ZERO_ANG_VELOCITY vec3(0, 0, 0)

#define PIT_DEPTH 0.2

#define NUM_SPHERES_TO_GEN 10
#define MIN_SPHERE_RADIUS 0.08
#define MAX_SPHERE_RADIUS 0.12
#define MIN_DIST_BETWEEN_SPHERES 0.1

#define SPHERE_VOLUME_MIN \
    vec3(-1.5 + MIN_SPHERE_RADIUS, -1 + MIN_SPHERE_RADIUS, 0.5)  // volume in which spheres are generated
#define SPHERE_VOLUME_MAX vec3(1.5 - MIN_SPHERE_RADIUS, 1 - MIN_SPHERE_RADIUS, 2.5)

#define MAX_SPHERE_GEN_ATTEMPTS 100

#define GRAVITY_ACCEL vec3(0, 0, -9.8)  // m/s/s

#define MIN_DELTA_T_FOR_COLLISIONS \
    0.001  // minimum delta-t between a non-collision state and a collision state (for binary search)

#define COEFF_OF_RESTITUTION -0.8  // same between all pairs of objects

#define MIN_NORMAL_DISTANCE \
    0.05  // distance below which a sphere comes to rest perp to the plane (can still have parallel motion)
#define MIN_NORMAL_SPEED 0.05  // speed below which a sphere comes to rest

#define MAX_TIME_STEP 0.001  // max time of one integration step

#define RECTANGLE_EDGE_BUFFER 0.03  // distance beyond rectangle edge after which to release constrained sphere

#define MIN_SPHERE_Z -5  // height below which sphere is removed

#define W2(x) std::setw(2) << std::setfill('0') << std::fixed << (x)

// Rectangle definitions

#define NUM_RECTANGLES 9

const RectangleDef World::initRectangles[] = {

    // xDim, yDim, normal, centre

    {3.02, 2.02, vec3(0, 0, 1), vec3(0, 0, -PIT_DEPTH)},  // pit floor

    {3, PIT_DEPTH, vec3(0, -1, 0), vec3(0, 1, -PIT_DEPTH / 2.0)},  // pit sides
    {3, PIT_DEPTH, vec3(0, 1, 0), vec3(0, -1, -PIT_DEPTH / 2.0)},
    {PIT_DEPTH, 2.02, vec3(-1, 0, 0), vec3(-1.5, 0, -PIT_DEPTH / 2.0)},
    {PIT_DEPTH, 2.02, vec3(1, 0, 0), vec3(1.5, 0, -PIT_DEPTH / 2.0)},

    {9.02, 3.52, vec3(0, 0, 1), vec3(0, -2.75, 0)},  // ground around pit
    {9.02, 3.52, vec3(0, 0, 1), vec3(0, 2.75, 0)},
    {3.02, 2.02, vec3(0, 0, 1), vec3(-3, 0, 0)},
    {3.02, 2.02, vec3(0, 0, 1), vec3(3, 0, 0)}};

// World constructor

World::World(char *sphereFilename)

{
    // Add the rectangles defined above in 'initRectangles'

    for (int i = 0; i < NUM_RECTANGLES; i++)
        rectangles.add(Rectangle(initRectangles[i].xDim, initRectangles[i].yDim, initRectangles[i].normal,
                                 initRectangles[i].centre, ZERO_ORIENTATION, ZERO_VELOCITY, ZERO_ANG_VELOCITY));

    // Read spheres from file (if provided)

    if (sphereFilename != NULL) {
        ifstream in(sphereFilename);
        float radius;
        vec3 centre;

        while (in >> radius >> centre)
            spheres.add(Sphere(SPHERE_LEVELS, radius, centre, ZERO_ORIENTATION, ZERO_VELOCITY, ZERO_ANG_VELOCITY));

        cout << "Read " << spheres.size() << " spheres from " << sphereFilename << endl;
        return;
    }

    // Otherwise, generate spheres randomly in the [SPHERE_VOLUME_MIN, SPHERE_VOLUME_MAX] volume
    //
    // Ensure that they are separated by at least MIN_DIST_BETWEEN_SPHERES

    srand(23546234);

    vec3 sphereCentres[NUM_SPHERES_TO_GEN];
    float sphereRadii[NUM_SPHERES_TO_GEN];

    int numSpheres = 0;

    for (int i = 0; i < NUM_SPHERES_TO_GEN; i++) {
        vec3 centre;
        float radius;
        float minDist;

        int attempts = 0;
        do {
            // Generate a position

            for (int j = 0; j < 3; j++)
                centre[j] = randIn01() * (SPHERE_VOLUME_MAX[j] - SPHERE_VOLUME_MIN[j]) + SPHERE_VOLUME_MIN[j];

            radius = randIn01() * (MAX_SPHERE_RADIUS - MIN_SPHERE_RADIUS) + MIN_SPHERE_RADIUS;

            // Find min distance to other sphere centres

            minDist = FLT_MAX;
            for (int k = 0; k < i; k++) {
                float dist =
                    (centre - sphereCentres[k]).length() - sphereRadii[k] - radius;  // distance between sphere surfaces
                if (dist < minDist) minDist = dist;
            }

            attempts++;

        } while (minDist < MIN_DIST_BETWEEN_SPHERES && attempts < MAX_SPHERE_GEN_ATTEMPTS);  // try again if too close

        if (attempts < MAX_SPHERE_GEN_ATTEMPTS) {
            sphereRadii[numSpheres] = radius;
            sphereCentres[numSpheres] = centre;
            numSpheres++;

            spheres.add(Sphere(SPHERE_LEVELS, radius, centre, ZERO_ORIENTATION, ZERO_VELOCITY, ZERO_ANG_VELOCITY));
        }
    }

    if (numSpheres < NUM_SPHERES_TO_GEN)
        cout << "Only generated " << numSpheres << " spheres instead of the desired " << NUM_SPHERES_TO_GEN
             << ", likely due to crowding the the generation volume." << endl;

    // Record this last set of sphere in case we want to debug with
    // the same spheres that were randomly generated.

    ofstream out("../tests/lastSpheres.txt");

    for (int i = 0; i < numSpheres; i++) out << spheres[i].radius << " " << spheres[i].state.x << endl;
}

// Integrate
//
// Given the state at yStart, integrate over time deltaT to get state yEnd.
//
// Update the sphere states to yEnd.  Then call 'findCollisions' to
// set collisionAtEnd, collisionSphere, and collisionObject.

void World::integrate(State *yStart, State *yEnd, float deltaT, bool &collisionAtEnd, Sphere **collisionSphere,
                      Object **collisionObject)

{
    int nSpheres = spheres.size();

    State *y = new State[nSpheres];
    State *yDeriv = new State[nSpheres];

    // For each state in 'yStart', copy it to 'y' and set 'yDeriv'
    // appropriately.  Use GRAVITY_ACCEL as the velocity derivative.
    // Set the angular velocity derivative to zero.

    // [YOUR CODE HERE]
    for (int i = 0; i < nSpheres; i++) {
        // For each state in 'yStart', copy it to 'y' and set 'yDeriv'
        // appropriately. Use GRAVITY_ACCEL as the velocity derivative.
        // Set the angular velocity derivative to zero.

        // [YOUR CODE HERE]

        float q0 = yStart[i].q.q0;
        float q1 = yStart[i].q.q1;
        float q2 = yStart[i].q.q2;
        float q3 = yStart[i].q.q3;

        mat4 Q;
        Q.rows[0] = vec4(-q1, -q2, -q3, 0);
        Q.rows[1] = vec4(q0, -q3, q2, 0);
        Q.rows[2] = vec4(q3, q0, q1, 0);
        Q.rows[3] = vec4(-q2, q1, q0, 0);
        Q = 0.5 * Q;
        vec4 Qw = Q * vec4(yStart[i].w, 0);

        y[i].x = yStart[i].x;
        y[i].v = yStart[i].v;
        y[i].q = yStart[i].q;
        y[i].w = yStart[i].w;

        yDeriv[i].x = yStart[i].v;                           // velocity
        yDeriv[i].v = GRAVITY_ACCEL;                         // acceleration
        yDeriv[i].q = quaternion(Qw.x, Qw.y, Qw.z, Qw.w);    // Qw
        yDeriv[i].w = vec3(0, 0, 0);

        // Integrate: Compute yEnd = yStart + deltaT * yDeriv
        //
        // Do this on the individual floats in 'y' and 'yDeriv'. Do not
        // refer to the sphere states here.

        // [YOUR CODE HERE]

        yEnd[i].x = y[i].x + (deltaT * yDeriv[i].x);
        yEnd[i].v = y[i].v + (deltaT * yDeriv[i].v);
        yEnd[i].q = quaternion(y[i].q.q0 + (deltaT * yDeriv[i].q.q0), y[i].q.q1 + (deltaT * yDeriv[i].q.q1),
            y[i].q.q2 + (deltaT * yDeriv[i].q.q2), y[i].q.q3 + (deltaT * yDeriv[i].q.q3));
        yEnd[i].w = y[i].w + (deltaT * yDeriv[i].w);

        // Copy yEnd state into sphere states
        copyState(&yEnd[i], &spheres[i]);
    }

        // Check for collisions

        collisionAtEnd = findCollisions(collisionSphere, collisionObject);

        // Clean up
    
    delete[] y;
    delete[] yDeriv;
}

// Update the world state
//
// Move in integration steps of at most 'maxTimeStep'

void World::updateState(float elapsedTime)

{
    float simulatedElapsedTime = timeFactor * elapsedTime;

    float actualDeltaT = 0;

    // Go in steps of MAX_TIME_STEP until deltaT

    while (actualDeltaT < simulatedElapsedTime - MAX_TIME_STEP)
        actualDeltaT += updateStateByDeltaT(MAX_TIME_STEP);  // might advance less than MAX_TIME_STEP

    while (actualDeltaT < simulatedElapsedTime)
        actualDeltaT += updateStateByDeltaT(simulatedElapsedTime - actualDeltaT);

    // Remove any spheres that have fallen far off the base

    for (int i = 0; i < spheres.size(); i++)
        if (spheres[i].state.x.z < MIN_SPHERE_Z) {
            spheres.remove(i);
            i--;
        }
}

// Advance the state by time deltaT, or to the first collision that
// occurs.  If a collision occurs, resolve it and set the state to the
// time just before the collision.

float World::updateStateByDeltaT(float deltaT)

{
    if (spheres.size() == 0) return deltaT;

    // For spheres constrained to be rolling on rectangles, set the
    // sphere velocity normal to the rectangle to be zero.  Also ensure
    // that the sphere touches the rectangle and a single point.
    //
    // Also: Remove the constraint when the sphere moves off the
    // rectangle.
    //
    // This is not very realistic, as there's no rolling.

    for (int i = 0; i < spheres.size(); i++) {
        Sphere &s = spheres[i];
        for (int j = 0; j < spheres[i].constraintRectangles.size(); j++) {
            Rectangle &r = *(spheres[i].constraintRectangles[j]);

            // Check for constraint removal

            vec3 sphereCentre =
                (r.OCS_to_WCS().inverse() * vec4(s.state.x, 1.0)).toVec3();  // now in coordinate system of rectangle

            if (fabs(sphereCentre.x) > r.xDim / 2.0 + RECTANGLE_EDGE_BUFFER ||
                fabs(sphereCentre.y) > r.yDim / 2.0 + RECTANGLE_EDGE_BUFFER) {
                spheres[i].constraintRectangles.remove(j);
#if 0
	cout << "Removed s" << W2(i) << "-r" << W2(j) << " constraint" << endl;
#endif
                j--;
                continue;
            }

            // Set position and velocity

            vec3 &n = r.normal;   // normal
            vec3 &v = s.state.v;  // velocity
            vec3 &x = s.state.x;  // position

            v = v - (v * n) * n;                          // Set normal velocity to zero
            x = x - ((x - r.centre) * n - s.radius) * n;  // Set normal position one sphere radius from rectangle
        }
    }

    // Collect start state

    State *yStart = new State[spheres.size()];
    State *yEnd = new State[spheres.size()];

    copyState(&spheres[0], yStart);  // copy sphere states into state vector 'yStart'

    // Integrate

    bool collisionAtEnd;
    Sphere *collisionSphere = NULL;
    Object *collisionObject = NULL;

    float actualDeltaT;

    integrate(yStart, yEnd, deltaT, collisionAtEnd, &collisionSphere, &collisionObject);

    if (!collisionAtEnd) {
        // no collisions in this integration step.  We're done.

        actualDeltaT = deltaT;
        copyState(yEnd, &spheres[0]);  // update sphere states and return

    } else {
        // a collision: Do binary seach until interval is at most
        // MIN_DELTA_T_FOR_COLLISIONS.
        //
        // Once your search is complete, 'yStart' should contain the state
        // at the start of the interval and 'yEnd' should contain the
        // state at the end of the interval.  Call integrate() as
        // necessary to get the end state from a given start state.
        //
        // Once the search is complete, 'actualDeltaT' should be the time
        // to the start of the interval.  actualDeltaT is in (0,deltaT)
        // and is the amount of time the simulation advances before the
        // collision is arrived at.
        //
        // Read the code above to see how it can be determined that no
        // collisions occur in an interval.

        actualDeltaT = 0;

        // [YOUR CODE HERE]

        //
        //
        //   F          F    F  F  T       T
        //   ^          ^    ^  ^  ^       ^
        //   -------------------------------
        //   t                  |  |       t+∆t
        //                      ^^^^
        //                       t*

        float left = 0.0;
        float right = deltaT;
        float tStar = 0.0;

        while (left <= right) {
            // cout << spheres[0].state.x.z - spheres[0].radius << endl;
            float newDeltaT = left + (right - left) / 2;

            integrate(yStart, yEnd, newDeltaT, collisionAtEnd, &collisionSphere, &collisionObject);

            if (right - left <= MIN_DELTA_T_FOR_COLLISIONS) {
                // found t*
                // update state
                tStar = newDeltaT;
                break;
            } else if (!collisionAtEnd) {
                // no collision at 'newDeltaT'
                left = newDeltaT;
            } else {
                // collision at 'newDeltaT'
                right = newDeltaT;
            }
        }

        actualDeltaT = deltaT + left;

        // cout << right - left << endl;
        // cout << tStar << endl;
        // cout << deltaT << endl;
        // cout << actualDeltaT << endl;

        // Set the sphere states to that at the START of the interval so
        // that collision has not yet occurred.  Since the objects DO NOT
        // MOVE during collision resolution, this ensures that the objects
        // are not in collision immediately after the collision is
        // resolved.  (In the contrary case, a new collision would be
        // immediately detected, causing the simulation to stop advancing.)

        copyState(yStart, &spheres[0]);

        // Resolve the collision

        resolveCollision(collisionSphere, collisionObject);

        // Debugging: report collision

#if 0
    bool otherIsSphere = (dynamic_cast<Sphere*>( collisionObject ) != NULL);
    cout << "collision s" << W2(collisionSphere - &spheres[0]) << "-";
    if (otherIsSphere) {
      Sphere *s = dynamic_cast<Sphere*>( collisionObject );
      cout << "s" << W2(s - &spheres[0])
	   << ", relative speed " << (collisionSphere->state.v - s->state.v) * (collisionSphere->state.x - s->state.x).normalize()
	   << ", relative position " << (collisionSphere->state.x - s->state.x) * (collisionSphere->state.x - s->state.x).normalize()
	   << endl;
    } else {
      Rectangle *r = dynamic_cast<Rectangle*>( collisionObject );
      cout << "r" << W2(r - &rectangles[0])
	   << ", relative speed " << collisionSphere->state.v * r->normal
	   << ", relative position " << (collisionSphere->state.x - r->centre) * r->normal << endl;
    }
#endif
    }

    // Clean up

    delete[] yStart;
    delete[] yEnd;

    return actualDeltaT;
}

// If there was a collision over time deltaT, use binary search to
// find the time of collision (to within MIN_DELTA_T_FOR_COLLISIONS)
// and set yEnd to the state just BEFORE that collision.
//

bool World::findCollisions(Sphere **collisionSphere, Object **collisionObject)

{
    int nSpheres = spheres.size();

    // Reset min distances on each sphere (used for drawing lines to closest object)

    for (int i = 0; i < nSpheres; i++) spheres[i].minDist = FLT_MAX;

    // Check for sphere/sphere collisions

    float minDist = FLT_MAX;

    for (int i = 0; i < nSpheres; i++)
        for (int j = 0; j < nSpheres; j++)
            if (i != j) {
                vec3 centreToCentre = spheres[j].state.x - spheres[i].state.x;
                float dist = centreToCentre.length() - spheres[i].radius - spheres[j].radius;

                float relativeVelocitySign = (spheres[j].state.v - spheres[i].state.v) * centreToCentre;

                if (relativeVelocitySign < 0 ) {  // < 0 if coming together, > 0 is moving apart

                    if (dist < minDist) {
                        minDist = dist;
                        *collisionSphere = &spheres[i];
                        *collisionObject = &spheres[j];
                    }

                    if (dist < spheres[i].minDist) {
                        spheres[i].minDist = dist;
                        spheres[i].contactPoint = spheres[i].state.x + 0.5 * centreToCentre;
                    }
                }
            }

    // Check for sphere/rectangle collisions
    //
    // However, do not check against rectangles that a sphere is constrained to remain in contact with.

    for (int i = 0; i < nSpheres; i++)
        for (int j = 0; j < rectangles.size(); j++)
            if (!spheres[i].constraintRectangles.exists(&rectangles[j])) {  // skip constraining rectangles

                vec3 contactPoint;
                float dist = spheres[i].distToRectangle(rectangles[j], &contactPoint);

                float relativeVelocitySign =
                    (((spheres[i].state.x - rectangles[j].centre) * rectangles[j].normal) * rectangles[j].normal) *
                    spheres[i].state.v;

                if (relativeVelocitySign < 0 ) {  // < 0 if coming together, > 0 is moving apart

                    if (dist < minDist) {
                        minDist = dist;
                        *collisionSphere = &spheres[i];
                        *collisionObject = &rectangles[j];
                    }

                    if (dist < spheres[i].minDist) {
                        spheres[i].minDist = dist;
                        spheres[i].contactPoint = contactPoint;
                    }
                }
            }

    return (minDist <= 0);
}

// Resolve a collision between a sphere and another object.  The other
// object can be a sphere or a rectangle.  Rectangles are immovable.

void World::resolveCollision(Sphere *sphere, Object *otherObject)

{
    // Extract the collision object as a sphere or rectangle

    bool otherIsSphere = (dynamic_cast<Sphere *>(otherObject) != NULL);

    if (otherIsSphere) {
        // SPHERE/SPHERE COLLISION

        Sphere *sphere2 = dynamic_cast<Sphere *>(otherObject);

        // [YOUR CODE HERE: REPLACE THE CODE BELOW]

        // Find a normal to the tangent plane between the spheres

        vec3 n = (sphere->state.x-sphere->contactPoint-(sphere2->state.x-sphere2->contactPoint)).normalize();

        // Find the velocity in the normal direction after the collisions

        float v1b = sphere->state.v * n;   // sphere 1 velocity before in normal direction
        float v2b = sphere2->state.v * n;  // sphere 2 velocity before in normal direction

        float m1 = sphere->mass();  // sphere 1 mass
        float m2 = sphere2->mass();  // sphere 2 mass

        float v1a = 1 / m1 * (m1*(COEFF_OF_RESTITUTION-1)*v1b);  // sphere 1 velocity AFTER in normal direction
        float v2a = 1 / m2 * -(m2* (COEFF_OF_RESTITUTION - 1) * v2b);   // sphere 2 velocity AFTER in normal direction

        // Update sphere velocities in their respective 'state.v'

        sphere->state.v = sphere->state.v + (v1a - v1b) * n;  // sphere 1 velocity AFTER
        sphere2->state.v = sphere2->state.v + (v2a - v2b) * n;                   // sphere 2 velocity AFTER

        // [END OF YOUR CODE ABOVE]

    } else {
        // SPHERE/RECTANGLE COLLISION

        Rectangle *rectangle = dynamic_cast<Rectangle *>(otherObject);

        // [YOUR CODE HERE: REPLACE THE CODE BELOW]

        // Find tangent plane normal
        //
        // Be careful to consider when the sphere hits the edge or corner
        // of the rectangle, as the plane in these cases is NOT the plane
        // of the rectangle.

        // vec3 n = rectangle->normal;  // for now
        vec3 n = (sphere->state.x - sphere->contactPoint).normalize();

        // Find the velocity in the normal direction after the collisions
        // COEFF_OF_RESTITUTION

        float v1b = sphere->state.v * n;  // sphere velocity before in normal direction
        float v2b = 0.0;                  // rectangle velocity before in normal direction

        float m1 = sphere->mass();  // sphere mass

        float m2 = rectangle->mass();  // rectangle mass.  NOTE THAT THIS MASS IS VERY
                                       // LARGE AND CAN BE USED AS IF THE RECTANGLE IS
                                       // A MOVING OBJECT.  DO THIS!  See rectangle.h

        // float v1bPerp = sphere->state.v.y;
        // float v1bPar = sphere->state.v.x;
        // float v1a = (COEFF_OF_RESTITUTION * v1bPerp) + v1bPar;  // sphere velocity AFTER in normal direction
        // float v1a = v1b + COEFF_OF_RESTITUTION * 1 / (m1) * (v1b - v2b);
        // float v1a = (m2 * (0 - v2b)) / m1 + v1b;
        float v1a = COEFF_OF_RESTITUTION * (v1b - v2b);

        // Update state of sphere velocity only.  Do not change velocity of rectangle.

        // sphere->state.v = v1a * n;  // sphere velocity AFTER
        sphere->state.v = sphere->state.v + (v1a - v1b) * n;  // sphere velocity AFTER

        // [END OF YOUR CODE ABOVE]

        // Check whether a sphere/rectangle collision results in a tiny
        // sphere velocity normal to the plane such that the sphere will
        // remain close to the plane in the time step.  In this case,
        // add a constraint that the sphere remains on the plane.
        //
        // Note that a sphere could be constrained to multiple planes.

        float distToPlane = (sphere->state.x - rectangle->centre) * rectangle->normal - sphere->radius;

        if (fabs(distToPlane) < MIN_NORMAL_DISTANCE && fabs(v1a) < MIN_NORMAL_SPEED) {
            sphere->constraintRectangles.add(rectangle);
#if 0
      cout << "Added   s" << W2(sphere - &spheres[0]) << "-r" << W2(rectangle - &rectangles[0]) << " constraint" << endl;
#endif
        }
    }
}

// Draw the world

void World::draw(mat4 WCS_to_VCS, mat4 VCS_to_CCS, vec3 &lightDir)

{
    vec3 lightRed(0.984, 0.322, 0.220);  // free sphere
    vec3 greenish(0.105, 0.700, 0.305);  // sphere constrained to rectangle

    vec3 cerulean(0.608, 0.769, 0.886);    // pit sides and ground
    vec3 deeperCerulean = 0.8 * cerulean;  // pit bottom

    // Draw spheres

    for (int i = 0; i < spheres.size(); i++)
        spheres[i].draw(WCS_to_VCS, VCS_to_CCS, lightDir,
                        (spheres[i].constraintRectangles.size() == 0 ? lightRed : greenish));

    // Draw rectangles

    for (int i = 0; i < rectangles.size(); i++)
        rectangles[i].draw(WCS_to_VCS, VCS_to_CCS, lightDir, (i > 0 ? cerulean : deeperCerulean));

    // Draw a line between each sphere and its closest point on another object (for debugging)

    if (showClosest) {
        vec3 *segments = new vec3[2 * spheres.size()];
        int n = 0;

        for (int i = 0; i < spheres.size(); i++)
            if (spheres[i].minDist != FLT_MAX) {
                segments[n++] = spheres[i].state.x;
                segments[n++] = spheres[i].contactPoint;
            }

        mat4 MVP = VCS_to_CCS * WCS_to_VCS;
        segs->drawSegs(GL_LINES, segments, vec3(0.75, 0.9, 0.1), NULL, n, WCS_to_VCS, MVP, lightDir);

        // Clean up

        delete[] segments;
    }
}
