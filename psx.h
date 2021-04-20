#pragma once

#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <libcd.h>


#include "defines.h"
#include "custom_types.h"

// PSX setup

void init(DISPENV disp[2], DRAWENV draw[2], short db, MATRIX * cmat, CVECTOR * BG, VECTOR * BK );

void ScrRst(void);

void display(DISPENV * disp, DRAWENV * draw, u_long * otdisc, char * primbuff, char ** nextprim, char * db);

// Utils

void LvlPtrSet( LEVEL * curLevel, LEVEL * level );

int LoadLevel(const char*const LevelName, u_long * LoadAddress);

void SwitchLevel(const char*const LevelName,  u_long * LoadAddress, LEVEL * curLevel, LEVEL * loadLevel);

void LoadTexture(u_long * tim, TIM_IMAGE * tparam);
