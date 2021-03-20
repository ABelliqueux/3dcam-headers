// 3dcam
// With huge help from @NicolasNoble : https://discord.com/channels/642647820683444236/646765703143227394/796876392670429204

		
 /*		   PSX screen coordinate system 
 *
 *                           Z+
 *                          /
 *                         /
 *                        +------X+
 *                       /|
 *                      / |
 *                     /  Y+
 *                   eye		*/
// bpy. app. debug = True 

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// Precalculated sin/cos values
//~ #include "psin.c"
//~ #include "pcos.c"
#include "atan.c"

// Sample vector model
#include "coridor2.c"
//~ #include "tst-quads.c"
//~ #include "gnd.c"
//~ #include "startcube.c"

#define VMODE       0

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX		SCREENXRES/2
#define CENTERY		SCREENYRES/2

// pixel > cm : used in physics calculations
#define SCALE 4

#define FNT_POS_X 960
#define FNT_POS_Y 0

#define OT2LEN	    8	                   
#define OTLEN	    256	                    // Maximum number of OT entries
#define PRIMBUFFLEN	4096 * sizeof(POLY_GT4)	    // Maximum number of POLY_GT3 primitives

// atantable
#define SWAP(a,b,c)			{(c)=(a); (a)=(b); (b)=(c);} // swap(x, y, buffer)

// dotproduct of two vectors
#define dotProduct(v0, v1)  \
	(v0).vx * (v1).vx +	\
	(v0).vy * (v1).vy +	\
	(v0).vz * (v1).vz

// min value
#define min(a,b)    \
        (a)-(b)>0?(b):(a)
// max
#define max(a,b)    \
        (a)-(b)>0?(a):(b)

#define subVector(v0, v1) \
	(v0).vx - (v1).vx,	\
	(v0).vy - (v1).vy,	\
	(v0).vz - (v1).vz	

//~ extern ushort rcossin_tbl[];

// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];

// OT for BG/FG discrimination
u_long otdisc[2][OT2LEN] = {0};

// Main OT
u_long	    ot[2][OTLEN]  = {0};   		        // Ordering table (contains addresses to primitives)

char	primbuff[2][PRIMBUFFLEN] = {0};	        // Primitive list // That's our prim buffer

//~ int		    primcnt=0;			            // Primitive counter

char * nextpri = primbuff[0];			        // Primitive counter

char		    db	= 0;                        // Current buffer counter


CVECTOR BGc = {50, 50, 75, 0};                  // Far color
VECTOR BKc = {128, 128, 128, 0};                // Back color

// Local color matrix   

//~ static MATRIX	cmat = {
//~ /* light source    #0, #1, #2, */
		//~ ONE,  0,  0, /* R */
		//~ 0,    ONE,  0, /* G */
		//~ 0,    0,  ONE, /* B */
//~ };

//~ // local light matrix : Direction and reach of each light source. 
//~ // Each light is aligned with the axis, hence direction is in the same coordinate system as the PSX (Y-axis down)
//~ // One == 4096 is reach/intensity of light source
//~ static MATRIX lgtmat = {
//~ //  X     Y     Z   
    //~ ONE, 0,  0,	    // Light 0  
	//~ 0,0,0,	    // Light 1 
	//~ 0,0,0	    // Light 2 
//~ };

// Light 

//~ MATRIX	    rottrans;	
MATRIX		rotlgt;	
SVECTOR	    lgtang = {0, 0, 0};	
MATRIX		light;
	
//~ SVECTOR	lgtang = {1024, -512, 1024};

static int m_cosTable[512];                     // precalc costable
static const unsigned int DC_2PI = 2048;        // this is from gere : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
static const unsigned int DC_PI  = 1024;
static const unsigned int DC_PI2 = 512;

short vs;

typedef struct{
    int x, xv;                                 // x: current value += xv : new value 
    int y, yv;                                 // x,y,z, vx, vy, vz are in PSX units (ONE == 4096)
    int z, zv;
    int pan, panv;
    int tilt, tiltv;
    int rol;

    VECTOR pos;
    SVECTOR rot;
    SVECTOR dvs;

    MATRIX mat;
} CAMERA;

CAMERA camera = {
    0,0,
    0,0,
    0,0,
    0,0,
    0,0,
    0,
    
    {0,0,0},
    {0,0,0},
    {0,0,0}
};


// physics
long time = 0;
const int gravity = 10;

//~ //vertex anim

//~ typedef struct {
    //~ int nframes;    // number of frames e.g   20
    //~ int nvert;      // number of vertices e.g 21
    //~ SVECTOR data[]; // vertex pos as SVECTORs e.g 20 * 21 SVECTORS
    //~ } VANIM;
    
    
//Pad
int pressed = 0;
u_short timer = 0;

// Cam stuff 
int camMode = 0;
long timeB = 0;
int lerping    = 0;

// Inverted Cam coordinates for Forward Vector calc
VECTOR InvCamPos = {0,0,0,0};
VECTOR fVecActor = {0,0,0,0};

u_long triCount = 0;

// Prototypes

// Stolen from grumpycoder

// Sin/Cos Table
void generateTable(void);
int  ncos(u_int t);
int  nsin(u_int t);

// Atan table
long long patan(long x, long y);

//sqrt
u_int psqrt(u_int n);

//~ typedef	unsigned int	uint32_t;
//~ typedef	int	int32_t;

// fixed point math
static inline int32_t dMul(int32_t a, int32_t b);
static inline uint32_t lerpU(uint32_t start, uint32_t dest, unsigned pos);
static inline int32_t lerpS(int32_t start, int32_t dest, unsigned pos);
static inline int32_t lerpD(int32_t start, int32_t dest, int32_t pos);
static inline long long lerpL(long long start, long long dest, long long pos);

// PSX setup
void init(void);
void display(void);

// Utils
void LoadTexture(u_long * tim, TIM_IMAGE * tparam);
int cliptest3(short * v1);
int lerp(int start, int end, int factor); // FIXME : not working as it should
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor); // FIXME 
VECTOR getVectorTo(VECTOR actor, VECTOR target);
int alignAxisToVect(VECTOR target, short axis, int factor);

// Camera 
void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance);
void applyCamera(CAMERA * cam);
void setCameraPos(VECTOR pos, SVECTOR rot);

// Physics
VECTOR getIntCollision(BODY one, BODY two);
VECTOR getExtCollision(BODY one, BODY two);
void   ResolveCollision( BODY * one, BODY * two );
VECTOR angularMom(BODY body);                    // Not yours ;)

void applyAcceleration(BODY * actor);

void callback();

