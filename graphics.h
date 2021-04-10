#pragma once

//~ #include <sys/types.h>
//~ #include <libgte.h>
//~ #include <libgpu.h>

//~ #include "psx.h"
#include "camera.h"
#include "physics.h"
#include "defines.h"

//~ int camMode = 0;

//~ #define SCREENXRES 320

//~ #define SCREENYRES 240

//~ #define OT2LEN	    8	                   

//~ #define OTLEN	    256	

// Drawing

void transformMesh(CAMERA * camera, MESH * meshes);

void drawPoly(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);

void drawBG(CAMANGLE * camPtr, char ** nextpri, u_long * otdisc, char * db);
