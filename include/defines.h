#define VMODE       0 // 0 == NTSC, 1 == PAL
#define VSYNC       0
#define SCREENXRES 320
#define SCREENYRES 240
#define CENTERX     SCREENXRES/2
#define CENTERY     SCREENYRES/2
#define FOV CENTERX
#define FOG_COLOR {128, 128, 128} // Default to neutral grey
#define CLEAR_COLOR_R 0
#define CLEAR_COLOR_G 0
#define CLEAR_COLOR_B 0

// Sound
#define SPU_00CH (0x1L<< 0)
#define SPU_01CH (0x1L<< 1)
#define SPU_02CH (0x1L<< 2)
#define SPU_03CH (0x1L<< 3)
#define SPU_04CH (0x1L<< 4)
#define SPU_05CH (0x1L<< 5)
#define SPU_06CH (0x1L<< 6)
#define SPU_07CH (0x1L<< 7)

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
#define FNT_SCR_Y 32
#define FNT_SCR_W 240
#define FNT_SCR_H 48
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