int main() {
	
    // Mesh stuff
	int		i;
	long	t, p, OTz, OTc, Flag, nclip;                // t == vertex count, p == depth cueing interpolation value, OTz ==  value to create Z-ordered OT, Flag == see LibOver47.pdf, p.143
    POLY_GT3 * poly;                        
    POLY_GT4 * poly4;      
                      
    SPRT * sprt;                        
    DR_TPAGE * tpage;
    
    // Poly subdiv
    DIVPOLYGON4	div4 = { 0 };
    div4.pih = SCREENXRES;
	div4.piv = SCREENYRES;
    div4.ndiv = 2;
    
    //~ DIVPOLYGON3	div3 = { 0 };
    //~ div3.pih = SCREENXRES;
	//~ div3.piv = SCREENYRES;
    //~ div3.ndiv = 1;
    
    MATRIX Cmatrix = {0};
    
	init();

    generateTable();

    VSyncCallback(callback);
    
    //~ SetLightMatrix(&LLM);
	SetColorMatrix(&cmat);
    
    SetBackColor(BKc.vx,BKc.vy,BKc.vz);
    //~ SetFarColor(BGc.r, BGc.g, BGc.b);
    SetFogNearFar(1200, 1600, SCREENXRES);
    
    for (int k = 0; k < sizeof(meshes)/sizeof(TMESH *); k++){
        LoadTexture(meshes[k]->tim_data, meshes[k]->tim);
    }
    
    //~ camPtr = &camAngle_camPath_001;
    
    if (camPtr->tim_data){
        LoadTexture(camPtr->tim_data, camPtr->BGtim);
    }
    
    // physics
    short physics = 1;
    long dt;

    VECTOR col_lvl, col_sphere, col_sphere_act = {0};
    
    // Cam stuff 
    
    VECTOR posToActor    = {0, 0, 0, 0};      // position of camera relative to actor    
    VECTOR camAngleToAct = {0, 0, 0, 0};      // rotation angles for the camera to point at actor
    
    // Sprite sustem
    VECTOR posToCam      = {0, 0, 0, 0};
    VECTOR objAngleToCam = {0, 0, 0, 0};
    
    int angle     = 0;                      //PSX units = 4096 == 360° = 2Pi
    int dist      = 0;                      //PSX units 

    short timediv = 1;

    int atime = 0;
    
    for (int k = 0; k < sizeof(meshes)/sizeof(meshes[0]); k++){
            triCount += meshes[k]->tmesh->len;
    }
    
    // Pre-calc bg test
    setCameraPos(camPtr->campos->pos, camPtr->campos->rot);
    
    //~ camera.rot.vz = 100;
    
	// Main loop
	while (1) {
        
        // Clear the main OT
		ClearOTagR(otdisc[db], OT2LEN);
		
        // Clear Secondary OT
        ClearOTagR(ot[db], OTLEN);
        
        // timeB = time;
        time ++;
        
        // atime is used for animations timing
        timediv = 1;
        
        if (time % timediv == 0){
            atime ++;
        }
            
        // Angle between camera and actor
        // using atantable (faster)
        camAngleToAct.vy = (patan(-posToActor.vx, -posToActor.vz) / 16) - 3076 ;
        camAngleToAct.vx = patan(dist, posToActor.vy) >> 4;

        //~ posToCam = getVectorTo(*meshPlan.pos, camera.pos);
        //~ posToCam = getVectorTo(camera.pos, *meshPlan.pos);

        //~ objAngleToCam.vy = ( (patan(meshPlan.pos->vx, meshPlan.pos->vz) - patan(camera.pos.vx > 0 ? camera.pos.vx : camera.pos.vx, camera.pos.vz) ) >> 4) - 1024;
        //~ objAngleToCam.vy = ( (patan(meshPlan.pos->vx > 0 ? meshPlan.pos->vx : 4096 + meshPlan.pos->vx, meshPlan.pos->vz) - patan(camera.pos.vx > 0 ? camera.pos.vx : 4096 + camera.pos.vx, camera.pos.vz) ) >> 4) - 1024;
        //~ objAngleToCam.vx = ratan2(posToCam.pad, posToCam.vy);
        
        //~ meshPlan.rot->vy = objAngleToCam.vy;
        //~ meshPlan.rot->vx = objAngleToCam.vy;
        
        // Actor Forward vector 

        fVecActor = *actorPtr->pos;
        
        fVecActor.vx = actorPtr->pos->vx + (nsin(actorPtr->rot->vy/2));
        fVecActor.vz = actorPtr->pos->vz - (ncos(actorPtr->rot->vy/2));

        if(camMode != 2){
            
            camera.rot.vy = camAngleToAct.vy;
            // using csin/ccos, no need for theta
            //~ camera.rot.vy = angle; 
            camera.rot.vx = camAngleToAct.vx;   
        
        }
        
        if(camMode < 4 ){
            lerping = 0;
            }
        
        // Camera follows actor with lerp for rotations
        if(camMode == 0){                       
            dist = 150;
            camera.pos.vx = -(camera.x/ONE);
            //~ camera.pos.vy = -(camera.y/ONE);
            camera.pos.vz = -(camera.z/ONE);
            
            //~ InvCamPos.vx = camera.x/ONE;
            //~ InvCamPos.vz = camera.z/ONE;
        
            //~ applyVector(&InvCamPos, -1,-1,-1, *=);
            
            angle = -actorPtr->rot->vy / 2;
            //~ angle = actorPtr->rot->vy;

            getCameraXZ(&camera.x, &camera.z, actorPtr->pos->vx, actorPtr->pos->vz, angle, dist);

            // FIXME! camera lerping to pos
            //~ angle += lerp(camera.rot.vy, -actorPtr->rot->vy, 128);
            //~ angle = lerpD(camera.rot.vy << 12, actorPtr->rot->vy << 12, 1024 << 12) >> 12;
            
        }
        
        // Camera rotates continuously around actor
        if (camMode == 1){                      
            
            dist = 150;
            camera.pos.vx = -(camera.x/ONE);
            //~ camera.pos.vy = -(camera.y/ONE);
            camera.pos.vz = -(camera.z/ONE);
            
            //~ fVecActor = *actorPtr->pos;
            
            //~ fVecActor.vx = actorPtr->pos->vx + (nsin(actorPtr->rot->vy));
            //~ fVecActor.vz = actorPtr->pos->vz - (ncos(actorPtr->rot->vy));
            
            
            getCameraXZ(&camera.x, &camera.z, actorPtr->pos->vx, actorPtr->pos->vz, angle, dist);
            angle += 10;
        }
        
        // Fixed Camera with actor tracking
        if (camMode == 3){                              
            
            // Using libgte sqrt ( slower)
            //~ dist = SquareRoot0( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
            
            // Using precalc sqrt
            dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
            
            camera.pos.vx = 190;
            camera.pos.vz = 100;
            camera.pos.vy = 180;
        }
        
        // Fixed Camera angle
        if (camMode == 2){                              
            
            // Load BG image in two SPRT since max width == 256 
            if (camPtr->tim_data){
                
                // left part
                sprt = (SPRT *) nextpri;
                
                setSprt(sprt);
                setRGB0(sprt, 128,128,128);
                setXY0(sprt, 0, 0);
                setWH(sprt, 256, 240);
                setUV0(sprt, 0, 0);
                setClut(sprt, camPtr->BGtim->crect->x, camPtr->BGtim->crect->y);
                
                addPrim(&otdisc[db][OT2LEN-1], sprt);        
                            
                nextpri += sizeof(SPRT);
                
                tpage = (DR_TPAGE *) nextpri;
                
                setDrawTPage(tpage, 0, 1,   
                             getTPage(camPtr->BGtim->mode & 0x3, 0,       
                             camPtr->BGtim->prect->x, camPtr->BGtim->prect->y));
                                                        
                addPrim(&otdisc[db][OT2LEN-1], tpage);

                nextpri += sizeof(DR_TPAGE); 
                
                
                // right part
                sprt = (SPRT *) nextpri;
                
                setSprt(sprt);
                setRGB0(sprt, 128,128,128);
                setXY0(sprt, 320-(320-256), 0);
                setWH(sprt, 320-256, 240);
                setUV0(sprt, 0, 0);
                
                setClut(sprt, camPtr->BGtim->crect->x, camPtr->BGtim->crect->y);
                            
                addPrim(&otdisc[db][OT2LEN-1], sprt);        
                            
                nextpri += sizeof(SPRT);
                
                tpage = (DR_TPAGE *) nextpri;
                
                setDrawTPage(tpage, 0, 1,   
                             getTPage(camPtr->BGtim->mode & 0x3, 0,       
                             camPtr->BGtim->prect->x + 128, camPtr->BGtim->prect->y));
                                                        
                addPrim(&otdisc[db][OT2LEN-1], tpage);

                nextpri += sizeof(DR_TPAGE); 
            }
            
            setCameraPos(camPtr->campos->pos, camPtr->campos->rot);

        }
        
        // Flyby mode with LERP from camStart to camEnd
        if (camMode == 4){                               
            
            // If key pos exist for camera
            if (camPath.len) {
                
                // Lerping sequence has not begun
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
                    camera.pos.vx = camPath.points[camPath.cursor].vx;
                    camera.pos.vy = camPath.points[camPath.cursor].vy;
                    camera.pos.vz = camPath.points[camPath.cursor].vz;
                    
                    // Lerping sequence is starting
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    camPath.pos = 0;
                    
                    }
                    
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
                int precision = 12;
                
                camera.pos.vx = lerpD(camPath.points[camPath.cursor].vx << precision, camPath.points[camPath.cursor+1].vx << precision, camPath.pos << precision) >> precision;
                camera.pos.vy = lerpD(camPath.points[camPath.cursor].vy << precision, camPath.points[camPath.cursor+1].vy << precision, camPath.pos << precision) >> precision;
                camera.pos.vz = lerpD(camPath.points[camPath.cursor].vz << precision, camPath.points[camPath.cursor+1].vz << precision, camPath.pos << precision) >> precision;
                
                //~ FntPrint("Cam %d, %d\n", (int32_t)camPath.points[camPath.cursor].vx, camPath.points[camPath.cursor+1].vx);
                //~ FntPrint("Cam %d, %d, %d\n", camera.pos.vx, camera.pos.vy, camera.pos.vz);
                //~ FntPrint("Theta y: %d x: %d\n", theta.vy, theta.vx);
                //~ FntPrint("Pos: %d Cur: %d\nTheta y: %d x: %d\n", camPath.pos, camPath.cursor, theta.vy, theta.vx);

                // Linearly increment the lerp factor
                camPath.pos += 20;
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (camPath.pos > (1 << precision) ){
                    camPath.pos = 0;
                    camPath.cursor ++;
                }                    
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( camPath.cursor == camPath.len - 1 ){
                    lerping = 0;
                    camPath.cursor = 0;
                }
                
            } else { 
                // if no key pos exists, switch to next camMode
                camMode ++; }
        }
        
        // Camera "on a rail" - cam is tracking actor, and moving with constraints on all axis
        if (camMode == 5) {
            
            // track actor. If theta (actor/cam rotation angle) is above or below an arbitrary angle, 
            // move cam so that the angle doesn't increase/decrease anymore.
            
            if (camPath.len) {
            // Lerping sequence has not begun
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
                    camera.pos.vx = camPath.points[camPath.cursor].vx;
                    camera.pos.vy = camPath.points[camPath.cursor].vy;
                    camera.pos.vz = camPath.points[camPath.cursor].vz;
                    
                    // Lerping sequence is starting
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    camPath.pos = 0;
                    
                    }
                    
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
                short precision = 12;
                                    
                camera.pos.vx = lerpD(camPath.points[camPath.cursor].vx << precision, camPath.points[camPath.cursor + 1].vx << precision, camPath.pos << precision) >> precision;
                camera.pos.vy = lerpD(camPath.points[camPath.cursor].vy << precision, camPath.points[camPath.cursor + 1].vy << precision, camPath.pos << precision) >> precision;
                camera.pos.vz = lerpD(camPath.points[camPath.cursor].vz << precision, camPath.points[camPath.cursor + 1].vz << precision, camPath.pos << precision) >> precision;
                
                //~ FntPrint("Cam %d, %d\n", (int32_t)camPath.points[camPath.cursor].vx, camPath.points[camPath.cursor+1].vx);
                //~ FntPrint("Cam %d, %d, %d\n", camera.pos.vx, camera.pos.vy, camera.pos.vz);
                //~ FntPrint("Pos: %d Cur: %d\nTheta y: %d x: %d\n", camPath.pos, camPath.cursor, theta.vy, theta.vx);

                if ( camAngleToAct.vy < -50 ) {  
                    camPath.pos += 40;
                }
                if ( camAngleToAct.vy > 50 ) {  
                    camPath.pos -= 40;
                }
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (camPath.pos > (1 << precision) ){
                    camPath.pos = 0;
                    camPath.cursor ++;
                    //~ camPath.dir = 1;
                } 
                
                if (camPath.pos < -100 ){
                    camPath.pos = 1 << precision;
                    camPath.cursor --;
                    //~ camPath.dir *= -1;
                }                   
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( camPath.cursor == camPath.len - 1 || camPath.cursor < 0 ){
                    lerping = 0;
                    camPath.cursor = 0;
                }
                
            } else { 
                // if no key pos exists, switch to next camMode
                camMode ++; }
            
        }
    
        //~ dt = time/180+1 - time/180;
        
        
        // Spatial partitioning
        
        for (int msh = 0; msh < curNode->siblings->index; msh ++){
        
            if ( !getIntCollision( *actorPtr->body , *curNode->siblings->list[msh]->plane->body).vx &&
                 !getIntCollision( *actorPtr->body , *curNode->siblings->list[msh]->plane->body).vz )
            {
            
            // FntPrint("%d", msh );
            //~ curNode = curNode->siblings->list[msh];
            // curNode = &nodegnd;
            levelPtr = curNode->siblings->list[msh]->plane;
            
            }
            
        }
        
        // Physics
        
        if (physics){
            
            // if(time%1 == 0){
                
                 for ( int k = 0; k < sizeof(meshes)/sizeof(meshes[0]);k ++){
                //~ for ( int k = 0; k < curNode->objects->index ; k ++){
                                        
                     if ( ( *meshes[k]->isRigidBody == 1 ) ) {
                    //~ if ( ( *curNode->rigidbodies->list[k]->isRigidBody == 1 ) ) {

                        //~ applyAcceleration(curNode->rigidbodies->list[k]->body);
                        
                        applyAcceleration(meshes[k]->body);
                    
                        // Get col with level                         ( modelgnd_body )        
                        
                        col_lvl = getIntCollision( *meshes[k]->body , *levelPtr->body );
                        
                        //~ col_lvl = getIntCollision( *actorPtr->body , *curNode->plane->body );
                        
                        //~ col_sphere = getIntCollision( *propPtr->body, *propPtr->body->curNode->plane->body );
                        
                        col_sphere = getIntCollision( *propPtr->body, *levelPtr->body );
                        
                        col_sphere_act = getExtCollision( *actorPtr->body, *propPtr->body );
                        
                        //~ // If !col, keep moving
                        
                        //~ if ( !col_lvl.vx ){ curNode->rigidbodies->list[k]->pos->vx = curNode->rigidbodies->list[k]->body->position.vx; } 
                        
                        //~ if ( !col_lvl.vy ){ curNode->rigidbodies->list[k]->pos->vy = curNode->rigidbodies->list[k]->body->position.vy; };//meshes[k]->body->gForce.vy = 0;} // FIXME : Why the 15px offset ? 
                        
                        //~ if ( !col_lvl.vz ){ curNode->rigidbodies->list[k]->pos->vz = curNode->rigidbodies->list[k]->body->position.vz; }
                                               
                        //~ // If no col with ground, fall off
                
                        if ( col_lvl.vy ) {
                            if (!col_lvl.vx && !col_lvl.vz){actorPtr->body->position.vy = actorPtr->body->min.vy;}
                        }
                        if (col_sphere.vy){
                            if (!col_sphere.vx && !col_sphere.vz){propPtr->body->position.vy = propPtr->body->min.vy; }
                        }
                        
                        //~ if (col_sphere_act.vx && col_sphere_act.vz ){

                            //~ propPtr->body->velocity.vx += actorPtr->body->velocity.vx;// * ONE / propPtr->body->restitution ;
                            //~ propPtr->body->velocity.vz += actorPtr->body->velocity.vz;// * ONE / propPtr->body->restitution ;
                            
                            //~ if (propPtr->body->velocity.vx){
                                
                                //~ VECTOR L = angularMom(*propPtr->body);
                                //~ propPtr->rot->vz -= L.vx;
                            //~ }
                            
                            //~ if (propPtr->body->velocity.vz){
                                
                                //~ VECTOR L = angularMom(*propPtr->body);
                                //~ propPtr->rot->vx -= L.vz;
                            //~ }
                        //~ }
                        
                        //~ if (!col_sphere_act.vx){
                            //~ propPtr->body->velocity.vx = 0;
                            //~ }
                        
                        

                        meshes[k]->pos->vx = meshes[k]->body->position.vx;
                        meshes[k]->pos->vy = meshes[k]->body->position.vy ;
                        meshes[k]->pos->vz = meshes[k]->body->position.vz;
                        
                        
                    }
                    
                    meshes[k]->body->velocity.vy = 0;
                    meshes[k]->body->velocity.vx = 0;
                    meshes[k]->body->velocity.vz = 0;
                    
                    //~ curNode->rigidbodies->list[k]->body->velocity.vx = curNode->rigidbodies->list[k]->body->velocity.vz = 0;
                    
                    //~ FntPrint("V:%d ", curNode->rigidbodies->list[k]->body->velocity.vy);
                }
            // }
        }
        
        // Camera setup 
        
        // position of cam relative to actor
        posToActor.vx = actorPtr->pos->vx + camera.pos.vx;
        posToActor.vz = actorPtr->pos->vz + camera.pos.vz;
        posToActor.vy = actorPtr->pos->vy + camera.pos.vy;
        
        // position of object relative to cam
        
        
		// Clear the current OT
		// ClearOTagR(ot[db], OTLEN);


        // Polygon drawing
		for (int k = 0; k < sizeof(meshes)/sizeof(meshes[0]); k++){
        
            // loop on each mesh
            t=0;
            
            // If rigidbdy, apply rot/transform matrix
            if (*meshes[k]->isRigidBody | *meshes[k]->isStaticBody){
                                        
                    //~ PushMatrix();                                         // Push current matrix on the stack (real slow -> dma transfer )
                    
                    RotMatrix_gte(meshes[k]->rot, meshes[k]->mat);            // Apply rotation matrix
                    TransMatrix(meshes[k]->mat, meshes[k]->pos);
                                      
                    // Apply translation matrix
                    
                    CompMatrix(&camera.mat, meshes[k]->mat, meshes[k]->mat);  // Was using &PolyMatrix instead of meshes[k]->mat 

                    
                    SetRotMatrix(meshes[k]->mat);                             // Set default rotation matrix - Was using &PolyMatrix instead of meshes[k]->mat
                    SetTransMatrix(meshes[k]->mat);                                                          // Was using &PolyMatrix instead of meshes[k]->mat
                    
            }

            // mesh is POLY_GT3 ( triangle )
            if (meshes[k]->index[t].code == 4) {
                //~ t=0;
                      
                // modelCube is a TMESH, len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
                for (i = 0; i < (meshes[k]->tmesh->len * 3); i += 3) {               
                    
                    // if mesh is not part of BG, draw them, else, discard
                    if (!*meshes[k]->isBG || camMode != 2) {
                    
                        poly = (POLY_GT3 *)nextpri;

                        // FIXME : Polygon subdiv - is it working ?
                        
                        //~ OTc = *meshes[k]->OTz>>4;
                        //~ FntPrint("OTC:%d", OTc);
                        //~ if (OTc < 15) {
                        
                            //~ if (OTc > 5) div3.ndiv = 1; else div3.ndiv = 1;
                                
                                //~ DivideGT3(
                                    //~ // Vertex coord
                                    //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],  
                                    //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz ],
                                    //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                    //~ // UV coord
                                    //~ meshes[k]->tmesh->u[i+0],
                                    //~ meshes[k]->tmesh->u[i+2],
                                    //~ meshes[k]->tmesh->u[i+1],
                                    
                                    //~ // Color
                                    //~ meshes[k]->tmesh->c[i], 
                                    //~ meshes[k]->tmesh->c[i+1], 
                                    //~ meshes[k]->tmesh->c[i+2],

                                    //~ // Gpu packet
                                    //~ poly,
                                    //~ &ot[db][*meshes[k]->OTz],
                                    //~ &div3);
                                            
                                // Increment primitive list pointer
                                //~ nextpri  += ( (sizeof(POLY_GT3) + 3) / 4 ) * (( 1 << ( div3.ndiv )) << ( div3.ndiv ));
                                //~ triCount = ((1<<(div3.ndiv))<<(div3.ndiv));
                                //triCount = ( (sizeof(POLY_GT3) + 3) / 4 ) * (( 1 << ( div3.ndiv )) << ( div3.ndiv ));
                        
                        //~ }
                        
                        // Vertex Anim
                        if (*meshes[k]->isAnim){
                        
                            // with interpolation
                            if(meshes[k]->anim->interpolate){
                                
                                
                                 // ping pong
                                 //~ if (meshes[k]->anim->cursor > 4096 || meshes[k]->anim->cursor < 0){
                                    //~ meshes[k]->anim->dir *= -1;
                                 //~ }
                                 
                                 short precision = 12;

                                 //~ // next keyframe 
                                 if (meshes[k]->anim->cursor > (1 << precision)) {
                                    if ( meshes[k]->anim->lerpCursor < meshes[k]->anim->nframes - 1 ) {
                                        meshes[k]->anim->lerpCursor ++;
                                        meshes[k]->anim->cursor = 0;
                                    }
                                    if ( meshes[k]->anim->lerpCursor == meshes[k]->anim->nframes - 1 ) {
                                     //~ else {
                                        meshes[k]->anim->lerpCursor = 0;
                                        meshes[k]->anim->cursor = 0;
                                    }
                                 }
                                 
                                 //~ FntPrint("%d %d %d\n",meshes[k]->anim->lerpCursor, meshes[k]->anim->nframes, meshes[k]->anim->cursor );
                                                          
                                 // overflows somewhere ?
                                 //~ for (int v = 0; v <= 1; v++){
                                     
                                 //~ meshes[k]->tmesh->v[* &meshes[k]->index[t].order.vx + v].vx = lerpD( meshes[k]->anim->data[0 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vx << 12 , meshes[k]->anim->data[10 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vx  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                 //~ meshes[k]->tmesh->v[* &meshes[k]->index[t].order.vx + v].vz = lerpD( meshes[k]->anim->data[0 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vz << 12 , meshes[k]->anim->data[10 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vz  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                 //~ meshes[k]->tmesh->v[* &meshes[k]->index[t].order.vx + v].vy = lerpD( meshes[k]->anim->data[0 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vy << 12 , meshes[k]->anim->data[10 * meshes[k]->anim->nvert + * &meshes[k]->index[t].order.vx + v].vy  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                
                                //~ }

                                // Let's lerp between keyframes
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vx << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vx  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vz << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vz  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vy << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vy  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                 
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vx << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vx  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vz << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vz  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vy << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vy  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vx << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vx  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vz << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vz  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vy << precision , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vy  << precision, meshes[k]->anim->cursor << precision)  >> precision;
                                 
                                meshes[k]->anim->cursor += 2 * meshes[k]->anim->dir;
                                 
                                //~ FntPrint("%d %d\n", meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vx, meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vz);
                                //~ FntPrint("%d %d\n", *&meshes[k]->index[t].order.vx, *(&meshes[k]->index[t].order.vx+1));
                                //~ FntPrint("Anim fps : %d\n",  meshes[k]->anim->cursor);
                                 
                                // Coord transformation
                                nclip = RotAverageNclip3(
                                        &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],  
                                        &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz ],
                                        &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                        (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                                        meshes[k]->p,
                                        meshes[k]->OTz,
                                        &Flag
                                    );
                                    
                                } else { 
                                    // No interpolation : just take the vertices coordinates from the anim data
                                    nclip = RotAverageNclip3(
                                        &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx],
                                        &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz],
                                        &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy],
                                        (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                                        meshes[k]->p,
                                        meshes[k]->OTz,
                                        &Flag
                                    );
                            }
                                
                        } else {                        
                            // No animation
                            // Use model's regular vertex pos
                            nclip = RotAverageNclip3(
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],  
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz ],
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                                meshes[k]->p,
                                meshes[k]->OTz,
                                &Flag
                                );
                        }
                        
                        if (nclip > 0 && *meshes[k]->OTz > 0 && (*meshes[k]->p < 4096) ) {

                            
                            SetPolyGT3(poly);
                            
                            // Transparency effect
                            if (*meshes[k]->isPrism){ 
                                
                                // Use current DRAWENV clip as TPAGE
                                ((POLY_GT3 *)poly)->tpage = getTPage(meshes[k]->tim->mode&0x3, 0,
                                                                     draw[db].clip.x,
                                                                     draw[db].clip.y
                                );
                                
                                //~ setShadeTex(poly, 1);
                                // Use projected coordinates (results from RotAverage...) as UV coords and clamp them to 0-255,0-224 
                                setUV3(poly,  (poly->x0 < 0? 0 : poly->x0 > 255? 255 : poly->x0), 
                                              (poly->y0 < 0? 0 : poly->y0 > 224? 224 : poly->y0), 
                                              (poly->x1 < 0? 0 : poly->x1 > 255? 255 : poly->x1), 
                                              (poly->y1 < 0? 0 : poly->y1 > 224? 224 : poly->y1), 
                                              (poly->x2 < 0? 0 : poly->x2 > 255? 255 : poly->x2), 
                                              (poly->y2 < 0? 0 : poly->y2 > 224? 224 : poly->y2)
                                              );
                                
             
                            } else {
                                
                                // Use regular TPAGE
                                ((POLY_GT3 *)poly)->tpage = getTPage(meshes[k]->tim->mode&0x3, 0,
                                                                 meshes[k]->tim->prect->x,
                                                                 meshes[k]->tim->prect->y
                                );
                                
                                //~ if (!meshes[k]->isBG) {
                                    //~ setShadeTex(poly, 1);
                                    //~ setSemiTrans(poly, 1);
                                //~ }
                                setUV3(poly,  meshes[k]->tmesh->u[i].vx  , meshes[k]->tmesh->u[i].vy   + meshes[k]->tim->prect->y,
                                              meshes[k]->tmesh->u[i+2].vx, meshes[k]->tmesh->u[i+2].vy + meshes[k]->tim->prect->y,
                                              meshes[k]->tmesh->u[i+1].vx, meshes[k]->tmesh->u[i+1].vy + meshes[k]->tim->prect->y);
                                //~ }
                                 //~ else {
                            
                                //~ // Use model UV coordinates
                                //~ setUV3(poly,  255  , 255,
                                              //~ 255  , 255,
                                              //~ 255  , 255);
                                
                                //~ }

                            }

                            // If tim mode  == 0 | 1, set CLUT coordinates
                            if ((meshes[k]->tim->mode & 0x3) < 2){
                                setClut(poly,             
                                        meshes[k]->tim->crect->x,
                                        meshes[k]->tim->crect->y);
                            }
                                         
                        // If vertex anim has updated normals
                        
                            //~ if (*meshes[k]->isAnim){
                                // &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx],
                                //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t]], &meshes[k]->tmesh->c[meshes[k]->index[t]], *meshes[k]->p, &outCol);
                                //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+1]], &meshes[k]->tmesh->c[meshes[k]->index[t+1]], *meshes[k]->p, &outCol1);
                                //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+2]], &meshes[k]->tmesh->c[meshes[k]->index[t+2]], *meshes[k]->p, &outCol2);
                            //~ } else {
                            
                            
                            
                            // using precalc BG, default to black
                            CVECTOR outCol  ={128,128,128,0};
                            CVECTOR outCol1 ={128,128,128,0};
                            CVECTOR outCol2 ={128,128,128,0};
                        
                            //~ if ( !camPtr->tim_data ) {
                            
                                // default to neutral grey
                            //~ outCol.r , outCol.g , outCol.b  = 128;
                            //~ outCol1.r, outCol1.g, outCol1.b = 128;
                            //~ outCol2.r, outCol2.g, outCol2.b = 128;
                        
                            NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vx ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vx ], *meshes[k]->p, &outCol);
                            NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vz ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vz ], *meshes[k]->p, &outCol1);
                            NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vy ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vy ], *meshes[k]->p, &outCol2);                           
                        
                            //~ }
                            
                            if (*meshes[k]->isPrism){ 
                                
                                // Use un-interpolated (i.e: no light, no fog) colors
                                setRGB0(poly, meshes[k]->tmesh->c[i].r,   meshes[k]->tmesh->c[i].g, meshes[k]->tmesh->c[i].b);
                                setRGB1(poly, meshes[k]->tmesh->c[i+1].r, meshes[k]->tmesh->c[i+1].g, meshes[k]->tmesh->c[i+1].b);
                                setRGB2(poly, meshes[k]->tmesh->c[i+2].r, meshes[k]->tmesh->c[i+2].g, meshes[k]->tmesh->c[i+2].b);
                            
                            } else {
                                
                                setRGB0(poly, outCol.r, outCol.g  , outCol.b);
                                setRGB1(poly, outCol1.r, outCol1.g, outCol1.b);
                                setRGB2(poly, outCol2.r, outCol2.g, outCol2.b);
                            } 
                                   
                            if ((*meshes[k]->OTz > 0) && (*meshes[k]->OTz < OTLEN) && (*meshes[k]->p < 4096)){
                                AddPrim(&ot[db][*meshes[k]->OTz-2], poly);        // OTz - 2
                            }
                            
                            nextpri += sizeof(POLY_GT3);
                        }
                    
                        t+=1;
                    
                    //~ if (*meshes[k]->isRigidBody){
                        //~ PopMatrix();                    // Pull previous matrix from stack (slow)
                    //~ }
                    }
                }
            
            } 

            // mesh is POLY_GT4 ( quads )
            if (meshes[k]->index[t].code == 8) {
                t=0;

                for (i = 0; i < (meshes[k]->tmesh->len * 4); i += 4) {               
                    
                    // if mesh is not part of BG, draw them, else, discard
                    if (!*meshes[k]->isBG || camMode != 2) {
                    
                        poly4 = (POLY_GT4 *)nextpri;
                            
                                         
                        // Vertex Anim 
                        if (*meshes[k]->isAnim){
                            
                            // with interpolation
                            if(meshes[k]->anim->interpolate){
                                
                                // ping pong
                                //~ if (meshes[k]->anim->cursor > 4096 || meshes[k]->anim->cursor < 0){
                                   //~ meshes[k]->anim->dir *= -1;
                                //~ }
                                
                                short precision = 12;

                                if (meshes[k]->anim->cursor > 1<<precision) {

                                    if ( meshes[k]->anim->lerpCursor < meshes[k]->anim->nframes - 1 ) {

                                        meshes[k]->anim->lerpCursor ++;

                                        meshes[k]->anim->cursor = 0;

                                    }

                                    if ( meshes[k]->anim->lerpCursor == meshes[k]->anim->nframes - 1 ) {

                                        meshes[k]->anim->lerpCursor = 0;

                                        meshes[k]->anim->cursor = 0;
                                    }
                                }
                                
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vx << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vx  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vz << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vz  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vx].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vy << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx].vy  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vx << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vx  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vz << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vz  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vz].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vy << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz].vy  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vx << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vx  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vz << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vz  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.vy].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vy << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy].vy  << 12, meshes[k]->anim->cursor << 12)  >> 12;
                                
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.pad].vx = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vx << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vx  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.pad].vz = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vz << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vz  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                meshes[k]->tmesh->v[meshes[k]->index[t].order.pad].vy = lerpD( meshes[k]->anim->data[meshes[k]->anim->lerpCursor * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vy << 12 , meshes[k]->anim->data[(meshes[k]->anim->lerpCursor + 1) * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad].vy  << 12, meshes[k]->anim->cursor << 12) >> 12;
                                
                                meshes[k]->anim->cursor += 2 * meshes[k]->anim->dir;
                                
                                // Coord transformations
                                nclip = RotAverageNclip4(
                                    &meshes[k]->tmesh->v[ meshes[k]->index[t].order.pad ],  
                                    &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz],
                                    &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],
                                    &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                    (long*)&poly4->x0, (long*)&poly4->x1, (long*)&poly4->x2, (long*)&poly4->x3,
                                    meshes[k]->p,
                                    meshes[k]->OTz,
                                    &Flag
                                    );
                                    
                            } else {
                                
                                // No interpolation, use all vertices coordinates in anim data
                                 
                                OTz = RotAverageNclip4(
                                    &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.pad ],
                                    &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vz ],
                                    &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vx ],
                                    &meshes[k]->anim->data[ atime % meshes[k]->anim->nframes * meshes[k]->anim->nvert + meshes[k]->index[t].order.vy ],
                                    (long*)&poly4->x0, (long*)&poly4->x1, (long*)&poly4->x2, (long*)&poly4->x3,
                                    meshes[k]->p,
                                    meshes[k]->OTz,
                                    &Flag
                                );
                            }
                                    
                        } else {                        
                        
                            // No animation
                            // Use regulare vertex coords
                            
                            nclip = RotAverageNclip4(
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.pad ],  
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz],
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],
                                &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                (long*)&poly4->x0, (long*)&poly4->x1, (long*)&poly4->x2, (long*)&poly4->x3,
                                meshes[k]->p,
                                meshes[k]->OTz,
                                &Flag
                            );
                        }
                        
                        if (nclip > 0 && *meshes[k]->OTz > 0 && (*meshes[k]->p < 4096)) {
                     
                            SetPolyGT4(poly4);
                                
                            //~ // FIXME : Polygon subdiv - is it working ?
                            
                            //~ OTc = *meshes[k]->OTz >> 4;
                            //~ FntPrint("OTC:%d", OTc);
                            
                            //~ if (OTc < 4) {
                            
                                //~ if (OTc > 1) div4.ndiv = 1; else div4.ndiv = 2;
                                    
                                    //~ DivideGT4(
                                        //~ // Vertex coord
                                        //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.pad ],  
                                        //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz ],
                                        //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],
                                        //~ &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                                        //~ // UV coord
                                        //~ meshes[k]->tmesh->u[i+3],
                                        //~ meshes[k]->tmesh->u[i+2],
                                        //~ meshes[k]->tmesh->u[i+0],
                                        //~ meshes[k]->tmesh->u[i+1],
                                        
                                        //~ // Color
                                        //~ meshes[k]->tmesh->c[i], 
                                        //~ meshes[k]->tmesh->c[i+1], 
                                        //~ meshes[k]->tmesh->c[i+2], 
                                        //~ meshes[k]->tmesh->c[i+3], 

                                        //~ // Gpu packet
                                        //~ poly4,
                                        //~ &ot[db][*meshes[k]->OTz],
                                        //~ &div4);
                                                
                                    //~ // Increment primitive list pointer
                                    //~ nextpri  += ( (sizeof(POLY_GT4) + 3) / 4 ) * (( 1 << ( div4.ndiv )) << ( div4.ndiv ));
                                    //~ triCount = ((1<<(div4.ndiv))<<(div4.ndiv));
                            
                            //~ } else if (OTc < 48) {
                        
                            // Transparency effect
                            if (*meshes[k]->isPrism){ 
                                
                                
                                // Use current DRAWENV clip as TPAGE
                                ((POLY_GT4 *)poly4)->tpage = getTPage(meshes[k]->tim->mode&0x3, 0,
                                                                     draw[db].clip.x,
                                                                     draw[db].clip.y
                                );
                                
                                //SetShadeTex(poly4, 1);
                                
                                // Use projected coordinates (results from RotAverage...) as UV coords and clamp them to 0-255,0-224 
                                setUV4(poly4, (poly4->x0 < 0? 0 : poly4->x0 > 255? 255 : poly4->x0), 
                                              (poly4->y0 < 0? 0 : poly4->y0 > 224? 224 : poly4->y0), 
                                              (poly4->x1 < 0? 0 : poly4->x1 > 255? 255 : poly4->x1), 
                                              (poly4->y1 < 0? 0 : poly4->y1 > 224? 224 : poly4->y1), 
                                              (poly4->x2 < 0? 0 : poly4->x2 > 255? 255 : poly4->x2), 
                                              (poly4->y2 < 0? 0 : poly4->y2 > 224? 224 : poly4->y2),
                                              (poly4->x3 < 0? 0 : poly4->x3 > 255? 255 : poly4->x3), 
                                              (poly4->y3 < 0? 0 : poly4->y3 > 224? 224 : poly4->y3)
                                              );
                                
             
                            } else {
                                
                                // Use regular TPAGE
                                ((POLY_GT4 *)poly4)->tpage = getTPage(meshes[k]->tim->mode&0x3, 0,
                                                                 meshes[k]->tim->prect->x,
                                                                 meshes[k]->tim->prect->y
                                );
                                
                                // Use model UV coordinates
                                setUV4(poly4, meshes[k]->tmesh->u[i+3].vx, meshes[k]->tmesh->u[i+3].vy   + meshes[k]->tim->prect->y,
                                              meshes[k]->tmesh->u[i+2].vx, meshes[k]->tmesh->u[i+2].vy + meshes[k]->tim->prect->y,
                                              meshes[k]->tmesh->u[i+0].vx, meshes[k]->tmesh->u[i+0].vy + meshes[k]->tim->prect->y,
                                              meshes[k]->tmesh->u[i+1].vx, meshes[k]->tmesh->u[i+1].vy + meshes[k]->tim->prect->y);
                            

                            }
                            
                            
                                // If tim mode  == 0 | 1, set CLUT coordinates
                                if ((meshes[k]->tim->mode & 0x3) < 2){
                                    setClut(poly,             
                                            meshes[k]->tim->crect->x,
                                            meshes[k]->tim->crect->y);
                                }
                                CVECTOR outCol  = {128,128,128,0};
                                CVECTOR outCol1 = {128,128,128,0};
                                CVECTOR outCol2 = {128,128,128,0};
                                CVECTOR outCol3 = {128,128,128,0};

                                NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.pad ]  , &meshes[k]->tmesh->c[ meshes[k]->index[t].order.pad ], *meshes[k]->p, &outCol);
                                NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vz ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vz ], *meshes[k]->p, &outCol1);
                                NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vx ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vx ], *meshes[k]->p, &outCol2);
                                NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vy ], &meshes[k]->tmesh->c[  meshes[k]->index[t].order.vy ], *meshes[k]->p, &outCol3);

                            if (*meshes[k]->isPrism){ 
                                
                                // Use un-interpolated (i.e: no light, no fog) colors
                                setRGB0(poly4, meshes[k]->tmesh->c[i].r, meshes[k]->tmesh->c[i].g, meshes[k]->tmesh->c[i].b);
                                setRGB1(poly4, meshes[k]->tmesh->c[i+1].r, meshes[k]->tmesh->c[i+1].g, meshes[k]->tmesh->c[i+1].b);
                                setRGB2(poly4, meshes[k]->tmesh->c[i+2].r, meshes[k]->tmesh->c[i+2].g, meshes[k]->tmesh->c[i+2].b);
                                setRGB3(poly4, meshes[k]->tmesh->c[i+3].r, meshes[k]->tmesh->c[i+3].g, meshes[k]->tmesh->c[i+3].b);
                            
                            } else {
                                
                                setRGB0(poly4, outCol.r, outCol.g  , outCol.b);
                                setRGB1(poly4, outCol1.r, outCol1.g, outCol1.b);
                                setRGB2(poly4, outCol2.r, outCol2.g, outCol2.b);
                                setRGB3(poly4, outCol3.r, outCol3.g, outCol3.b);
                            } 
                                   
                            if ((*meshes[k]->OTz > 0) && (*meshes[k]->OTz < OTLEN) && (*meshes[k]->p < 4096)){
                                AddPrim(&ot[db][*meshes[k]->OTz-3], poly4);        // OTz - 2
                            }
                            
                            nextpri += sizeof(POLY_GT4);
                            
                            //~ }    
                        }
                    
                    t+=1;

                    }
                }
            }
            
            // Find and apply light rotation matrix
            RotMatrix(&lgtang, &rotlgt);	
            MulMatrix0(&lgtmat, &rotlgt, &light);
            SetLightMatrix(&light);

            applyCamera(&camera);
        
        }
        
        
        // Add secondary OT to main OT
        AddPrims(otdisc[db], ot[db] + OTLEN - 1, ot[db]);

        FntPrint("CurNode : %x\nIndex: %d", curNode, curNode->siblings->index);
        
        //~ FntPrint("Time    : %d %d dt :%d\n",time, atime, dt);
        //~ FntPrint("CamMode: %d Slowmo : %d\nTricount: %d OTz: %d\nOTc: %d, p: %d\n", camMode, actorPtr->anim->interpolate, triCount, *meshes[9]->OTz, OTc, *meshes[9]->p);
        //~ FntPrint("Fy: %d Vy:%d\n", actorPtr->body->gForce.vy, actorPtr->body->velocity.vy );
        //~ FntPrint("Vy: %4d\n", actorPtr->body->gForce.vy );
        //~ FntPrint("%d %d %d", meshes[0]->tim->mode & 0x3, meshes[0]->tim->crect->x, meshes[0]->tim->crect->y);
        
        //~ FntPrint("%d %d %d %d\n", getVectorTo(InvCamPos, *actorPtr->pos));
        //~ FntPrint("Tst : %d %d %d %d\n", getVectorTo(fVecActor, *actorPtr->pos));
        //~ FntPrint("Cc %d Sc %d\n", ncos(camera.rot.vy), nsin(camera.rot.vy));
        
        //~ FntPrint("CRot: %d\n", camera.rot.vy );
        //~ FntPrint("AcRot: %d %d\n", actorPtr->rot->vy, angle);
        
        //~ FntPrint("Cam pos: %d %d\n", -camera.pos.vx, -camera.pos.vz );
        //~ FntPrint("Ac  pos: %d %d\n", actorPtr->pos->vx, actorPtr->pos->vz );
        //~ FntPrint("fVec  pos: %d %d\n", fVecActor.vx, fVecActor.vz);
        //~ FntPrint("pos2cam: %d %d \n", posToCam.vx, posToCam.vz );
        //~ FntPrint("ang2cam: %d %d", objAngleToCam.vy, objAngleToCam.vx);
        
        FntFlush(-1);
		
		display();

        //~ frame = VSync(-1);

	}
    return 0;
}

