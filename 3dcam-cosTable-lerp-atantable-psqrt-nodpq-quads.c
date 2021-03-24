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

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>
#include <stdio.h>

// Precalculated sin/cos values
//~ #include "psin.c"
//~ #include "pcos.c"
#include "atan.c"

// Sample vector model
#include "coridor.c"
//~ #include "gnd.c"

#define VMODE       0

#define SCREENXRES 320
#define SCREENYRES 240

#define CENTERX		SCREENXRES/2
#define CENTERY		SCREENYRES/2

#define OTLEN	    256	                    // Maximum number of OT entries
#define PRIMBUFFLEN	2260 * sizeof(POLY_GT3)	    // Maximum number of POLY_GT3 primitives

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

u_long	    ot[2][OTLEN]  = {0};   		        // Ordering table (contains addresses to primitives)
char	primbuff[2][PRIMBUFFLEN] = {0};	        // Primitive list // That's our prim buffer

//~ int		    primcnt=0;			            // Primitive counter

char * nextpri = primbuff[0];			        // Primitive counter

char		    db	= 0;                        // Current buffer counter


CVECTOR BGc = {50, 50, 75, 0};
VECTOR BKc = {100, 100, 100, 0};

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


//~ //vertex anim

//~ typedef struct {
    //~ int nframes;    // number of frames e.g   20
    //~ int nvert;      // number of vertices e.g 21
    //~ SVECTOR data[]; // vertex pos as SVECTORs e.g 20 * 21 SVECTORS
    //~ } VANIM;
    
    
//Pad
int pressed = 0;

// Cam stuff 
int camMode = 2;
long timeB = 0;

u_long triCount = 0;

// Prototypes

// Sin/Cos Table
void generateTable(void);
int  ncos(u_int t);
int  nsin(u_int t);

// Atan table
int patan(int x, int y);

//sqrt
u_int psqrt(u_int n);

// PSX setup
void init(void);
void display(void);

// Utils
void LoadTexture(u_long * tim, TIM_IMAGE * tparam);
int cliptest3(short * v1);
int lerp(int start, int end, int factor); // FIXME : not working as it should
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor); // FIXME 

// Camera 
void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance);
void applyCamera(CAMERA * cam);
void setCameraPos(VECTOR pos, SVECTOR rot);

// Physics
VECTOR getIntCollision(BODY one, BODY two);
VECTOR getExtCollision(BODY one, BODY two);
void ResolveCollision( BODY * one, BODY * two );


void applyAcceleration(BODY * actor);

void callback();

