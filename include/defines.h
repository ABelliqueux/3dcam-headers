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

// Debug Font
#define FNT_VRAM_X 960
#define FNT_VRAM_Y 256
#define FNT_SCR_X 16
#define FNT_SCR_Y 192
#define FNT_SCR_W 240
#define FNT_SCR_H 32
#define FNT_SCR_BG 0
#define FNT_SCR_MAX_CHAR 256

// Ordering table
#define OT2LEN 8                 
#define OTLEN       4096
#define PRIMBUFFLEN 4096 * sizeof(POLY_GT4)     // Maximum number of POLY_GT4 primitives

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
#define PadR3    ( 1 << 2 )
#define PadL3    ( 1 << 1 )

// Triggers applied on PADR
#define PadShldL1    ( 1 << 2 )
#define PadShldL2    ( 1 )
#define PadShldR1    ( 1 << 3 )
#define PadShldR2    ( 1 << 1 )
