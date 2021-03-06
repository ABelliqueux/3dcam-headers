#pragma once
#include <sys/types.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <defines.h>
#include <math.h>
#include <macros.h>
#include <custom_types.h>
#include "../thirdparty/nugget/common/syscalls/syscalls.h"
#define printf ramsyscall_printf

short checkLineW( VECTOR * pointA, VECTOR * pointB, MESH * mesh );
short checkLineS( VECTOR * pointA, VECTOR * pointB, MESH * mesh );
VECTOR getIntCollision(BODY one, BODY two);
VECTOR getExtCollision(BODY one, BODY two);
VECTOR checkBodyCol(BODY * one, BODY * two);
void applyAngMom(LEVEL curLvl );
void   ResolveCollision( BODY * one, BODY * two );
VECTOR angularMom(BODY body);
void applyAcceleration(BODY * actor, int dt);
u_int jump(BODY * actor, int dt);
void respawnMesh(LEVEL * level, MESH * mesh, VECTOR * rot, VECTOR * pos, NODE * node);
