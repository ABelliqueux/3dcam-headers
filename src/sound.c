#include <sound.h>
#include <space.h>

// VAG playback
void initSnd(SpuCommonAttr * spuSettings, char * spu_malloc_rec, u_int mallocMax){
    SpuInitMalloc(mallocMax, spu_malloc_rec);                      // Maximum number of blocks, mem. management table address.
    spuSettings->mask = (SPU_COMMON_MVOLL | SPU_COMMON_MVOLR | SPU_COMMON_CDVOLL | SPU_COMMON_CDVOLR | SPU_COMMON_CDMIX );  // Mask which attributes to set
    spuSettings->mvol.left  = MVOL_L;                           // Master volume left
    spuSettings->mvol.right = MVOL_R;                           // see libref47.pdf, p.1058
    spuSettings->cd.volume.left = CDVOL_L;
    spuSettings->cd.volume.right = CDVOL_R;
    // Enable CD input ON
    spuSettings->cd.mix = SPU_ON;
    // Apply settings
    SpuSetCommonAttr(spuSettings);                           
    // Set transfer mode 
    SpuSetTransferMode(SPU_TRANSFER_BY_DMA);
    SpuSetIRQ(SPU_OFF);
    // Mute all voices
    SpuSetKey(SpuOff, SPU_ALLCH);
};
u_long sendVAGtoSPU(unsigned int VAG_data_size, u_char *VAG_data){
    u_long transferred;
    SpuSetTransferMode(SpuTransByDMA);                              // DMA transfer; can do other processing during transfer
    transferred = SpuWrite (VAG_data + sizeof(VAGhdr), VAG_data_size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted (SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion
    return transferred;
};
void setVoiceAttr(SpuVoiceAttr * voiceAttributes, u_int pitch, long channel, u_long soundAddr ){
    voiceAttributes->mask=                                   //~ Attributes (bit string, 1 bit per attribute)
    (
      SPU_VOICE_VOLL |
      SPU_VOICE_VOLR |
      SPU_VOICE_PITCH |
      SPU_VOICE_WDSA |
      SPU_VOICE_ADSR_AMODE |
      SPU_VOICE_ADSR_SMODE |
      SPU_VOICE_ADSR_RMODE |
      SPU_VOICE_ADSR_AR |
      SPU_VOICE_ADSR_DR |
      SPU_VOICE_ADSR_SR |
      SPU_VOICE_ADSR_RR |
      SPU_VOICE_ADSR_SL
    );
    voiceAttributes->voice        = channel;                 //~ Voice (low 24 bits are a bit string, 1 bit per voice )
    voiceAttributes->volume.left  = 0x0;                  //~ Volume 
    voiceAttributes->volume.right = 0x0;                  //~ Volume
    voiceAttributes->pitch        = pitch;                   //~ Interval (set pitch)
    voiceAttributes->addr         = soundAddr;               //~ Waveform data start address
    voiceAttributes->a_mode       = SPU_VOICE_LINEARIncN;    //~ Attack rate mode  = Linear Increase - see libref47.pdf p.1091
    voiceAttributes->s_mode       = SPU_VOICE_LINEARIncN;    //~ Sustain rate mode = Linear Increase
    voiceAttributes->r_mode       = SPU_VOICE_LINEARDecN;    //~ Release rate mode = Linear Decrease
    voiceAttributes->ar           = 0x0;                     //~ Attack rate
    voiceAttributes->dr           = 0x0;                     //~ Decay rate
    voiceAttributes->rr           = 0x0;                     //~ Release rate
    voiceAttributes->sr           = 0x0;                     //~ Sustain rate
    voiceAttributes->sl           = 0xf;                     //~ Sustain level
    SpuSetVoiceAttr(voiceAttributes);                      // set attributes
};
u_long setSPUtransfer(SpuVoiceAttr * voiceAttributes, VAGsound * sound){
    // Return spu_address
    u_long transferred, spu_address;
    u_int pitch;
    const VAGhdr * VAGheader = (VAGhdr *) sound->VAGfile;
    pitch = (SWAP_ENDIAN32(VAGheader->samplingFrequency) << 12) / 44100L; 
    spu_address = SpuMalloc(SWAP_ENDIAN32(VAGheader->dataSize));                // Allocate an area of dataSize bytes in the sound buffer. 
    SpuSetTransferStartAddr(spu_address);                                       // Sets a starting address in the sound buffer
    transferred = sendVAGtoSPU(SWAP_ENDIAN32(VAGheader->dataSize), sound->VAGfile);
    setVoiceAttr(voiceAttributes, pitch, sound->spu_channel, spu_address); 
    return spu_address;
};
void setVAGvolume(SpuVoiceAttr * voiceAttributes, VAGsound * sound, int volumeL,int volumeR ){
    voiceAttributes->mask= ( SPU_VOICE_VOLL | SPU_VOICE_VOLR );
    voiceAttributes->voice        = sound->spu_channel;
    // Range 0 - 3fff
    voiceAttributes->volume.left  = volumeL;
    voiceAttributes->volume.right = volumeR;
    SpuSetVoiceAttr(voiceAttributes);    
};
void setLvlVAG(LEVEL * level, SpuCommonAttr * spuSettings, SpuVoiceAttr * voiceAttributes, char spu_malloc_rec[]){
    if (level->VAG != 0){
        // Free SPU mem
        for (u_short vag = 0; vag < level->VAG->index; vag++ ){
            if(level->VAG->samples[vag].spu_address != 0){
                SpuFree(level->VAG->samples[vag].spu_address);
            }
        }
        // Init sound settings
        initSnd(spuSettings, spu_malloc_rec, level->VAG->index );
        for (u_short vag = 0; vag < level->VAG->index; vag++ ){
            level->VAG->samples[vag].spu_address = setSPUtransfer(voiceAttributes, &level->VAG->samples[vag]);
        }
    }
};
void playSFX(SpuVoiceAttr * voiceAttributes, VAGsound *  sound, int volumeL, int volumeR ){
    // Set voice volume to sample volume
    setVAGvolume(voiceAttributes, sound, volumeL, volumeR);
    // Play voice
    SpuSetKey(SpuOn, sound->spu_channel);
};
void setSFXdist(LEVEL * level, CAMERA * camera, int camMode ){
    VECTOR sndPos2D = {0};
    if (level->levelSounds != 0){
        for(int snd = 0; snd < level->levelSounds->index; snd++){
            u_int r;
            // If parent is actor, 
            if (level->levelSounds->sounds[snd]->parent == level->actorPtr && camMode <= 1){
                r = CAM_DIST_TO_ACT;
            // update sound location if sound has a parent and it's not actor
            } else if ( level->levelSounds->sounds[snd]->parent != 0){
                VECTOR dist;
                copyVector(&level->levelSounds->sounds[snd]->location, &level->levelSounds->sounds[snd]->parent->pos);
                // Get distance between sound source and camera
                dist.vx = -camera->pos->vx - level->levelSounds->sounds[snd]->location.vx;
                dist.vz = -camera->pos->vz - level->levelSounds->sounds[snd]->location.vz;
                r = psqrt((dist.vx * dist.vx) + (dist.vz * dist.vz));
                // Get snd screen coordinates
                // Range -1024 0 == screen left, 0 +1024 == screen right 
                worldToScreen(&level->levelSounds->sounds[snd]->location, &sndPos2D);
            } 
            // Find volume base on dist
            u_int volumeBase = (level->levelSounds->sounds[snd]->volume_max/r) * SND_NMALIZED > SND_MAX_VOL ? SND_MAX_VOL : 
                               (level->levelSounds->sounds[snd]->volume_max/r) * SND_NMALIZED < 0 ? 0 :
                               (level->levelSounds->sounds[snd]->volume_max/r) * SND_NMALIZED;
            // Avoid value of 0
            sndPos2D.vx = sndPos2D.vx == 0 || sndPos2D.vx == -0 ? 1 : sndPos2D.vx;
            level->levelSounds->sounds[snd]->volumeL = volumeBase / ( (sndPos2D.vx >  SND_DZ ? (    sndPos2D.vx - SND_DZ   >> 7) + 1 : 1) ); 
            level->levelSounds->sounds[snd]->volumeR = volumeBase / ( (sndPos2D.vx < -SND_DZ ? ( ( -sndPos2D.vx - SND_DZ ) >> 7) + 1 : 1) );
        }
    }
};
void XAsetup(void){   
    u_char param[4];
    // ORing the parameters we need to set ; drive speed,  ADPCM play, Subheader filter, sector size
    // If using CdlModeSpeed(Double speed), you need to load an XA file that has 8 channels.
    // In single speed, a 4 channels XA is to be used.
    param[0] = CdlModeSpeed|CdlModeRT|CdlModeSF|CdlModeSize1;
    // Issue primitive command to CD-ROM system (Blocking-type)
    // Set the parameters above
    CdControlB(CdlSetmode, param, 0);
    // Pause at current pos
    CdControlF(CdlPause,0);
};
void getXAoffset(LEVEL * level){
        // TODO : Only works for first XA file
        CdlFILE XAPos = {0};
        // Load XA file
        //~ CdSearchFile(&XAPos, level->XA->name);
        CdSearchFile(&XAPos, level->XA->banks[0]->name);
        // Set cd head to start of file
        //~ level->XA->offset = CdPosToInt(&XAPos.pos);
        level->XA->banks[0]->offset = CdPosToInt(&XAPos.pos);
};
void setXAsample(XAsound * sound, CdlFILTER * filter){
    filter->chan = sound->channel;
    filter->file = sound->file;
    // Set filter
    CdControlF(CdlSetfilter, (u_char *)filter);
    // Reset sample's cursor
    sound->cursor = 0;
};
void setLvlXA(LEVEL * level, int sample){
    if(sample >= 0){
        // CD filter
        CdlFILTER filter;
        // File position in m/s/f
        CdlLOC  loc;
        if (level->XA != 0){
            // Change XA track
            XAsetup();
            //~ sample = !sample;
            level->XA->banks[0]->samples[sample].cursor = -1;
            getXAoffset(level);
            setXAsample(&level->XA->banks[0]->samples[sample], &filter);
            CdIntToPos(level->XA->banks[0]->samples[sample].start + level->XA->banks[0]->offset , &loc);
            // Send CDROM read command
            CdControlF(CdlReadS, (u_char *)&loc);
        }
    }
};
void XAplayback(LEVEL * level, int sample, long dt){
    if (sample != -1 ){
        // CD filter
        CdlFILTER filter;
        // File position in m/s/f
        CdlLOC  loc;
        // Begin XA file playback...
        // if sample's cursor is 0
        if (level->XA->banks[0]->samples[sample].cursor == 0){
            // Convert sector number to CD position in min/second/frame and set CdlLOC accordingly.
            CdIntToPos(level->XA->banks[0]->samples[sample].start + level->XA->banks[0]->offset , &loc);
            // Send CDROM read command
            CdControlF(CdlReadS, (u_char *)&loc);
            //~ *XATime = VSync(-1);
            // Set playing flag
        }
        // if sample's cursor is close to sample's end position, stop playback
        // XA playback has fixed rate
        if ((level->XA->banks[0]->samples[sample].cursor += XA_CDSPEED / ((XA_RATE/(dt+1)+1)) ) >= (level->XA->banks[0]->samples[sample].end - level->XA->banks[0]->samples[sample].start) * ONE  ){
            //~ CdControlF(CdlStop,0);
            level->XA->banks[0]->samples[sample].cursor = -1;
            setXAsample(&level->XA->banks[0]->samples[sample], &filter);
        }
    }
};
