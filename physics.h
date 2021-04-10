#pragma once

#include <sys/types.h>
//~ #include <stddef.h>
//~ #include <stdint.h>
#include <libgte.h>
#include <libgpu.h>

#include "defines.h"

#include "macros.h"

#include "coridor2.h"

short checkLineW( VECTOR * pointA, VECTOR * pointB, MESH * mesh );

short checkLineS( VECTOR * pointA, VECTOR * pointB, MESH * mesh );

VECTOR getIntCollision(BODY one, BODY two);

VECTOR getExtCollision(BODY one, BODY two);

void   ResolveCollision( BODY * one, BODY * two );

VECTOR angularMom(BODY body);

void applyAcceleration(BODY * actor);
