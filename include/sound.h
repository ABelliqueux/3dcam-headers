#pragma once
#include "../include/psx.h"
#include "../include/macros.h"
// XA
// Sector offset for XA data 4: simple speed, 8: double speed
#define XA_CHANNELS 8
#define XA_CDSPEED (XA_CHANNELS >> VSYNC) * ONE
#define XA_RATE 380
// Number of XA samples ( != # of XA files )
#define XA_TRACKS 2
// VAG
// Number of VAG files to load
#define VAG_NBR 8
#define MALLOC_MAX VAG_NBR            // Max number of time we can call SpuMalloc
// Custom struct to handle VAG files
//~ typedef struct VAGsound {
    //~ u_char * VAGfile;        // Pointer to VAG data address
    //~ u_long spu_channel;      // SPU voice to playback to
    //~ u_long spu_address;      // SPU address for memory freeing spu mem
    //~ } VAGsound;
//~ typedef struct VAGbank {
    //~ u_int index;
    //~ VAGsound samples[];
//~ } VAGbank;
// VAG header struct (see fileformat47.pdf, p.209)
typedef struct VAGhdr {                // All the values in this header must be big endian
        char id[4];                    // VAGp         4 bytes -> 1 char * 4
        unsigned int version;          // 4 bytes
        unsigned int reserved;         // 4 bytes
        unsigned int dataSize;         // (in bytes) 4 bytes
        unsigned int samplingFrequency;// 4 bytes
        char  reserved2[12];           // 12 bytes -> 1 char * 12
        char  name[16];                // 16 bytes -> 1 char * 16
        // Waveform data after that
} VAGhdr;
//~ // XA
//~ typedef struct XAsound {
    //~ u_int id;
    //~ u_int size;
    //~ u_char file, channel;
    //~ u_int start, end;
    //~ int cursor;
//~ } XAsound;

//~ typedef struct XAbank {
    //~ char * name;
    //~ u_int index;
    //~ int offset;
    //~ XAsound samples[];
//~ } XAbank;

// VAG playback
void initSnd(SpuCommonAttr * spuSettings, char * spu_malloc_rec);
u_long sendVAGtoSPU(unsigned int VAG_data_size, u_char *VAG_data);
void setVoiceAttr(SpuVoiceAttr * voiceAttributes, u_int pitch, long channel, u_long soundAddr );
u_long setSPUtransfer(SpuVoiceAttr * voiceAttributes, VAGsound * sound);
void playSFX(SpuVoiceAttr * voiceAttributes, VAGsound *  sound);
// XA playback
void XAsetup(void);
void getXAoffset(LEVEL * level);
void setXAsample(XAsound * sound, CdlFILTER * filter);
