#pragma once
#include "../include/psx.h"

#define CD_SECTOR_SIZE 2048
// XA
// Sector offset for XA data 4: simple speed, 8: double speed
#define XA_SECTOR_OFFSET 4
// Number of XA files
#define XA_TRACKS 1
// Number of populated XA streams/channels in each XA file
#define INDEXES_IN_XA   1
#define TOTAL_TRACKS    (XA_TRACKS*INDEXES_IN_XA)

// XA track struc
typedef struct {
    int start;
    int end;
} XA_TRACK;

void setSPUsettings(SpuCommonAttr * spuSettings);
int prepareXAplayback(CdlFILTER * filter, char * channel);
int resetXAsettings(void);
void loadXAfile( char * XAfile, XA_TRACK * XATrack);
u_char startXAPlayback(int * sectorPos, CdlLOC * loc);
int stopXAPlayback(void);
int setXAchannel(CdlFILTER * filter, char * channel);
int playXAtrack(int * CurPos, XA_TRACK * XATrack, CdlFILTER * filter, CdlLOC * loc, char * channel);
