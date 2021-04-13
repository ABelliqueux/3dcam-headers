#pragma once

#include "camera.h"
#include "physics.h"
#include "defines.h"

// Drawing

void transformMesh(CAMERA * camera, MESH * meshes);

void drawPoly(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);

void drawBG(CAMANGLE * camPtr, char ** nextpri, u_long * otdisc, char * db);
