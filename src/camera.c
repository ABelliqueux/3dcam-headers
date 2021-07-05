#include "../include/psx.h"
#include "../include/camera.h"
#include "../include/math.h"

void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance) {
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj    
    *x = (actorX << 12) + (distance * nsin(angle));
    *z = (actorZ << 12) - (distance * ncos(angle));
};
void getCameraXZY(int * x, int * z, int * y, int actorX, int actorZ, int actorY, int angle, int angleX, int distance) {
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj    
    *x = (actorX << 12) + (distance * nsin(angle));
    *z = (actorZ << 12) - (distance * ( ( ncos(angle) * ncos(angleX) ) >> 12 ) );
    *y = (actorY << 12) - (distance * nsin(angleX));
};
void getCameraZY( int * z, int * y, int actorZ, int actorY, int angleX, int distance) {
    *z = (actorZ << 12) - (distance * ncos(angleX));
    *y = (actorY << 12) - (distance * nsin(angleX));
};
// @Will : you might want to use sin/cos to move the camera in a circle but you could do that by moving it along it’s tangent and then clamping the distance
void applyCamera( CAMERA * cam ) {
    VECTOR vec;                                         // Vector that holds the output values of the following instructions
    RotMatrix_gte(&cam->rot, &cam->mat);                // Convert rotation angle in psx units (360° == 4096) to rotation matrix)
    gte_ApplyMatrix(&cam->mat, &cam->pos, &vec);          // Multiply matrix by vector pos and output to vec
    TransMatrix(&cam->mat, &vec);                       // Apply transform vector
    gte_SetRotMatrix(&cam->mat);                            // Set Rotation matrix
    gte_SetTransMatrix(&cam->mat);                          // Set Transform matrix
};
void setCameraPos( CAMERA * camera, SVECTOR pos, SVECTOR rot ) {
    camera->pos =  pos;
    camera->rot =  rot;
};
