#pragma once

#include <stddef.h>
#include <stdint.h>
#include <libgte.h>
#include "../include/macros.h"

// Precalculated arctan values
#include "../src/atan.c"

// fixed point math
int32_t dMul(int32_t a, int32_t b);
uint32_t lerpU(uint32_t start, uint32_t dest, unsigned pos);
int32_t lerpS(int32_t start, int32_t dest, unsigned pos);
int32_t lerpD(int32_t start, int32_t dest, int32_t pos);
long long lerpL(long long start, long long dest, long long pos);

// Sin/Cos Table
void generateTable(void);
int  ncos(u_int t);
int  nsin(u_int t);

// Atan table
long long patan(long x, long y);

// Sqrt
u_int psqrt(u_int n);

// Lerps
int lerp(int start, int end, int factor); // FIXME : not working as it should
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor); // FIXME 
long long easeIn(long long i);
int easeOut(int i);
//~ int easeInOut(int i, int div);
VECTOR getVectorTo(VECTOR actor, VECTOR target);

int32_t round( int32_t n);