void init(){
    // Reset the GPU before doing anything and the controller
	PadInit(0);
	ResetGraph(0);
	
	// Initialize and setup the GTE
	InitGeom();
	SetGeomOffset(CENTERX, CENTERY);        // x, y offset
	SetGeomScreen(CENTERX);                 // Distance between eye and screen  
	
    	// Set the display and draw environments
	SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
	SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    
	SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
	SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    
    if (VMODE)
    {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }
	
    setRGB0(&draw[0], BGc.r, BGc.g, BGc.b);
    setRGB0(&draw[1], BGc.r, BGc.g, BGc.b);

    draw[0].isbg = 1;
    draw[1].isbg = 1;

    PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);
		
	// Init font system
	FntLoad(FNT_POS_X, FNT_POS_Y);
	FntOpen(16, 180, 240, 96, 0, 512);
	
    }

void display(void){
    
    DrawSync(0);
    vs = VSync(0);

    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    SetDispMask(1);
    
    // Main OT
    DrawOTag(otdisc[db] + OT2LEN - 1);
    
    // Secondary OT
    //~ DrawOTag(ot[db] + OTLEN - 1);
    
    db = !db;

    nextpri = primbuff[db];
    
        
    }

// Nic's function
void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance) {
    
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj    
    *x = (actorX << 12) + (distance * nsin(angle));
    *z = (actorZ << 12) - (distance * ncos(angle));

}
// @Will : you might want to use sin/cos to move the camera in a circle but you could do that by moving it along it’s tangent and then clamping the distance

