#pragma once
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include "../include/defines.h"
#include "../include/macros.h"
#include "../custom_types.h"

short checkLineW( VECTOR * pointA, VECTOR * pointB, MESH * mesh );
short checkLineS( VECTOR * pointA, VECTOR * pointB, MESH * mesh );
VECTOR getIntCollision(BODY one, BODY two);
VECTOR getExtCollision(BODY one, BODY two);
void   ResolveCollision( BODY * one, BODY * two );
VECTOR angularMom(BODY body);
void applyAcceleration(BODY * actor);