int main() {
	
    // Mesh stuff
    	
	int		i;
	long	t, p, OTz, OTc, Flag, nclip;                // t == vertex count, p == depth cueing interpolation value, OTz ==  value to create Z-ordered OT, Flag == see LibOver47.pdf, p.143
    POLY_GT3 * poly;                        
    
    // Poly subdiv
    DIVPOLYGON3	div = { 0 };
    div.pih = SCREENXRES;
	div.piv = SCREENYRES;

    //~ CVECTOR outCol ={0,0,0,0};
    //~ CVECTOR outCol1 ={0,0,0,0};
    //~ CVECTOR outCol2 ={0,0,0,0};
    
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
    
    // physics
    short physics = 1;
    long time = 0;

    long dt;

    VECTOR col_lvl, col_sphere = {0};
    
    // Actor start pos
    
    //~ modelobject_body.position.vx = modelobject_pos.vx = 50;

    // Cam stuff 
    
    VECTOR posToActor  = {0, 0, 0, 0};      // position of camera relative to actor    
    VECTOR theta       = {0, 0, 0, 0};      // rotation angles for the camera to point at actor
    
    int angle     = 0;                      //PSX units = 4096 == 360° = 2Pi
    int dist      = 0;                      //PSX units 

    int lerping    = 0;
    
    // Vertex anim
    
    //~ SVECTOR interpCache[5];
    SVECTOR a,b,c = {0,0,0,0};

    short timediv = 1;

    int atime = 0;
    
    for (int k = 0; k < sizeof(meshes)/sizeof(meshes[0]); k++){
            triCount += meshes[k]->tmesh->len;
    }
    
	// Main loop
	while (1) {
        //~ timeB = time;
        time ++;
        
        timediv = 2;
        
        if (time % timediv == 0){
            atime ++;
        }
                
        //~ timediv = 1;


        
        //~ // Physics
        //~ if (time%2 == 0){
            

            // using libgte ratan (slower)
            //~ theta.vy = -ratan2(posToActor.vx, posToActor.vz) ;
            //~ theta.vx = 1024 - ratan2(dist, posToActor.vy);

            // using atantable (faster)
            theta.vy = patan(posToActor.vx, posToActor.vz) / 16 - 1024 ;
            theta.vx = patan(dist, posToActor.vy)/16;
                        
            if(camMode != 2){
                
                camera.rot.vy = theta.vy;
                // using csin/ccos, no need for theta
                //~ camera.rot.vy = angle; 
                camera.rot.vx = theta.vx;   
            
            }
            
            if(camMode != 4){
                lerping = 0;
                }
            
            if(camMode == 0){                       // Camera follows actor with lerp for rotations
                dist = 150;
                camera.pos.vx = -(camera.x/ONE);
                //~ camera.pos.vy = -(camera.y/ONE);
                camera.pos.vz = -(camera.z/ONE);
                
                getCameraXZ(&camera.x, &camera.z, modelobject_pos.vx, modelobject_pos.vz, angle, dist);

                angle += lerp(camera.rot.vy, modelobject_rot.vy, 128);
                
            }
            
            if (camMode == 1){                      // mode 1 : Camera rotates continuously
                
                dist = 150;
                camera.pos.vx = -(camera.x/ONE);
                //~ camera.pos.vy = -(camera.y/ONE);
                camera.pos.vz = -(camera.z/ONE);
                                                    
                getCameraXZ(&camera.x, &camera.z, modelobject_pos.vx, modelobject_pos.vz, angle, dist);
                angle += 10;
            }
            
            if (camMode == 3){                              // mode 3 : Fixed Camera with actor tracking
                
                // Using libgte sqrt ( slower)
                //~ dist = SquareRoot0( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
                
                // Using precalc sqrt
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
                
                camera.pos.vx = 290;
                camera.pos.vz = 100;
                camera.pos.vy = 180;
            }
            
            if (camMode == 2){                              // mode 2 : Fixed Camera
                
                setCameraPos(camStartPos.pos, camStartPos.rot);

            }
            
            if(camMode == 4){                               // Flyby mode from camStart to camEnd
                
                if (!lerping){
                    // Set cam start position
                
                    camera.pos.vx = camPath.points[camPath.cursor].vx;
                    camera.pos.vy = camPath.points[camPath.cursor].vy;
                    camera.pos.vz = camPath.points[camPath.cursor].vz;
                    
                    lerping = 1;
                    }
                    
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
                
                short r = camPath.points[camPath.cursor+1].vx -  camera.pos.vx;
                short s = camPath.points[camPath.cursor+1].vy -  camera.pos.vy;
                short t = camPath.points[camPath.cursor+1].vz -  camera.pos.vz;
                
                // FIXME : the lerp function is incorrect
                //~ camera.pos.vx += lerp(camPath.points[camPath.cursor].vx, camPath.points[camPath.cursor+1].vx, 64);
                //~ camera.pos.vy += lerp(camPath.points[camPath.cursor].vy, camPath.points[camPath.cursor+1].vy, 64);
                //~ camera.pos.vz += lerp(camPath.points[camPath.cursor].vz, camPath.points[camPath.cursor+1].vz, 64);
                
                // easeOut 
                camera.pos.vx += lerp(camera.pos.vx, camPath.points[camPath.cursor+1].vx, 128);
                camera.pos.vy += lerp(camera.pos.vy, camPath.points[camPath.cursor+1].vy, 128);
                camera.pos.vz += lerp(camera.pos.vz, camPath.points[camPath.cursor+1].vz, 128);
                
                //~ if ( camera.pos.vx  <= camPath.points[camPath.cursor+1].vx ||
                     //~ camera.pos.vy  >= camPath.points[camPath.cursor+1].vy ||
                     //~ camera.pos.vz  <= camPath.points[camPath.cursor+1].vz){
                        //~ camPath.cursor ++;
                //~ }
                
                if ( camera.pos.vx + r == camPath.points[camPath.cursor+1].vx &&
                     camera.pos.vy + s == camPath.points[camPath.cursor+1].vy &&
                     camera.pos.vz + t == camPath.points[camPath.cursor+1].vz){
                        camPath.cursor ++;
                }
                
                if ( camPath.cursor == camPath.len - 1 ){
                    lerping = 0;
                    camPath.cursor = 0;
                }
                
            
            }
            
            
           //~ dt = time/180+1 - time/180;
        if (physics){
            if(time%1 == 0){
                for ( int k = 0; k < sizeof(meshes)/sizeof(meshes[0]);k ++){
                    
                    if ( *meshes[k]->isRigidBody == 1 ) {

                        applyAcceleration(meshes[k]->body);
                        
                        //~ VECTOR col_lvl, col_sphere = {0};
                        
                        // Get col with level ( modelgnd_body )
                        col_lvl = getIntCollision( *meshes[k]->body , modelgnd_body );
                        
                        col_sphere = getExtCollision( modelobject_body, modelSphere_body );
                        
                        // If !col, keep moving
                        
                        if ( !col_lvl.vx ){ meshes[k]->pos->vx = meshes[k]->body->position.vx; } 
                        
                        if ( !col_lvl.vy ){ meshes[k]->pos->vy = meshes[k]->body->position.vy; } // FIXME : Why the 15px offset ? 
                        
                        if ( !col_lvl.vz ){ meshes[k]->pos->vz = meshes[k]->body->position.vz; }
                                               
                        // If col with wall, change direction
                        
                        if ( col_lvl.vx ) { meshes[k]->body->gForce.vx *= -1; }

                        if ( col_lvl.vy ) { 
                            //~ meshes[k]->body->gForce.vy *= -1; 

                        }
                        
                        if ( col_lvl.vz ) { meshes[k]->body->gForce.vz *= -1; }
                        
                        // If col, reset velocity
                        
                        //~ if ( col_lvl.vx ||
                             //~ col_lvl.vy ||
                             //~ col_lvl.vz
                           //~ ) {
                            //~ meshes[k]->body->velocity.vy = meshes[k]->body->velocity.vz = 0;
                        //~ }
                        
                        ResolveCollision( &modelobject_body, &modelSphere_body);
                        //~ FntPrint("Vel: %d\n", modelSphere_body.velocity.vx);
                        if (col_sphere.vx){
                            
                            int w = (ONE / (( modelSphere_body.velocity.vx * ONE ) / ( (modelSphere_body.max.vx - modelSphere_body.min.vx) / 2 ))) ; 
                            
                            if (modelSphere_body.velocity.vx){
                                
                                //~ int w = (ONE / (( modelSphere_body.velocity.vx * ONE ) / ( (modelSphere_body.max.vx - modelSphere_body.min.vx) / 2 ))) * modelSphere_body.velocity.vx ; 
                                //~ FntPrint("W %d\n",w);
                                FntPrint("Vel %d\n",modelSphere_body.velocity.vx);
                                
                                modelSphere_rot.vz += w;
                            
                                //~ if ( col_sphere.vx ) {
                                    //~ meshes[k]->body->gForce.vx *= -1;
                                //modelSphere_body.gForce.vx = -meshes[k]->body->gForce.vx/4;                         //~ ResolveCollision(&modelobject_body, &modelSphere_body);
                                //~ }
                            }
                        }
                        
                        if (!col_sphere.vx){
                            modelSphere_body.velocity.vx = 0;
                            }

                        //~ if (w && !modelSphere_body.velocity.vx)
                        //~ {
                            //~ FntPrint("W %d\n",w);
                            //~ w --;
                        //~ }
                        
                        
                        //~ if ( col_sphere.vz ) { meshes[k]->body->gForce.vz *= -1; }
                        //~ if ( col_sphere.vy ) { meshes[k]->body->gForce.vy *= -1; }
                        
                        //~ if (modelSphere_body.gForce.vx){modelSphere_body.gForce.vx -= 5;}
                        meshes[k]->pos->vx = meshes[k]->body->position.vx;
                        //~ meshes[k]->pos->vy = meshes[k]->body->position.vy ;
                        meshes[k]->pos->vz = meshes[k]->body->position.vz;
                        
                        
                    }
                    meshes[k]->body->velocity.vy = meshes[k]->body->velocity.vx = meshes[k]->body->velocity.vz = 0;

                }
            }
        }
        // Camera setup 
        

        // position of cam relative to actor
        posToActor.vx = modelobject_pos.vx + camera.pos.vx;
        posToActor.vz = modelobject_pos.vz + camera.pos.vz;
        posToActor.vy = modelobject_pos.vy + camera.pos.vy;
        
        // find dist between actor and cam
        //~ dist = csqrt((posToActor.vx * posToActor.vx * 4096) + (posToActor.vz * posToActor.vz * 4096));
        //~ dist = SquareRoot0( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
        
        // find angles between cam and actor
        //~ theta.vy = ratan2(posToActor.vx, posToActor.vz);
        //~ theta.vx = 1024 - ratan2(dist, posToActor.vy);
        
        //~ camera.rot.vy = - theta.vy;
        // using csin/ccos, no need for theta
        // camera.rot.vy = angle; 
        //~ camera.rot.vx = theta.vx;   

        //~ applyCamera(&camera);
        
		// Clear the current OT
		ClearOTagR(ot[db], OTLEN);



		for (int k = 0; k < sizeof(meshes)/sizeof(meshes[0]); k++){
        
            // Render the sample vector model
            t=0;
            
            // If rigidbdy, apply rot/transform matrix
            if (*meshes[k]->isRigidBody){
                                        
                    //~ PushMatrix();                                         // Push current matrix on the stack (real slow -> dma transfer )
                    
                    RotMatrix_gte(meshes[k]->rot, meshes[k]->mat);            // Apply rotation matrix
                                                            
                    TransMatrix(meshes[k]->mat, meshes[k]->pos);              // Apply translation matrix
                    
                    CompMatrix(&camera.mat, meshes[k]->mat, meshes[k]->mat);  // Was using &PolyMatrix instead of meshes[k]->mat 

                    SetRotMatrix(meshes[k]->mat);                             // Set default rotation matrix - Was using &PolyMatrix instead of meshes[k]->mat
                    SetTransMatrix(meshes[k]->mat);                           // Was using &PolyMatrix instead of meshes[k]->mat
                    
            }

            
                                        
            // modelCube is a TMESH, len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
            if (meshes[k]->index[t].code == 4) {
                
                for (i = 0; i < (meshes[k]->tmesh->len * 3); i += 3) {               
                    
                    poly = (POLY_GT3 *)nextpri;
                    
                       // If Vertex Anim flag 
                    if (*meshes[k]->isAnim){
                        
                    // FIXME : SLERP VERTEX ANIM
                        
                        //~ SVECTOR a,b,c = {0,0,0,0};
                        
                        //~ for (int f = 0; f < 5; f++){
                            //~ interpCache[f] = SVlerp( (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]], (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]], 2048);
                            //~ interpCache[f+1] = SVlerp( (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]], (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]], 2048);
                            //~ interpCache[f+2] = SVlerp( (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]], (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]], 2048);
                           
                        //~ }
                        
                        //~ SVECTOR start = meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]];
                        
                        //~ SVECTOR end = meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]];
                        //~ if (a.vx != 0 && b.vx != 0 && c.vx != 0){
                            //~ SVECTOR d,e,f;
                            
                            //~ d = SVlerp( (SVECTOR) a, (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]], 2048);
                            //~ e = SVlerp( (SVECTOR) b, (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t+1]], 2048);
                            //~ f = SVlerp( (SVECTOR) c, (SVECTOR) meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t+2]], 2048);

                            //~ addVector( &a , &d );
                            //~ addVector( &b , &e );
                            //~ addVector( &c , &f );
                        //~ } else {
                            //~ a = (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]];
                            //~ b = (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t+1]];
                            //~ c = (SVECTOR) meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t+2]];
                            //~ }
                        //~ a.vx = lerp(start.vx, end.vx, 2048);
                        //~ a.vy = lerp(meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]].vy, meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]].vy, 2048);
                        //~ a.vz = lerp(meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]].vz, meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]].vz, 2048);

                        //~ b = meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t+1]];
                        //~ c = meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t+2]];
                        //~ SVlerp(meshes[k]->anim->data[ 0 * modelCylindre_anim.nvert + meshes[k]->index[t]],   meshes[k]->anim->data[ 10 * modelCylindre_anim.nvert + meshes[k]->index[t]],64, a);
                        //~ SVlerp(meshes[k]->anim->data[ 0 * modelCylindre_anim.nvert + meshes[k]->index[t+1]], meshes[k]->anim->data[ 10 * modelCylindre_anim.nvert + meshes[k]->index[t+1]],64, b);
                        //~ SVlerp(meshes[k]->anim->data[ 0 * modelCylindre_anim.nvert + meshes[k]->index[t+2]], meshes[k]->anim->data[ 10 * modelCylindre_anim.nvert + meshes[k]->index[t+2]],64, c);
                        
                        //~ FntPrint("%d %d %d\n", meshes[k]->anim->data[0 * modelCylindre_anim.nvert + meshes[k]->index[t]].vz, meshes[k]->anim->data[10 * modelCylindre_anim.nvert + meshes[k]->index[t]].vz, a.vz);
                        
                        //~ FntPrint("%d %d %d\n", c.vx, c.vy, c.vz);
                        
                        //~ FntPrint("%d %d %d\n", a.vx, b.vx, c.vx);
                        
                    // Rotate, translate, and project the vectors and output the results into a primitive

                        //~ OTz  = RotTransPers(&meshes[k]->tmesh->v[meshes[k]->index[t]]  , (long*)&poly->x0, meshes[k]->p, &Flag);                
                        //~ OTz += RotTransPers(&meshes[k]->tmesh->v[meshes[k]->index[t+1]], (long*)&poly->x1, meshes[k]->p, &Flag);
                        //~ OTz += RotTransPers(&meshes[k]->tmesh->v[meshes[k]->index[t+2]], (long*)&poly->x2, meshes[k]->p, &Flag);                                    
                        
                    // Use anim vertex's positions
                         
                        //~ nclip = RotAverageNclip3(
                            //~ &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t]],
                            //~ &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+2]],
                            //~ &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+1]],
                            //~ (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                            //~ meshes[k]->p,
                            //~ &OTz,
                            //~ &Flag
                        //~ );
                        
                    // Use anim vertex's positions
                         
                        nclip = RotAverageNclip3(
                            &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t].order.vx],
                            &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t].order.vz],
                            &meshes[k]->anim->data[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t].order.vy],
                            (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                            meshes[k]->p,
                            &OTz,
                            &Flag
                        );
                            
                        
                    } else {                        
                    
                        // Use model's regular vertex pos
                        nclip = RotAverageNclip3(
                            &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vx ],  
                            &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vz ],
                            &meshes[k]->tmesh->v[ meshes[k]->index[t].order.vy ],
                            (long*)&poly->x0, (long*)&poly->x1, (long*)&poly->x2,
                            meshes[k]->p,
                            &OTz,
                            &Flag
                            );
                    }
                    
                    //~ FntPrint("%d %d %d %d\n", meshes[k]->index[t].order.vx, meshes[k]->index[t].order.vy, meshes[k]->index[t].order.vz, meshes[k]->index[t].code);
                    
                    if (nclip > 0 && OTz > 0) {

                    
                    SetPolyGT3(poly);
                    
                    // Can use ?
                    //~ RotMeshPrimS_GCT3();

                    if (*meshes[k]->isPrism){ 
                        
                        // Use current DRAWENV clip as TPAGE
                        ((POLY_GT3 *)poly)->tpage = getTPage(meshes[k]->tim->mode&0x3, 0,
                                                             draw[db].clip.x,
                                                             draw[db].clip.y
                        );
                        
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
                        
                        // Use model UV coordinates
                        setUV3(poly,  meshes[k]->tmesh->u[i].vx  , meshes[k]->tmesh->u[i].vy   + meshes[k]->tim->prect->y,
                                      meshes[k]->tmesh->u[i+2].vx, meshes[k]->tmesh->u[i+2].vy + meshes[k]->tim->prect->y,
                                      meshes[k]->tmesh->u[i+1].vx, meshes[k]->tmesh->u[i+1].vy + meshes[k]->tim->prect->y);
                    

                    }
             
                 
                   
                // FIXME : Polygon subdiv 
                    
                    //~ OTc = OTz>>4;
                    
                    //~ if (OTc < 15) {
					
                        //~ if (OTc > 5) div.ndiv = 1; else div.ndiv = 2;
                            
                            //~ DivideGT3(
                                //~ // Vertex coord
                                //~ &meshes[k]->tmesh->v[meshes[k]->index[t]],  
                                //~ &meshes[k]->tmesh->v[meshes[k]->index[t+2]],
                                //~ &meshes[k]->tmesh->v[meshes[k]->index[t+1]],
                                //~ // UV coord
                                //~ meshes[k]->tmesh->u[i],
                                //~ meshes[k]->tmesh->u[i+2],
                                //~ meshes[k]->tmesh->u[i+1],
                                
                                //~ // Color
                                //~ meshes[k]->tmesh->c[i], 
                                //~ meshes[k]->tmesh->c[i+2], 
                                //~ meshes[k]->tmesh->c[i+1], 

                                //~ // Gpu packet
                                //~ poly,
                                //~ &ot[db][OTz],
                                //~ &div);
                                        
                            //~ // Increment primitive list pointer
                            //~ nextpri  += ( (sizeof(POLY_GT3) + 3) / 4 ) * (( 1 << ( div.ndiv )) << ( div.ndiv ));
                            //~ triCount = ((1<<(div.ndiv))<<(div.ndiv));
					
                    //~ }
                    
                // Interpolate a primary color vector and far color 
                
                // If vertex anim has updated normals
                
                    //~ if (*meshes[k]->isAnim){
                        //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t]], &meshes[k]->tmesh->c[meshes[k]->index[t]], *meshes[k]->p, &outCol);
                        //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+1]], &meshes[k]->tmesh->c[meshes[k]->index[t+1]], *meshes[k]->p, &outCol1);
                        //~ NormalColorDpq(&meshes[k]->anim->normals[ atime%19 * modelCylindre_anim.nvert + meshes[k]->index[t+2]], &meshes[k]->tmesh->c[meshes[k]->index[t+2]], *meshes[k]->p, &outCol2);
                    //~ } else {

                        CVECTOR outCol  ={0,0,0,0};
                        CVECTOR outCol1 ={0,0,0,0};
                        CVECTOR outCol2 ={0,0,0,0};

                        //~ NormalColorDpq(&meshes[k]->tmesh->n[meshes[k]->index[t]]  , &meshes[k]->tmesh->c[meshes[k]->index[t]], *meshes[k]->p, &outCol);
                        //~ NormalColorDpq(&meshes[k]->tmesh->n[meshes[k]->index[t+2]], &meshes[k]->tmesh->c[meshes[k]->index[t+2]], *meshes[k]->p, &outCol1);
                        //~ NormalColorDpq(&meshes[k]->tmesh->n[meshes[k]->index[t+1]], &meshes[k]->tmesh->c[meshes[k]->index[t+1]], *meshes[k]->p, &outCol2);
                       
                        NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vx ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vx ], *meshes[k]->p, &outCol);
                        NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vz ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vz ], *meshes[k]->p, &outCol1);
                        NormalColorDpq(&meshes[k]->tmesh->n[ meshes[k]->index[t].order.vy ], &meshes[k]->tmesh->c[ meshes[k]->index[t].order.vy ], *meshes[k]->p, &outCol2);
                    //~ }
                    
                // Other methods 

                    //~ NormalColorDpq3(&meshes[k]->tmesh->n[i],
                                    //~ &meshes[k]->tmesh->n[i+1],
                                    //~ &meshes[k]->tmesh->n[i+2],
                                    //~ &meshes[k]->tmesh->c[i],
                                    //~ *meshes[k]->p,
                                    //~ &outCol,&outCol1,&outCol2
                                    //~ );
                                    
                    //~ DpqColor3(&meshes[k]->tmesh->c[i],
                              //~ &meshes[k]->tmesh->c[i+1],
                              //~ &meshes[k]->tmesh->c[i+2],
                              //~ *meshes[k]->p,
                              //~ &outCol,&outCol1,&outCol2
                            //~ );          

                    if (*meshes[k]->isPrism){ 
                        
                        // Use un-interpolated (i.e: no light, no fog) colors
                        setRGB0(poly, meshes[k]->tmesh->c[i].r, meshes[k]->tmesh->c[i+1].g, meshes[k]->tmesh->c[i+2].b);
                        setRGB1(poly, meshes[k]->tmesh->c[i+1].r, meshes[k]->tmesh->c[i+1].g, meshes[k]->tmesh->c[i+1].b);
                        setRGB2(poly, meshes[k]->tmesh->c[i+2].r, meshes[k]->tmesh->c[i+2].g, meshes[k]->tmesh->c[i+2].b);
                    
                    } else {
                        
                        setRGB0(poly, outCol.r, outCol.g  , outCol.b);
                        setRGB1(poly, outCol1.r, outCol1.g, outCol1.b);
                        setRGB2(poly, outCol2.r, outCol2.g, outCol2.b);
                    } 
                           
                    // Sort the primitive into the OT
                    //~ OTz /= 3;
                    
                    // cliptest3((short *)&meshes[k]->tmesh->v[meshes[k]->index[t]])
                    
                        //~ if ((OTz > 0) && (OTz < OTLEN) && (*meshes[k]->p < 2048)){
                    if ((OTz > 0) && (OTz < OTLEN) && (*meshes[k]->p < 4096)){
                        AddPrim(&ot[db][OTz-2], poly);        // OTz - 2
                    }
                    
                    nextpri += sizeof(POLY_GT3);
                }
                
                t+=1;
            }    
                //~ if (*meshes[k]->isRigidBody){
                //~     PopMatrix();                    // Pull previous matrix from stack (slow)
                //~ }

        }

            // Find and apply light rotation matrix
            RotMatrix(&lgtang, &rotlgt);	
            MulMatrix0(&lgtmat, &rotlgt, &light);
            SetLightMatrix(&light);

            applyCamera(&camera);

        }
        
        //~ FntPrint("ColSphere: %d\n", (modelobject_body.position.vy + modelobject_body.max.vy) - (modelSphere_body.position.vy + modelSphere_body.min.vy) );
        //~ FntPrint("ColSphere: %d\n", (modelSphere_body.position.vy + modelSphere_body.max.vy) - (modelobject_body.position.vy + modelobject_body.min.vy) );
        //~ FntPrint("Col %d\n", col_sphere.vy );
        
        //~ FntPrint("Obj: %d,%d,%d\n",modelobject_body.velocity.vx,modelobject_body.velocity.vy,modelobject_body.velocity.vz);
        //~ FntPrint("Sph: %d,%d,%d\n",modelSphere_body.velocity.vx,modelSphere_body.velocity.vy,modelSphere_body.velocity.vz);
        
        //~ FntPrint("%d, %d\n",modelobject_body.position.vx, modelobject_pos.vx);
        
        //~ FntPrint("Time    : %d %d dt :%d\n",time, atime, dt);
        FntPrint("Tricount: %d OTz: %d\nOTc: %d, p: %d\n",triCount, OTz, OTc, *meshes[2]->p);
        
        //~ FntPrint("Sphr : %4d %4d %4d\n", modelSphere_body.gForce.vx, modelSphere_body.gForce.vy, modelSphere_body.gForce.vz);

        //~ FntPrint("isPrism: %d\n", *meshobject.isPrism);

        //~ FntPrint("L1: %d %d %d\n", light.m[0][0],light.m[0][1],light.m[0][2]);
        //~ FntPrint("L2: %d %d %d\n", light.m[1][0],light.m[1][1],light.m[1][2]);
        //~ FntPrint("L3: %d %d %d\n", light.m[2][0],light.m[2][1],light.m[2][2]);
 
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
	FntLoad(960, 0);
	FntOpen(16, 180, 240, 96, 0, 512);
	
    }