void applyCamera(CAMERA * cam){
    VECTOR vec;                                         // Vector that holds the output values of the following instructions

    RotMatrix_gte(&cam->rot, &cam->mat);                // Convert rotation angle in psx units (360° == 4096) to rotation matrix)
    
    ApplyMatrixLV(&cam->mat, &cam->pos, &vec);          // Multiply matrix by vector pos and output to vec

    TransMatrix(&cam->mat, &vec);                       // Apply transform vector
    
    SetRotMatrix(&cam->mat);                            // Set Rotation matrix
    SetTransMatrix(&cam->mat);                          // Set Transform matrix
    
   
}

void setCameraPos(VECTOR pos, SVECTOR rot){
    camera.pos =  pos;
    camera.rot =  rot;
    
    };

void LoadTexture(u_long * tim, TIM_IMAGE * tparam){     // This part is from Lameguy64's tutorial series : lameguy64.net/svn/pstutorials/chapter1/3-textures.html login/pw: annoyingmous
		OpenTIM(tim);                                   // Open the tim binary data, feed it the address of the data in memory
		ReadTIM(tparam);                                // This read the header of the TIM data and sets the corresponding members of the TIM_IMAGE structure
		
        LoadImage(tparam->prect, tparam->paddr);        // Transfer the data from memory to VRAM at position prect.x, prect.y
		DrawSync(0);                                    // Wait for the drawing to end
		
		if (tparam->mode & 0x8){ // check 4th bit       // If 4th bit == 1, TIM has a CLUT
			LoadImage(tparam->crect, tparam->caddr);    // Load it to VRAM at position crect.x, crect.y
			DrawSync(0);                                // Wait for drawing to end
	}

}   


