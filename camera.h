#pragma once

#include <sys/types.h>
#include <libgte.h>

typedef struct{
    
    int x, xv;                                 // x: current value += xv : new value 
    
    int y, yv;                                 // x,y,z, vx, vy, vz are in PSX units (ONE == 4096)
    
    int z, zv;
    
    int pan, panv;
    
    int tilt, tiltv;
    
    int rol;

    VECTOR pos;
    
    SVECTOR rot;
    
    SVECTOR dvs;

    MATRIX mat;

} CAMERA;

void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance);

void getCameraXZY(int * x, int * z, int * y, int actorX, int actorZ, int actorY, int angle, int angleX, int distance);

void getCameraZY( int * z, int * y, int actorZ, int actorY, int angleX, int distance);

void applyCamera(CAMERA * cam);

void setCameraPos(CAMERA * camera, VECTOR pos, SVECTOR rot);