void display(void){
    
    DrawSync(0);
    vs = VSync(0);

    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);

    SetDispMask(1);
    
    DrawOTag(ot[db] + OTLEN - 1);
    
    db = !db;

    nextpri = primbuff[db];
    
        
    }

// Nic's function
void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance) {
    
    
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj
    
    *x = (actorX * ONE) + (distance * nsin(angle));
    *z = (actorZ * ONE) - (distance * ncos(angle));
    
    //~ *x = (actorX * ONE) + (distance * csin(angle));
    //~ *z = (actorZ * ONE) - (distance * ccos(angle)); // Z is pointing away from the eye 
    
    // @soapy https://discord.com/channels/642647820683444236/663664210525290507/797188403748929547
    //~ *x = (actorX * ONE) + (distance * rcossin_tbl[(angle & 0xFFFU) * 2]);
    //~ *z = (actorZ * ONE) - (distance * rcossin_tbl[(angle & 0xFFFU) * 2 + 1]); // Z is pointing away from the eye 
    
    // Using precalculated psin and pcos
    //~ *x = (actorX * ONE) + (distance * psin[angle]);
    //~ *z = (actorZ * ONE) - (distance * pcos[angle]); // Z is pointing away from the eye 
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
    // This'll have to be a fixed point value between 0-4096
    // easeOut
    //~ return ( ( start ) + ( end - start ) * factor ) / 4096;
    // easeIn
    return ( ( start ) + ( end - start ) * factor ) / 4096;

    
    // kinda linear
    //~ return (( start ) + ( end - start )) * factor / 4096;

    }
    
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor){
    SVECTOR output = {0,0,0,0};
    output.vx = lerp(start.vx, end.vx, factor);
    output.vy = lerp(start.vy, end.vy, factor);
    output.vz = lerp(start.vz, end.vz, factor);
    return output;
    }