int lerp(int start, int end, int factor){
    // lerp interpolated cam movement
    // InBetween = Value 1 + ( ( Value2 - Value1 ) * lerpValue ) ;
    // lerpValue should be a float between 0 and 1.
    // easeIn
    return ( start ) + (( end - start ) * factor ) >> 12;

    }
long long easeIn(long long i, int div){
    return ((i << 7) * (i << 7) * (i << 7) / div ) >> 19;
    //~ ((i << 7) * (i << 7) * (i << 7) / div ) >> 19
}

int easeOut(int i){
    return (4096 >> 7) - ((4096 - (i << 7)) * (4096 - (i << 7))) >> 12;
    }
    
int easeInOut(int i, int div){
    return lerp(easeIn(i, div), easeOut(i) , i);
    }
    
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor){
    SVECTOR output = {0,0,0,0};
    output.vx = lerp(start.vx, end.vx, factor);
    output.vy = lerp(start.vy, end.vy, factor);
    output.vz = lerp(start.vz, end.vz, factor);
    return output;
    }
    
VECTOR getVectorTo(VECTOR actor, VECTOR target){
    
    VECTOR direction = { subVector(target, actor) };
    VECTOR Ndirection = {0,0,0,0};
    //~ VECTOR distance = {0};
    
    //~ copyVector(&distance, &direction);
    //~ applyVector(&distance, distance.vx, distance.vy, distance.vz, *=);
    
    u_int distSq = (direction.vx * direction.vx) + (direction.vz * direction.vz); // + distance.vy;
    
    direction.pad = psqrt(distSq);
    
    VectorNormal(&direction, &Ndirection);
    //~ direction.pad = csqrt(distSq);
    
    //~ FntPrint("%d ", distSq);
    
    return Ndirection ;
    }
    
