#pragma once
#include "../custom_types.h"
#include "../include/defines.h"

extern LEVEL level0;
extern CVECTOR level0_BGc;
extern VECTOR level0_BKc;
extern CAMPOS level0_camPos_Camera;
extern CAMPATH level0_camPath;
extern MATRIX level0_lgtmat;
extern MATRIX level0_cmat;
extern SVECTOR modelCube_mesh[];
extern SVECTOR level0_modelCube_normal[];
extern CVECTOR level0_modelCube_color[];
extern PRIM level0_modelCube_index[];
extern BODY  level0_modelCube_body;
extern TMESH level0_modelCube;
extern MESH level0_meshCube;
extern SVECTOR modelPlane_mesh[];
extern SVECTOR level0_modelPlane_normal[];
extern CVECTOR level0_modelPlane_color[];
extern PRIM level0_modelPlane_index[];
extern BODY  level0_modelPlane_body;
extern TMESH level0_modelPlane;
extern MESH level0_meshPlane;
extern MESH * level0_meshes[2];
extern int level0_meshes_length;
extern CAMANGLE level0_camAngle_Camera;
extern CAMANGLE * level0_camAngles[0];
extern SIBLINGS level0_nodePlane_siblings;
extern CHILDREN level0_nodePlane_objects;
extern CHILDREN level0_nodePlane_rigidbodies;
extern NODE level0_nodePlane;
extern MESH * level0_actorPtr;
extern MESH * level0_levelPtr;
extern MESH * level0_propPtr;
extern CAMANGLE * level0_camPtr;
extern NODE * level0_curNode;
extern NODE level0_nodePlane;
extern VAGbank VAGBank0;
extern XAbank XABank0;