VECTOR getIntCollision(BODY one, BODY two){
    
    VECTOR d1, d2, col;
    
    d1.vx = (one.position.vx - one.max.vx) - (two.position.vx + two.min.vx);
    d1.vy = (one.position.vy + one.min.vy) - (two.position.vy + two.min.vy);
    d1.vz = (one.position.vz - one.max.vz) - (two.position.vz + two.min.vz);
    
    d2.vx = (two.position.vx + two.max.vx) - (one.position.vx + one.max.vx);
    d2.vy = (two.position.vy + two.max.vy) - (one.position.vy + one.max.vy);
    d2.vz = (two.position.vz + two.max.vz) - (one.position.vz + one.max.vz);

    col.vx = !(d1.vx > 0 && d2.vx > 0);
    col.vy = !(d1.vy > 0 && d2.vy > 0);
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

    VECTOR acceleration = {actor->invMass * actor->gForce.vx ,  actor->invMass * actor->gForce.vy, actor->invMass * actor->gForce.vz};
    
    actor->velocity.vx += (acceleration.vx * dt) / 4096;
    actor->velocity.vy += (acceleration.vy * dt) / 4096;
    actor->velocity.vz += (acceleration.vz * dt) / 4096;
    
    actor->position.vx += (actor->velocity.vx * dt);
    actor->position.vy += (actor->velocity.vy * dt);
    actor->position.vz += (actor->velocity.vz * dt);
    
    }

