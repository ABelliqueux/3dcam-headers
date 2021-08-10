#include "../include/sound.h"
#include "../include/space.h"

void setSPUsettings(SpuCommonAttr * spuSettings){
    // Set master & CD volume to max
    spuSettings->mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX);
    spuSettings->mvol.left  = 0x6000;
    spuSettings->mvol.right = 0x6000;
    spuSettings->cd.volume.left = 0x6000;
    spuSettings->cd.volume.right = 0x6000;
    // Enable CD input ON
    spuSettings->cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(spuSettings);
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
}

int resetXAsettings(void){
    // Reset parameters
    u_char param = CdlModeSpeed;
    // Set CD mode
    return CdControlB(CdlSetmode, &param, 0);
}
int prepareXAplayback(CdlFILTER * filter, char * channel){
    u_char param = CdlModeSpeed|CdlModeRT|CdlModeSF|CdlModeSize1;
    // Set the parameters above
    CdControlB(CdlSetmode, &param, 0);
    // Pause at current pos
    CdControlF(CdlPause,0);
    // Set filter 
    // Use file 1, channel 0
    filter->file = 1;
    filter->chan = *channel;
    return CdControlF(CdlSetfilter, (u_char *)filter);
}
void loadXAfile( char * XAfile, XA_TRACK * XATrack){
    CdlFILE XAPos;
    CdSearchFile( &XAPos, XAfile);
    XATrack[0].start = CdPosToInt(&XAPos.pos);
    XATrack[0].end = XATrack[0].start + (XAPos.size/CD_SECTOR_SIZE) - 1;
}

u_char startXAPlayback(int * sectorPos, CdlLOC * loc){
    // Convert sector number to CD position in min/second/frame and set CdlLOC accordingly.
    CdIntToPos( *sectorPos, loc);
    // Send CDROM read command
    CdControlF(CdlReadS, (u_char *)loc);
    return 1;
}
int stopXAPlayback(void){
    // stop CD
    return CdControlF(CdlStop,0);
}
int setXAchannel(CdlFILTER * filter, char * channel){
    filter->chan = *channel;
    // Set filter
    return CdControlF(CdlSetfilter, (u_char *)filter);
}
int playXAtrack(int * CurPos, XA_TRACK * XATrack, CdlFILTER * filter, CdlLOC * loc, char * channel){
    static int gPlaying = 0;
    // Sound playback
    // Begin XA file playback
    if (gPlaying == 0 && *CurPos == XATrack[0].start){
        gPlaying = startXAPlayback(&XATrack[0].start, loc);
    }
    // When endPos is reached, set playing flag to 0
    if ((*CurPos += XA_SECTOR_OFFSET) >= XATrack[0].end){
        gPlaying = 0;
    }
    // If XA file end is reached, stop playback
    if ( gPlaying == 0 && *CurPos >= XATrack[0].end ){
        // Stop CD playback
        stopXAPlayback();
        // Optional
        //~ resetXAsettings();
        // Switch to next channel and start play back
        *channel = !*channel;
        setXAchannel(filter, channel);
        *CurPos = XATrack[0].start;
    }
}