int alignAxisToVect(VECTOR target, short axis, int factor){
    
    }



VECTOR getIntCollision(BODY one, BODY two){
    
    VECTOR d1, d2, col;
    short correction = 50;
    
    d1.vx = (one.position.vx + one.max.vx) - (two.position.vx + two.min.vx);
    d1.vy = (one.position.vy + one.max.vy) - (two.position.vy + two.min.vy);
    d1.vz = (one.position.vz + one.max.vz) - (two.position.vz + two.min.vz);
    
    d2.vx = (two.position.vx + two.max.vx) - (one.position.vx - one.max.vx);
    d2.vy = (two.position.vy + two.max.vy) - (one.position.vy + one.min.vy);
    d2.vz = (two.position.vz + two.max.vz) - (one.position.vz - one.max.vz);

    col.vx = !(d1.vx > 0 && d2.vx > 0);
    col.vy = d1.vy > 0 && d2.vy > 0;
    col.vz = !(d1.vz > 0 && d2.vz > 0);
        
    return col;

    }
    
VECTOR getExtCollision(BODY one, BODY two){
    
    VECTOR d1, d2, col;
    
    d1.vx = (one.position.vx + one.max.vx) - (two.position.vx + two.min.vx);
    d1.vy = (one.position.vy + one.max.vy) - (two.position.vy + two.min.vy);
    d1.vz = (one.position.vz + one.max.vz) - (two.position.vz + two.min.vz);
    
    d2.vx = (two.position.vx + two.max.vx) - (one.position.vx + one.min.vx);
    d2.vy = (two.position.vy + two.max.vy) - (one.position.vy + one.min.vy);
    d2.vz = (two.position.vz + two.max.vz) - (one.position.vz + one.min.vz);

    col.vx = d1.vx > 0 && d2.vx > 0;
    col.vy = d1.vy > 0 && d2.vy > 0;
    col.vz = d1.vz > 0 && d2.vz > 0;
        
    return col;

    }

