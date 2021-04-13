#pragma once

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

#include "defines.h"

#include "macros.h"

//~ #ifndef TYPES
    #include "custom_types.h"
    //~ #define TYPES 1 
//~ #endif

short checkLineW( VECTOR * pointA, VECTOR * pointB, MESH * mesh );

short checkLineS( VECTOR * pointA, VECTOR * pointB, MESH * mesh );

VECTOR getIntCollision(BODY one, BODY two);

VECTOR getExtCollision(BODY one, BODY two);

void   ResolveCollision( BODY * one, BODY * two );

VECTOR angularMom(BODY body);

void applyAcceleration(BODY * actor);
