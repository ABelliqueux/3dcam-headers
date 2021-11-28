#pragma once
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <defines.h>

int cliptest3(short * v1);
void worldToScreen( VECTOR * worldPos, VECTOR * screenPos );
void screenToWorld( VECTOR * screenPos, VECTOR * worldPos );