void applyAcceleration(BODY * actor){
    
    short dt = 1;

    VECTOR acceleration = {actor->invMass * actor->gForce.vx ,  (actor->invMass * actor->gForce.vy) + (gravity * ONE), actor->invMass * actor->gForce.vz};
    
    //~ FntPrint("acc: %d %d %d\n", acceleration.vx, acceleration.vy, acceleration.vz );
    
    actor->velocity.vx += (acceleration.vx * dt) >> 12;
    actor->velocity.vy += (acceleration.vy * dt) >> 12;
    actor->velocity.vz += (acceleration.vz * dt) >> 12;
    
    //~ FntPrint("acc: %d %d %d\n", acceleration.vx / ONE, acceleration.vy / ONE, acceleration.vz / ONE );
    
    actor->position.vx += (actor->velocity.vx * dt);
    actor->position.vy += (actor->velocity.vy * dt);
    actor->position.vz += (actor->velocity.vz * dt);
    
    //~ FntPrint("vel: %d %d %d\n", actor->velocity.vx, actor->velocity.vy, actor->velocity.vz );

    
    }

//~ // https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331

void ResolveCollision( BODY * one, BODY * two ){
  
  //~ FntPrint("rv: %d, %d, %d\n", one->velocity.vx, one->velocity.vy, one->velocity.vz);

  
  // Calculate relative velocity
  VECTOR rv = { subVector( one->velocity, two->velocity) };
  
  //~ FntPrint("rv: %d, %d, %d\n", rv.vx,rv.vy,rv.vz);
  
  // Collision normal
  VECTOR normal = { subVector( two->position, one->position ) };
  
  // Normalize collision normal
  normal.vx = normal.vx > 0 ? 1 : normal.vx < 0 ? -1 : 0 ;
  normal.vy = normal.vy > 256 ? 1 : normal.vy < -256 ? -1 : 0 ;
  normal.vz = normal.vz > 0 ? 1 : normal.vz < 0 ? -1 : 0 ;
  
  //~ FntPrint("norm: %d, %d, %d\n", normal.vx,normal.vy,normal.vz);
  
  // Calculate relative velocity in terms of the normal direction
  long velAlongNormal = dotProduct( rv, normal );
 
  //~ FntPrint("velN: %d\n", velAlongNormal);
 
  // Do not resolve if velocities are separating
  if(velAlongNormal > 0)
    return;
 
  // Calculate restitution
  long e = min( one->restitution, two->restitution );
  
  //~ FntPrint("e: %d\n", e);
  
  //~ // Calculate impulse scalar
  long j = -(1 + e) * velAlongNormal * ONE;
  j /= one->invMass + two->invMass;
  //~ j /= ONE;
 
  //~ FntPrint("j: %d\n", j);
  
  // Apply impulse
  applyVector(&normal, j, j, j, *=);
  
  //~ FntPrint("Cnormal %d %d %d\n",normal.vx,normal.vy,normal.vz); 
  
  VECTOR velOne = normal;
  VECTOR velTwo = normal;
  
  applyVector(&velOne,one->invMass,one->invMass,one->invMass, *=);
  applyVector(&velTwo,two->invMass,two->invMass,two->invMass, *=);
  
  //~ FntPrint("V1 %d %d %d\n", velOne.vx/4096,velOne.vy/4096,velOne.vz/4096);
  //~ FntPrint("V2 %d %d %d\n", velTwo.vx/4096,velTwo.vy/4096,velTwo.vz/4096);
  
  applyVector(&one->velocity, velOne.vx/4096/4096, velOne.vy/4096/4096, velOne.vz/4096/4096, +=);
  applyVector(&two->velocity, velTwo.vx/4096/4096, velTwo.vy/4096/4096, velTwo.vz/4096/4096, -=);

  //~ FntPrint("V1 %d %d %d\n", velOne.vx/4096/4096,velOne.vy/4096/4096,velOne.vz/4096/4096);
  //~ FntPrint("V2 %d %d %d\n", velTwo.vx/4096/4096,velTwo.vy/4096/4096,velTwo.vz/4096/4096);

}

VECTOR angularMom(BODY body){
    
    // L = r * p
    // p = m * v  
    VECTOR w = {0,0,0,0};
    
    int r = (body.max.vx - body.min.vx) >> 1;
    
    w.vx = (r * body.mass * body.velocity.vx) >> 2; 
    w.vy = (r * body.mass * body.velocity.vy) >> 2; 
    w.vz = (r * body.mass * body.velocity.vz) >> 2; 
    
    //~ FntPrint("v: %d, r:%d, w:%d\n", body.velocity.vz * r, r * r, w.vz);
    
    return w;
    
    }
    
// From : https://github.com/grumpycoders/pcsx-redux/blob/7438e9995833db5bc1e14da735bbf9dc78300f0b/src/mips/shell/math.h
static inline int32_t dMul(int32_t a, int32_t b) {
    long long r = a;
    r *= b;
    return r >> 24;
}


// standard lerp function
//  s = source, an arbitrary number up to 2^24
//  d = destination, an arbitrary number up to 2^24
//  p = position, a number between 0 and 256, inclusive
//  p = 0 means output = s
//  p = 256 means output = d
static inline uint32_t lerpU(uint32_t start, uint32_t dest, unsigned pos) { return (start * (256 - pos) + dest * pos) >> 8; }

static inline int32_t lerpS(int32_t start, int32_t dest, unsigned pos) { return (start * (256 - pos) + dest * pos) >> 8; }

// start, dest and pos have to be << x, then the result has to be >> x where x defines precision: 
// precision = 2^24 - 2^x
// << x : 0 < pos < precision
// https://discord.com/channels/642647820683444236/646765703143227394/811318550978494505
// my angles are between 0 and 2048 (full circle), so 2^11 for the range of angles; with numbers on a 8.24 representation, a 1.0 angle (or 2pi) means it's 2^24, so to "convert" my angles from 8.24 to my internal discrete cos, I only have to shift by 13

static inline int32_t lerpD(int32_t start, int32_t dest, int32_t pos) { return dMul(start, 16777216 - pos) + dMul(dest, pos); }

static inline long long lerpL(long long start, long long dest, long long pos){  return dMul( (start << 12), 16777216 - (pos << 12) ) + dMul((dest << 12), (pos << 12) ) >> 12; }

// A few notes on the following code :

int ncos(unsigned int t) {
    t %= DC_2PI;
    int r;

    if (t < DC_PI2) {
        r = m_cosTable[t];
    } else if (t < DC_PI) {
        r = -m_cosTable[DC_PI - 1 - t];
    } else if (t < (DC_PI + DC_PI2)) {
        r = -m_cosTable[t - DC_PI];
    } else {
        r = m_cosTable[DC_2PI - 1 - t];
    };

    return r >> 12;
};

// sin(x) = cos(x - pi / 2)
int nsin(unsigned int t) {
    t %= DC_2PI;

    if (t < DC_PI2){
        return ncos(t + DC_2PI - DC_PI2);
    };
    return ncos(t - DC_PI2);
};

