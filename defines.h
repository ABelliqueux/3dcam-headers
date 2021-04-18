#define VMODE       0

#define SCREENXRES 320

#define SCREENYRES 240

#define CENTERX		SCREENXRES/2

#define CENTERY		SCREENYRES/2

#define FOV CENTERX

#define FNT_POS_X 960

#define FNT_POS_Y 256

#define OT2LEN 8
                 
#define OTLEN	    256	

#define GRAVITY 10

#define SCALE 4

#define PRIMBUFFLEN	4096 * sizeof(POLY_GT4)	    // Maximum number of POLY_GT3 primitives


// Pad defines

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
