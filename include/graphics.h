#pragma once
#include "../include/camera.h"
#include "../include/physics.h"
#include "../include/defines.h"

// Drawing
void transformMesh(CAMERA * camera, MESH * meshes);
void drawPoly(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);
// Tri drawing
void set3VertexLerPos(MESH * mesh, long t);
void set3Prism(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, int i);
void set3Tex(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, long t, int i);
long interpolateTri(POLY_GT3 * poly, MESH * mesh, long t, long * Flag);
void drawTri(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);
//Quad drawing
void drawQuad(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);
void set4VertexLerPos(MESH * mesh, long t);
void set4Prism(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, int i);
void set4Tex(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, long t, int i);
long interpolateQuad(POLY_GT4 * poly4, MESH * mesh, long t, long * Flag);
//2D drawing
void drawBG(CAMANGLE * camPtr, char ** nextpri, u_long * otdisc, char * db);
