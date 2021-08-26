#include "../include/sound.h"
#include "../include/space.h"

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
}
u_long sendVAGtoSPU(unsigned int VAG_data_size, u_char *VAG_data){
    u_long transferred;
    SpuSetTransferMode(SpuTransByDMA);                              // DMA transfer; can do other processing during transfer
    transferred = SpuWrite (VAG_data + sizeof(VAGhdr), VAG_data_size);     // transfer VAG_data_size bytes from VAG_data  address to sound buffer
    SpuIsTransferCompleted (SPU_TRANSFER_WAIT);                     // Checks whether transfer is completed and waits for completion
    return transferred;
}
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
}
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
}
void setVAGvolume(SpuVoiceAttr * voiceAttributes, VAGsound * sound, int volume){
    voiceAttributes->mask= ( SPU_VOICE_VOLL | SPU_VOICE_VOLR );
    voiceAttributes->voice        = sound->spu_channel;
    // Range 0 - 3fff
    voiceAttributes->volume.left  = volume;
    voiceAttributes->volume.right = volume;
    SpuSetVoiceAttr(voiceAttributes);    
}
void playSFX(SpuVoiceAttr * voiceAttributes, VAGsound *  sound, int volume){
    // Set voice volume to sample volume
    setVAGvolume(voiceAttributes, sound, volume);
    // Play voice
    SpuSetKey(SpuOn, sound->spu_channel);
}
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
}
void getXAoffset(LEVEL * level){
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
}
