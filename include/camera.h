#pragma once
#include <sys/types.h>
#include <libgte.h>

// Camera modes
#define ACTOR  0
#define ROTATE 1
#define FIXED  2
#define TRACK  3
#define FLYCAM 4
#define FOLLOW 5

typedef struct{
    int x, xv;                                 // x: current value += xv : new value 
    int y, yv;                                 // x,y,z, vx, vy, vz are in PSX units (ONE == 4096)
    int z, zv;
    int pan, panv;
    int tilt, tiltv;
    int rol;
    SVECTOR * pos;
    SVECTOR * rot;
    SVECTOR dvs;
    MATRIX * mat;
} CAMERA;

void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance);
void getCameraXZY(int * x, int * z, int * y, int actorX, int actorZ, int actorY, int angle, int angleX, int distance);
void getCameraZY( int * z, int * y, int actorZ, int actorY, int angleX, int distance);
void applyCamera(CAMERA * cam);
void setCameraPos(CAMERA * camera, SVECTOR * pos, SVECTOR * rot);

void setCameraMode(LEVEL * curLvl, CAMERA * camera, VECTOR * camAngleToAct, VECTOR * posToActor, VECTOR * angle, VECTOR * angleCam, short curCamAngle, int camMode, int * lerping);
