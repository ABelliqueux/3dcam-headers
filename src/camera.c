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
    //~ VECTOR vec;                                         // Vector that holds the output values of the following instructions
    RotMatrix_gte(dc_camRot, dc_camMat);                // Convert rotation angle in psx units (360° == 4096) to rotation matrix)
    gte_ApplyMatrix(dc_camMat, dc_camPos, dc_wrklvector);          // Multiply matrix by vector pos and output to vec
    TransMatrix(dc_camMat, dc_wrklvector);                       // Apply transform vector
    gte_SetRotMatrix(dc_camMat);                            // Set Rotation matrix
    gte_SetTransMatrix(dc_camMat);                          // Set Transform matrix
};
void setCameraPos( CAMERA * camera, SVECTOR * pos, SVECTOR * rot ) {
    copyVector(camera->pos, pos);
    copyVector(camera->rot, rot);
};
