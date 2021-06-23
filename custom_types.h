#pragma once
#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>

struct BODY;
struct VANIM;
struct PRIM;
struct MESH;
struct CAMPOS;
struct CAMPATH;
struct CAMANGLE;
struct SIBLINGS;
struct CHILDREN;
struct NODE;
struct QUAD;

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
	int cursor;     // anim cursor
	int lerpCursor; // anim cursor
	int dir;        // playback direction (1 or -1)
	int interpolate; // use lerp to interpolate keyframes
	SVECTOR data[]; // vertex pos as SVECTORs e.g 20 * 21 SVECTORS
	} VANIM;

typedef struct PRIM {
	VECTOR order;
	int    code; // Same as POL3/POL4 codes : Code (F3 = 1, FT3 = 2, G3 = 3,
// GT3 = 4) Code (F4 = 5, FT4 = 6, G4 = 7, GT4 = 8)
	} PRIM;

typedef struct MESH {  
	TMESH   *    tmesh;
	PRIM    *    index;
	TIM_IMAGE *  tim;  
	unsigned long * tim_data;
	MATRIX      mat;
	VECTOR      pos;
	SVECTOR     rot;
	short       isRigidBody;
	short       isStaticBody;
	short       isPrism;
	short       isAnim;
	short       isActor;
	short       isLevel;
	short       isBG;
	short       isSprite;
	long        p;
	long        OTz;
	BODY     *  body;
	VANIM    *  anim;
	struct NODE   *    node;
	VECTOR      pos2D;
	} MESH;

typedef struct QUAD {
	VECTOR       v0, v1;
	VECTOR       v2, v3;
	} QUAD;

typedef struct CAMPOS {
	VECTOR  pos;
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

typedef struct LEVEL {
	CVECTOR * BGc;
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
	MESH * meshPlan; // This one is temporary
	} LEVEL;
