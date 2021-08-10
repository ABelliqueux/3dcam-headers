#pragma once
#include <sys/types.h>
#include <stdio.h>
#include <libgte.h>
#include <libetc.h>
#include <libgpu.h>
#include <libapi.h>
#include <libcd.h>
#include <libspu.h>
#include <string.h>
#include <inline_n.h>
#include <gtemac.h>
#include "../include/defines.h"
#include "../custom_types.h"

// PSX setup
void setDCLightEnv(MATRIX * curLevelCMat, MATRIX * curLevelLgtMat, SVECTOR * curLevelLgtAng);
void setLightEnv(DRAWENV draw[2], CVECTOR * BGc, VECTOR * BKc);
void init(DISPENV disp[2], DRAWENV draw[2], short db, CVECTOR * BG, VECTOR * BK, VECTOR * FC );
void ScrRst(void);
void display(DISPENV * disp, DRAWENV * draw, u_long * otdisc, char * primbuff, char ** nextprim, char * db);

// Utils
void LvlPtrSet( LEVEL * curLevel, LEVEL * level );
int LoadLevelCD(const char*const LevelName, u_long * LoadAddress);
void SwitchLevel( LEVEL * curLevel, LEVEL * loadLevel);
void LoadTexture(u_long * tim, TIM_IMAGE * tparam);