// f(n) = cos(n * 2pi / 2048) <- 2048 is == DC_2PI value
// f(n) = 2 * f(1) * f(n - 1) - f(n - 2)
void generateTable(void){
    
    m_cosTable[0] = 16777216;               // 2^24 * cos(0 * 2pi / 2048) => 2^24 * 1 = 2^24 : here, 2^24 defines the precision we want after the decimal point
    static const long long C = 16777137;    // 2^24 * cos(1 * 2pi / 2048) = C = f(1);
    m_cosTable[1] = C;
    
    for (int i = 2; i < 512; i++){
        m_cosTable[i] = ((C * m_cosTable[i - 1]) >> 23) - m_cosTable[i - 2];

        m_cosTable[511] = 0;
    }
};

// https://github.com/Arsunt/TR2Main/blob/411cacb35914c616cb7960c0e677e00c71c7ee88/3dsystem/phd_math.cpp#L432
long long patan(long x, long y){
    long long result;
    int swapBuf;
    int flags = 0;
    
    // if either x or y are 0, return 0
    if( x == 0 && y == 0){
        return 0;
    }

    if( x < 0 ) {
        flags |= 4; 
        x = -x;
        
    }
    
    if ( y < 0 ) {
        flags |= 2;
        y = -y;
    }
    
    if ( y > x ) {
        flags |= 1;
        SWAP(x, y ,swapBuf);
    }
    
    result = AtanBaseTable[flags] + AtanAngleTable[0x800 * y / x];
    if ( result < 0 ){
        result = -result;
        
    return result;
    
}
    
    }

u_int psqrt(u_int n){
    u_int result = 0;
    u_int base = 0x40000000;
    u_int basedResult;
    
    for( ; base != 0; base >>= 2 ) {
        for( ; base != 0; base >>= 2 ) {
            basedResult = base + result;
            result >>= 1;
            
            if( basedResult > n ) {
                break;
            }
            
            n -= basedResult;
            result |= base;
        }
    }
    return result;
}

int cliptest3(short *v1)
{

	if( v1[0]<0 && v1[2]<0 && v1[4]<0 ) return 0;
	if( v1[1]<0 && v1[3]<0 && v1[5]<0 ) return 0;

	if( v1[0] > SCREENXRES && v1[2] > SCREENXRES && v1[4] > SCREENXRES) return 0;
	if( v1[1] > SCREENYRES && v1[3] > SCREENYRES && v1[5] > SCREENYRES) return 0;

	return 1;
}

void callback(){
    
    u_short pad = PadRead(0);
    
    static u_short lastPad;
    
    static short forceApplied = 0;
    
    int div = 4096 >> 7;
    
    static int lerpValues[4096 >> 7];
    
    static short cursor = 0;
    
    static short curCamAngle = 0;
    
    if(!lerpValues[0]){
        for ( long long i = 0; i < div ; i++ ){
        // lerp
        //~ lerpValues[15-i] = lerp(-24, -224, (i << 8));
        // lerp with easeOut : replace factor (i << 8) with (frame/duration)² : in fixed point math : ((i << 8) * (i << 8) / (4096 >> 8)) >> 8
        lerpValues[(div-1)-i] = lerp(-24, -264, easeIn(i, div));
        
        //~ FntPrint("%d, ", lerpValues[div-1-i] );
        //~ FntPrint("%d , ", ((i << 8) / (4096 >> 8) / (4096 >> 8)));
        //~ FntPrint("1: %d , ", ((i << 8) / (4096 >> 8) * (i << 8) / (4096 >> 8)) >> 4  );
        //~ FntPrint("2: %d , ", ((i << 8) * (i << 8) / (4096 >> 8)) >> 8  );
        }
    }
    
    //~ long long flip = 4096 - (32 << 7);
    //~ return (4096 >> 7) - (flip * flip) >> 12;
    
    //~ FntPrint("%d - ", (4096 >> 7) - ((4096 - (0 << 7)) * (4096 - (0 << 7))) >> 12 );
    //~ FntPrint("%d - ", easeInOut(105, div));
    
    //~ static short cursor = 0;
    
    if(timer){timer--;}    
    if(cursor>0){cursor--;}    

    if (pad & PADR1 && !timer){
        
        if (!camPtr->tim_data){
            if(camMode < 6){ 
                
                    camMode ++;
                    lerping = 0;
                
            } else {
                setCameraPos(camPtr->campos->pos, camPtr->campos->rot);
                camPath.cursor = 0;
                camMode = 0;
                lerping = 0;
            }
            //~ lastPad = pad;
            //~ timer = 10;
            //~ pressed = 1;
        } else {
            if (curCamAngle < 5) {
                curCamAngle++;
                camPtr = camAngles[curCamAngle];
                LoadTexture(camPtr->tim_data, camPtr->BGtim);
            } else {
                curCamAngle = 0;
            }
        }
        lastPad = pad;
        timer = 10;
    }
        
    if (!(pad & PADR1) && lastPad & PADR1){
        //~ pressed = 0;
    }
    
    if (pad & PADL2){
        lgtang.vy += 32;
    }
    if (pad & PADL1){
        lgtang.vz += 32;
    }
    
    if (pad & PADRup && !timer){
        if (*actorPtr->isPrism){
            *actorPtr->isPrism = 0;
        } else {
            *actorPtr->isPrism = 1;
        }
        timer = 10;
        lastPad = pad;
    }
    
    if ( pad & PADRdown && !timer ){
        //~ if (actorPtr->body->gForce.vy >= 0 && actorPtr->body->position.vy >= actorPtr->body->min.vy  ){
                //~ forceApplied -= 150;
        //~ }
        cursor = div - 15;
        timer = 30;
        lastPad = pad;
    }
    
    if ( !(pad & PADRdown) && lastPad & PADRdown){
        //~ lastPad = pad;
    }
    
    if (pad & PADRleft && !timer){
        if (actorPtr->anim->interpolate){
            actorPtr->anim->interpolate = 0;
        } else {
            actorPtr->anim->interpolate = 1;
        }
        timer = 10;
        lastPad = pad;
    }
        
    if (pad & PADLup){
        //~ actorPtr->body->gForce.vz = (10  * ncos(actorPtr->rot->vy)) >> 12 ;
        //~ actorPtr->body->gForce.vx = (10  * nsin(actorPtr->rot->vy)) >> 12 ;
        // Cammode 0 :
        //~ actorPtr->body->gForce.vz = getVectorTo(InvCamPos, *actorPtr->pos).vz >> 8 ;
        //~ actorPtr->body->gForce.vx = getVectorTo(InvCamPos, *actorPtr->pos).vx >> 8 ;
        // Others
        actorPtr->body->gForce.vz = getVectorTo(fVecActor, *actorPtr->pos).vz >> 8 ;
        actorPtr->body->gForce.vx = -getVectorTo(fVecActor, *actorPtr->pos).vx >> 8 ;
        
        lastPad = pad;
    }
    
    if ( !(pad & PADLup) && lastPad & PADLup) {
        actorPtr->body->gForce.vz = 0;
        actorPtr->body->gForce.vx = 0;
    }
    
    if (pad & PADLdown){
        //~ actorPtr->body->gForce.vz = -10 ;
        //~ actorPtr->body->gForce.vx = -10 ;
        // Cammode 0 :
        //~ actorPtr->body->gForce.vz = -getVectorTo(InvCamPos, *actorPtr->pos).vz >> 8 ;
        //~ actorPtr->body->gForce.vx = -getVectorTo(InvCamPos, *actorPtr->pos).vx >> 8 ;
        // Others:
        actorPtr->body->gForce.vz = -getVectorTo(fVecActor, *actorPtr->pos).vz >> 8 ;
        actorPtr->body->gForce.vx = getVectorTo(fVecActor, *actorPtr->pos).vx >> 8 ;
        lastPad = pad;
    }
    
    if ( !(pad & PADLdown) && lastPad & PADLdown) {
        actorPtr->body->gForce.vz = 0;
        actorPtr->body->gForce.vx = 0;
        lastPad = pad;
    }
    
    if (pad & PADLleft){
        //~ actorPtr->rot->vx = 0;
        //~ actorPtr->rot->vz = 0;
        actorPtr->rot->vy -= 32;
        lastPad = pad;

    }
    
    if (pad & PADLright){
        //~ actorPtr->rot->vx = 0;
        //~ actorPtr->rot->vz = 0;
        actorPtr->rot->vy += 32;
        lastPad = pad;
    }
    
    //~ actorPtr->body->gForce.vy = forceApplied;
    //~ if (actorPtr->body->gForce.vy < 0){
        //~ forceApplied += gravity;
    //~ }
    
    if (cursor){
        actorPtr->body->position.vy = lerpValues[cursor];}
    
    //~ FntPrint("Mode : %d  Angle: %d\n", camMode, curCamAngle);
    
    //~ FntPrint("Curs: %d Vy: %d\n", cursor, actorPtr->body->position.vy );
    //~ FntPrint("C %d S %d\n", ncos(actorPtr->rot->vy), nsin(actorPtr->rot->vy));
    //~ FntPrint("%d\n", !(pad & PADRdown) && lastPad & PADRdown);
    //~ FntPrint("sin: %d cos:%d\n", nsin(actorPtr->rot->vy), ncos(actorPtr->rot->vy));
    //~ FntPrint("sin: %d cos:%d\n", 10 * nsin(actorPtr->rot->vy) >> 12, 10 * ncos(actorPtr->rot->vy) >> 12);
}
