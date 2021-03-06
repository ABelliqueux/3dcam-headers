#pragma once
#include <camera.h>
#include <physics.h>
#include <defines.h>

// Drawing
void updateLight(void);
void transformMesh(CAMERA * camera, MESH * meshes);
void enlightMesh(LEVEL * curLvl, MESH * actorPtr, SVECTOR * lgtang);
void drawPoly(MESH * mesh, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw);
// Tri drawing
long drawTri(MESH * mesh, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw, int t, int i);
void set3VertexPos(MESH * mesh, long t, int atime);
void set3VertexLerPos(MESH * mesh, long t);
void set3Prism(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, char * db, int i, int t);
void set3Tex(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, long t, int i);
void interpolateTri(POLY_GT3 * poly, MESH * mesh, long t);
//Quad drawing
long drawQuad(MESH * mesh, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw, int t, int i);
void set4VertexPos(MESH * mesh, long t, int atime);
void set4VertexLerPos(MESH * mesh, long t);
void set4Prism(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, char * db, int i, int t);
void set4Tex(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, long t, int i);
int set4Subdiv(MESH * mesh, POLY_GT4 * poly4, u_long * ot, long t, int i, char ** nextpri);
void interpolateQuad(POLY_GT4 * poly4, MESH * mesh, long t);
//2D drawing
void drawBG(CAMANGLE * camPtr, char ** nextpri, u_long * otdisc, char * db);
// Rendering
void renderScene(LEVEL * curLvl, CAMERA * camera, int * camMode, char ** nextpri,  u_long * ot, u_long * otdisc,  char * db, DRAWENV * draw, short curCamAngle, int atime);
// Animation
void playAnim(MESH * mesh, VANIM * anim, int loop);
