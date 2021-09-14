#define VMODE       0 // 0 == NTSC, 1 == PAL
#define VSYNC       1
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX     SCREENXRES/2
#define CENTERY     SCREENYRES/2
#define FOV CENTERX
#define FOG_COLOR {128, 128, 128} // Default to neutral grey
#define CLEAR_COLOR_R 0
#define CLEAR_COLOR_G 0
#define CLEAR_COLOR_B 0

#define CAM_DIST_TO_ACT 200
#define CAM_DIST_TO_GND 100

// Sound
// Sound engine
#define SND_DIST_MIN 200
#define SND_DIST_MAX 800
#define SND_ATTENUATION 6144
#define SND_MAX_VOL 16383
#define SND_RANGE (SND_DIST_MAX - SND_DIST_MIN)
#define SND_NMALIZED (SND_RANGE * SND_ATTENUATION / SND_MAX_VOL)
#define SND_DZ 32
// SPU channels
#define SPU_00CH (0x1L<< 0)
#define SPU_01CH (0x1L<< 1)
#define SPU_02CH (0x1L<< 2)
#define SPU_03CH (0x1L<< 3)
#define SPU_04CH (0x1L<< 4)
#define SPU_05CH (0x1L<< 5)
#define SPU_06CH (0x1L<< 6)
#define SPU_07CH (0x1L<< 7)
#define SPU_08CH (0x1L<< 8)
#define SPU_09CH (0x1L<< 9)
#define SPU_10CH (0x1L<<10)
#define SPU_11CH (0x1L<<11)
#define SPU_12CH (0x1L<<12)
#define SPU_13CH (0x1L<<13)
#define SPU_14CH (0x1L<<14)
#define SPU_15CH (0x1L<<15)
#define SPU_16CH (0x1L<<16)
#define SPU_17CH (0x1L<<17)
#define SPU_18CH (0x1L<<18)
#define SPU_19CH (0x1L<<19)

#define SPU_20CH (0x1L<<20)
#define SPU_21CH (0x1L<<21)
#define SPU_22CH (0x1L<<22)
#define SPU_23CH (0x1L<<23)
// CDDA / XA volume
#define XA_CHANNELS 8
#define MVOL_L 0x3fff
#define MVOL_R 0x3fff
#define CDVOL_L 0x7fff
#define CDVOL_R 0x7fff
#define VOICEVOL_L 0x3fff
#define VOICEVOL_R 0x3fff

// Debug Font
#define FNT_VRAM_X 960
#define FNT_VRAM_Y 256
#define FNT_SCR_X 16
#define FNT_SCR_Y 128
#define FNT_SCR_W 240
#define FNT_SCR_H 88
#define FNT_SCR_BG 0
#define FNT_SCR_MAX_CHAR 512

// Ordering table
#define OT2LEN 8                 
#define OTLEN       4096
#define PRIMBUFFLEN 4096 * sizeof(POLY_GT4)     // Maximum number of POLY_GT4 primitives

#define cpu_uldr(r,dp)                                                 \
asm(\
  "sw %0, 0(%1);"                                                      \
  : "=r" (r)\
  : "r"  (dp)\
)

// DCache setup
#define  dc_camdirp ((sshort*)  getScratchAddr(0))
#define  dc_ip      ((uchar*)   getScratchAddr(1))
#define  dc_opzp    ((slong*)   getScratchAddr(2))
#define  dc_wrkmatp   ((MATRIX*)  getScratchAddr(3))
#define  dc_retmatp   ((MATRIX*)  getScratchAddr(9))
#define  dc_lgtmatp   ((MATRIX*)  getScratchAddr(12))
#define  dc_lvllgtmatp   ((MATRIX*)  getScratchAddr(16))
#define  dc_lvlcmatp   ((MATRIX*)  getScratchAddr(24))
#define  dc_lgtangp   ((SVECTOR*)  getScratchAddr(32))
#define  dc_wrklvector   ((VECTOR*)  getScratchAddr(34))
#define  dc_camMat   ((MATRIX*)  getScratchAddr(38))
#define  dc_camRot   ((SVECTOR*)  getScratchAddr(46))
#define  dc_camPos   ((SVECTOR*)  getScratchAddr(48))
#define  dc_actorRot   ((VECTOR*)  getScratchAddr(50))

//~ #define  dc_sxytbl  ((DVECTOR*) getScratchAddr(15)) // 6 DVEC == 12
//~ #define  dc_verts   ((SVECTOR*) getScratchAddr(27)) // store verts here
//~ #define  dc_verts1  ((SVECTOR*) getScratchAddr(35)) // store verts here
//~ #define  dc_verts2  ((SVECTOR*) getScratchAddr(43)) // store verts here

// Fog
#define FOG_NEAR 2300
#define FOG_FAR  2600

// Physics
#define GRAVITY 10
#define SCALE 4

// Pad codes defines

// Dual shock sticks dead zone
#define DS_DZ 60
// Applied on PADL
#define PadSelect  ( 1 )
#define PadStart ( 1 << 3 )

// Up, Right, Down, Left will be used on PADL (left side of pad )and PADR (right side of pad)
#define PadUp     ( 1 << 4 )
#define PadRight  ( 1 << 5 )
#define PadDown   ( 1 << 6 )
#define PadLeft   ( 1 << 7 )

#define Triangle       ( 1 << 12 )
#define Circle      ( 1 << 13 )
#define Cross     ( 1 << 14 )
#define Square     ( 1 << 15 )
// Joysticks
#define PadR3    ( 1 << 2 )
#define PadL3    ( 1 << 1 )

// Triggers applied on PADR
#define PadShldL1    ( 1 << 10 )
#define PadShldL2    ( 1 << 8)
#define PadShldR1    ( 1 << 11 )
#define PadShldR2    ( 1 << 9 )