//~ // https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
void ResolveCollision( BODY * one, BODY * two ){
  
  // Calculate relative velocity
  VECTOR rv = { subVector( one->velocity, two->velocity) };
  
  //~ FntPrint("rv: %d, %d, %d\n", rv.vx,rv.vy,rv.vz);
  
  // Collision normal
  VECTOR normal = { subVector( two->position, one->position ) };
  
  // Normalize collision normal
  normal.vx = normal.vx > 32 ? 1 : normal.vx < -32 ? -1 : 0 ;
  normal.vy = normal.vy > 256 ? 1 : normal.vy < -256 ? -1 : 0 ;
  normal.vz = normal.vz > 32 ? 1 : normal.vz < -32 ? -1 : 0 ;
  
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
  
  //~ FntPrint("V1 %d %d %d\n", velOne.vx/4096/4096,velOne.vy/4096/4096,velOne.vz/4096/4096);
  //~ FntPrint("V2 %d %d %d\n", velTwo.vx/4096/4096,velTwo.vy/4096/4096,velTwo.vz/4096/4096);
  
  applyVector(&one->velocity, velOne.vx/4096/4096, velOne.vy/4096/4096, velOne.vz/4096/4096, -=);
  applyVector(&two->velocity, velTwo.vx/4096/4096, velTwo.vy/4096/4096, velTwo.vz/4096/4096, +=);

  //~ FntPrint("V1 %d %d %d\n", velOne.vx/4096/4096,velOne.vy/4096/4096,velOne.vz/4096/4096);
  //~ FntPrint("V2 %d %d %d\n", velTwo.vx/4096/4096,velTwo.vy/4096/4096,velTwo.vz/4096/4096);

}


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
int patan(int x, int y){
    int result;
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
    
    int pad = PadRead(0);
    
    if (pad & PADRright && !pressed){
        if(camMode < 4){ 
            camMode += 1;
        } else {
            setCameraPos(camStartPos.pos, camStartPos.rot);
            camPath.cursor = 0;
            camMode = 0;
        }
        pressed = 1;
    }
        
    if (!(pad & PADRright)){
        pressed = 0;
    }
    
    if (pad & PADRdown){
        lgtang.vy += 32;
        //~ lgtang.vx += 32;
    }
    if (pad & PADRup){
        lgtang.vz += 32;
        //~ lgtang.vx += 32;
    }
    //~ RotMatrix(&lgtang, &rotlgt);	
    //~ MulMatrix(&rotlgt, &rottrans);
    
    if (pad & PADLdown && !pressed){
        if (*meshobject.isPrism){
            *meshobject.isPrism = 0;
        } else {
            *meshobject.isPrism = 1;
        }
        pressed = 1;
    }
    if (!pad & PADLdown){
        pressed = 0;
        }
            
}
