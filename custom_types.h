#pragma once
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

struct BODY;
struct VANIM;
struct MESH_ANIMS_TRACKS;
struct PRIM;
struct MESH;
struct CAMPOS;
struct CAMPATH;
struct CAMANGLE;
struct SIBLINGS;
struct CHILDREN;
struct NODE;
struct QUAD;
struct LEVEL;
struct VAGsound;
struct VAGbank;
struct XAsound;
struct XAbank;
struct XAfiles;
struct SOUND_OBJECT;
struct LEVEL_SOUNDS;

typedef struct BODY {
	VECTOR  gForce;
	VECTOR  position;
	SVECTOR velocity;
	int     mass;
	int     invMass;
	VECTOR  min; 
	VECTOR  max; 
	int     restitution; 
	} BODY;

typedef struct VANIM { 
	int nframes;    // number of frames e.g   20
	int nvert;      // number of vertices e.g 21
	int cursor;     // anim cursor : -1 == not playing, n>=0 == current frame number
	int lerpCursor; // anim cursor
	int loop;       // loop anim : -1 == infinite, n>0  == play n times
	int dir;        // playback direction (1 or -1)
	int pingpong;   // ping pong animation (A>B>A)
	int interpolate; // use lerp to interpolate keyframes
	SVECTOR data[]; // vertex pos as SVECTORs e.g 20 * 21 SVECTORS
	} VANIM;

typedef struct MESH_ANIMS_TRACKS {
	u_short index;
	VANIM * strips[];
} MESH_ANIMS_TRACKS;

typedef struct PRIM {
	VECTOR order;
	int    code; // Same as POL3/POL4 codes : Code (F3 = 1, FT3 = 2, G3 = 3,
// GT3 = 4) Code (F4 = 5, FT4 = 6, G4 = 7, GT4 = 8)
	} PRIM;

typedef struct MESH {  
	int      totalVerts;
	TMESH   *    tmesh;
	PRIM    *    index;
	TIM_IMAGE *  tim;  
	unsigned long * tim_data;
	MATRIX      mat;
	VECTOR      pos;
	SVECTOR     rot;
	short       isProp;
	short       isRigidBody;
	short       isStaticBody;
	short       isRound;
	short       isPrism;
	short       isAnim;
	short       isActor;
	short       isLevel;
	short       isWall;
	short       isBG;
	short       isSprite;
	long        p;
	long        OTz;
	BODY     *  body;
	MESH_ANIMS_TRACKS    *  anim_tracks;
	VANIM *     currentAnim;
	struct NODE   *    node;
	VECTOR      pos2D;
	} MESH;

typedef struct QUAD {
	VECTOR       v0, v1;
	VECTOR       v2, v3;
	} QUAD;

typedef struct CAMPOS {
	SVECTOR  pos;
	SVECTOR rot;
	} CAMPOS;


// Blender cam ~= PSX cam with these settings : 
// NTSC - 320x240, PAL 320x256, pixel ratio 1:1,
// cam focal length : perspective 90° ( 16 mm ))
// With a FOV of 1/2, camera focal length is ~= 16 mm / 90°
// Lower values mean wider angle

typedef struct CAMANGLE {
	CAMPOS    * campos;
	TIM_IMAGE * BGtim;
	unsigned long * tim_data;
	QUAD  bw, fw;
	int index;
	MESH * objects[];
	} CAMANGLE;

typedef struct CAMPATH {
	short len, cursor, pos;
	VECTOR points[];
	} CAMPATH;

typedef struct SIBLINGS {
	int index;
	struct NODE * list[];
	} SIBLINGS ;

typedef struct CHILDREN {
	int index;
	MESH * list[];
	} CHILDREN ;

typedef struct NODE {
	MESH * plane;
	SIBLINGS * siblings;
	CHILDREN * objects;
	CHILDREN * rigidbodies;
	} NODE;

//VAG
typedef struct VAGsound {
	u_char * VAGfile;        // Pointer to VAG data address
	u_long spu_channel;      // SPU voice to playback to
	u_long spu_address;      // SPU address for memory freeing spu mem
	} VAGsound;

typedef struct VAGbank {
	u_int index;
	VAGsound samples[];
	} VAGbank;

// XA
typedef struct XAsound {
	u_int id;
	u_int size;
	u_char file, channel;
	u_int start, end;
	int cursor;
	} XAsound;

typedef struct XAbank {
	char name[16];
	u_int index;
	int offset;
	XAsound samples[];
	} XAbank;

typedef struct XAfiles {
	u_int index;
	XAbank * banks[];
	} XAfiles;

typedef struct SOUND_OBJECT {
	VECTOR location;
	int volumeL, volumeR, volume_min, volume_max;
	VAGsound * VAGsample;
	XAsound * XAsample;
	MESH * parent;
} SOUND_OBJECT;

typedef struct LEVEL_SOUNDS {
	int index;
	SOUND_OBJECT * sounds[];
} LEVEL_SOUNDS;

typedef struct LEVEL {
	CVECTOR * BGc;
	VECTOR * BKc;
	MATRIX * cmat;
	MATRIX * lgtmat;
	MESH   ** meshes;
	int * meshes_length;
	MESH * actorPtr;
	MESH * levelPtr;
	MESH * propPtr;
	CAMANGLE * camPtr;
	CAMPATH * camPath;
	CAMANGLE ** camAngles;
	NODE * curNode;
	LEVEL_SOUNDS * levelSounds;
	VAGbank * VAG;
	XAfiles * XA;
	} LEVEL;
