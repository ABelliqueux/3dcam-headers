#include "level0.h"

CVECTOR level0_BGc = { 0, 218, 216, 0 };

CAMPOS level0_camPos_Camera = {
    { -486,347,423 },
    { 301,531,0 }
};

CAMPATH level0_camPath = {
    0,
    0,
    0
};

MATRIX level0_lgtmat = {
    -2319,  3254,   -894,
    0,0,0,
    0,0,0,

    };

MATRIX level0_cmat = {
    4096,0,0,
    4096,0,0,
    4096,0,0
    };

SVECTOR level0_modelCube_mesh[] = {
    { 65,65,65 },
    { 65,65,-65 },
    { -65,65,-65 },
    { -65,65,65 },
    { 65,-65,65 },
    { 65,-65,-65 },
    { -65,-65,-65 },
    { -65,-65,65 }
};

SVECTOR level0_modelCube_normal[] = {
    -2365,-2365,-2365, 0,
    -2365,-2365,2365, 0,
    2365,-2365,2365, 0,
    2365,-2365,-2365, 0,
    -2365,2365,-2365, 0,
    -2365,2365,2365, 0,
    2365,2365,2365, 0,
    2365,2365,-2365, 0
};

CVECTOR level0_modelCube_color[] = {
    255,0,172, 0,
    16,0,255, 0,
    33,255,0, 0,
    255,208,0, 0,
    229,30,196, 0,
    255,190,28, 0,
    255,223,81, 0,
    16,255,1, 0,
    255,245,91, 0,
    255,30,226, 0,
    16,255,1, 0,
    29,17,255, 0,
    16,0,255, 0,
    253,0,10, 0,
    255,0,8, 0,
    33,255,0, 0,
    33,255,0, 0,
    255,142,3, 0,
    255,132,3, 0,
    255,208,0, 0,
    37,255,0, 0,
    45,244,22, 0,
    255,43,33, 0,
    255,193,77, 0
};

PRIM level0_modelCube_index[] = {
    0,1,2,3,8,
    4,7,6,5,8,
    0,4,5,1,8,
    1,5,6,2,8,
    2,6,7,3,8,
    4,0,3,7,8
};

BODY level0_modelCube_body = {
    {0, 0, 0, 0},
    0,-130,23, 0,
    0,0,0, 0,
    1,
    ONE/1,
    -65,-65,-65, 0,
    65,65,65, 0,
    0,
    };

TMESH level0_modelCube = {
    level0_modelCube_mesh,
    level0_modelCube_normal,
    0,
    level0_modelCube_color, 
    6
};

MESH level0_meshCube = {
    &level0_modelCube,
    level0_modelCube_index,
    0,
    0,
    {0},
    {0,-130,23, 0},
    {0,0,0},
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    &level0_modelCube_body,
    0,
    0,
    0
};

SVECTOR level0_modelPlane_mesh[] = {
    { -260,0,-260 },
    { 260,0,-260 },
    { -260,0,260 },
    { 260,0,260 }
};

SVECTOR level0_modelPlane_normal[] = {
    0,4096,0, 0,
    0,4096,0, 0,
    0,4096,0, 0,
    0,4096,0, 0
};

CVECTOR level0_modelPlane_color[] = {
    255,37,10, 0,
    255,37,10, 0,
    255,37,10, 0,
    255,37,10, 0,
};

PRIM level0_modelPlane_index[] = {
    0,1,3,2,8
};

BODY level0_modelPlane_body = {
    {0, 0, 0, 0},
    0,0,0, 0,
    0,0,0, 0,
    1,
    ONE/1,
    -260,0,-260, 0,
    260,0,260, 0,
    0,
    };

TMESH level0_modelPlane = {
    level0_modelPlane_mesh,
    level0_modelPlane_normal,
    0,
    level0_modelPlane_color, 
    1
};

MESH level0_meshPlane = {
    &level0_modelPlane,
    level0_modelPlane_index,
    0,
    0,
    {0},
    {0,0,0, 0},
    {0,0,0},
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    &level0_modelPlane_body,
    0,
    0,
    0
};

MESH * level0_meshes[2] = {
    &level0_meshCube,
    &level0_meshPlane
}; 

int level0_meshes_length = 2;

CAMANGLE level0_camAngle_Camera = {
    &level0_camPos_Camera,
    0,
     0,
     { 0 },
     { 0 },
     0,
     0
};

CAMANGLE * level0_camAngles[0] = {
};

SIBLINGS level0_nodePlane_siblings = {
    0,
    {
        0
    }
};

CHILDREN level0_nodePlane_objects = {
    0,
    {
        0
    }
};

CHILDREN level0_nodePlane_rigidbodies = {
    1,
    {
        &level0_meshCube
    }
};

NODE level0_nodePlane = {
    &level0_meshPlane,
    &level0_nodePlane_siblings,
    &level0_nodePlane_objects,
    &level0_nodePlane_rigidbodies
};

MESH * level0_actorPtr = &level0_meshCube;
MESH * level0_levelPtr = &level0_meshPlane;
MESH * level0_propPtr  = &level0_meshCube;

CAMANGLE * level0_camPtr =  &level0_camAngle_Camera;

NODE * level0_curNode =  &level0_nodePlane;

LEVEL level0 = {
    &level0_BGc,
    &level0_cmat,
    &level0_lgtmat,
    (MESH **)&level0_meshes,
    &level0_meshes_length,
    &level0_meshCube,
    &level0_meshPlane,
    &level0_meshCube,
    &level0_camAngle_Camera,
    &level0_camPath,
    (CAMANGLE **)&level0_camAngles,
    &level0_nodePlane,
};
