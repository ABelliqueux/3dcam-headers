// 3dcam
// With huge help from :
// @NicolasNoble : https://discord.com/channels/642647820683444236/646765703143227394/796876392670429204
// @Lameguy64
// @Impiaa
// @paul

		
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

// Blender debug mode
// bpy. app. debug = True 

//~ #include <sys/types.h>
//~ #include <libgte.h>
//~ #include <libgpu.h>
//~ #include <libetc.h>
//~ #include <stdio.h>
//~ #include <stdint.h>
//~ #include <stddef.h>

//~ #include "coridor2.h"

#include "psx.h"
#include "math.h"
#include "camera.h"
#include "physics.h"
#include "graphics.h"
#include "space.h"

#include "coridor2.c"

//~ #define VMODE       0

//~ #define SCREENXRES 320

//~ #define SCREENYRES 240

//~ #define CENTERX		SCREENXRES/2

//~ #define CENTERY		SCREENYRES/2

//~ #define FOV CENTERX                             // With a FOV of 1/2, camera focal length is ~= 16 mm / 90°
                                                //~ // Lower values mean wider angle
//~ // pixel > cm : used in physics calculations

//~ #define SCALE 4

//~ #define FNT_POS_X 960

//~ #define FNT_POS_Y 256


//~ #define OT2LEN	    8	                   

//~ #define OTLEN	    256	                        // Maximum number of OT entries

//~ #define PRIMBUFFLEN	4096 * sizeof(POLY_GT4)	    // Maximum number of POLY_GT3 primitives

// Display and draw environments, double buffered

DISPENV disp[2];

DRAWENV draw[2];

//~ // OT for BG/FG discrimination

u_long otdisc[2][OT2LEN] = {0};

// Main OT

u_long	    ot[2][OTLEN]  = {0};   		        // Ordering table (contains addresses to primitives)

char	primbuff[2][PRIMBUFFLEN] = {0};	        // Primitive list // That's our prim buffer

int		    primcnt=0;			            // Primitive counter

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
	
short vs;

CAMERA camera = {0};

// physics

long time = 0;

//~ const int gravity = 10;
    
int camMode = 2;

//Pad

int pressed = 0;

u_short timer = 0;

// Cam stuff 


//~ long timeB = 0;

int lerping    = 0;

short curCamAngle = 0;

// Inverted Cam coordinates for Forward Vector calc

VECTOR InvCamPos = {0,0,0,0};

VECTOR fVecActor = {0,0,0,0};

u_long triCount = 0;

// Drawing

//~ void drawBG(void);

//~ void drawPoly(MESH * meshes, long * Flag, int atime);


// Pad 

void callback();

int main() {
    
    //~ cmatP = &cmat;
    
    VECTOR sp = {CENTERX,CENTERY,0};
    
    VECTOR wp = {0,0,0};
    
    // FIXME : Poly subdiv
    
    //~ DIVPOLYGON4	div4 = { 0 };
    //~ div4.pih = SCREENXRES;
	//~ div4.piv = SCREENYRES;
    //~ div4.ndiv = 2;
    //~ long OTc = 0;
    
    //~ DIVPOLYGON3	div3 = { 0 };
    //~ div3.pih = SCREENXRES;
	//~ div3.piv = SCREENYRES;
    //~ div3.ndiv = 1;
    
	init(disp, draw, db, &cmat, &BGc, &BKc);
    

    generateTable();

    VSyncCallback(callback);

    // Load textures
    
    for (int k = 0; k < sizeof(meshes)/sizeof(TMESH *); k++){
    
        LoadTexture(meshes[k]->tim_data, meshes[k]->tim);
    
    }
    
    // Load current BG
    
    if (camPtr->tim_data){

        LoadTexture(camPtr->tim_data, camPtr->BGtim);

    }
    
    // Physics

    short physics = 1;

    long dt;

    VECTOR col_lvl, col_sphere, col_sphere_act = {0};
    
    // Cam stuff 
    
    VECTOR posToActor    = {0, 0, 0, 0};      // position of camera relative to actor    
    
    VECTOR camAngleToAct = {0, 0, 0, 0};      // rotation angles for the camera to point at actor
    
    // Sprite system
    
    VECTOR posToCam      = {0, 0, 0, 0};
    
    VECTOR objAngleToCam = {0, 0, 0, 0};
    
    int angle     = 0;                      //PSX units = 4096 == 360° = 2Pi
    
    int dist      = 0;                      //PSX units 

    short timediv = 1;

    int atime = 0;
    
    // Polycount
    
    for (int k = 0; k < sizeof(meshes)/sizeof(meshes[0]); k++){
    
            triCount += meshes[k]->tmesh->len;
    
    }
    
    // Set camera starting pos
    
    setCameraPos(&camera, camPtr->campos->pos, camPtr->campos->rot);
    
    // Find curCamAngle if using pre-calculated BGs
    
    if (camMode == 2) {                              
          
        if (camPtr->tim_data){
    
            curCamAngle = 1;
    
        }
    }
    
	// Main loop
	
    //~ while (1) {
    
    while ( VSync(1) ) {
        
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

        // Sprite system WIP

        objAngleToCam.vy = patan( posToCam.vx,posToCam.vz );
        
        objAngleToCam.vx = patan( posToCam.vx,posToCam.vy );

        //~ objAngleToCam.vz = patan( posToCam.vz,posToCam.vy );
        
        //~ objAngleToCam.vx = patan( psqrt(posToCam.vx * posToCam.vx + posToCam.vy * posToCam.vy), posToCam.vy );

        //~ meshPlan.rot->vx = -( (objAngleToCam.vx >> 4) - 3076 ) ;
        
        //~ meshPlan.rot->vx = (( (objAngleToCam.vx >> 4) - 3076 ) * ( (objAngleToCam.vz >> 4) - 3076 ) >> 12) * (nsin(posToCam.vz) >> 10 < 0 ? -1 : 1);
        
        //~ meshPlan.rot->vx = ( (objAngleToCam.vx >> 4) - 3076 ) * ( (objAngleToCam.vz >> 4) - 3076 ) >> 12 ;
        
        meshPlan.rot->vy = -( (objAngleToCam.vy >> 4) + 1024 ) ;

        //~ posToCam = getVectorTo(*meshPlan.pos, camera.pos);
        
        //~ posToCam = getVectorTo(camera.pos, *meshPlan.pos);

        posToCam.vx = -camera.pos.vx - modelPlan_pos.vx ;
        
        posToCam.vz = -camera.pos.vz - modelPlan_pos.vz ;
        
        posToCam.vy = -camera.pos.vy - modelPlan_pos.vy ;
        
        //~ psqrt(posToCam.vx * posToCam.vx + posToCam.vy * posToCam.vy);
        
        // Actor Forward vector for 3d relative orientation

        fVecActor = *actorPtr->pos;
        
        fVecActor.vx = actorPtr->pos->vx + (nsin(actorPtr->rot->vy/2));
        
        fVecActor.vz = actorPtr->pos->vz - (ncos(actorPtr->rot->vy/2));

    // Camera modes

        if(camMode != 2) {
            
            camera.rot.vy = camAngleToAct.vy;
            
            // using csin/ccos, no need for theta
            
            //~ camera.rot.vy = angle; 
            
            camera.rot.vx = camAngleToAct.vx;   
        
        }
        
        if(camMode < 4 ) {
       
            lerping = 0;
       
        }
        
        // Camera follows actor with lerp for rotations
        
        if(camMode == 0) {
            
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
        
        if (camMode == 1) {                      
            
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
        if (camMode == 3) {                              
            
            // Using precalc sqrt

            dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
            
            camera.pos.vx = 190;

            camera.pos.vz = 100;

            camera.pos.vy = 180;
        }
        
        // Fixed Camera angle
        if (camMode == 2) {                              
          
          
            // If BG images exist
            
            if (camPtr->tim_data){
            
                 checkLineW( &camAngles[ curCamAngle ]->fw.v3, &camAngles[ curCamAngle ]->fw.v2, actorPtr);
            
                if ( camAngles[ curCamAngle ]->fw.v0.vx ) {
                    
                    //~ FntPrint("BL x : %d, y : %d\n", camAngles[ curCamAngle ]->fw.v3.vx, camAngles[ curCamAngle ]->fw.v3.vy); 
                    //~ FntPrint("BR x : %d, y : %d\n", camAngles[ curCamAngle ]->fw.v2.vx, camAngles[ curCamAngle ]->fw.v2.vy); 
                    
                    //~ FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->fw.v3, &camAngles[ curCamAngle ]->fw.v2, actorPtr) );
                    
                    //~ FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, actorPtr) );
                    // If actor in camAngle->fw area of screen
                    
                    if ( checkLineW( &camAngles[ curCamAngle ]->fw.v3, &camAngles[ curCamAngle ]->fw.v2, actorPtr) == -1  && 
                         
                         ( checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, actorPtr) >= 0 
                    
                           ) 
                    
                       ) { 
                    
                        if (curCamAngle < 5) {

                            curCamAngle++;

                            camPtr = camAngles[ curCamAngle ];

                            LoadTexture(camPtr->tim_data, camPtr->BGtim);

                        }
                    
                    }
                    
                }
                
                if ( camAngles[ curCamAngle ]->bw.v0.vx ) {
                    
                    //~ FntPrint("BL x : %d, y : %d\n", camAngles[ curCamAngle ]->bw.v3.vx, camAngles[ curCamAngle ]->bw.v3.vy); 
                    
                    //~ FntPrint("BR x : %d, y : %d\n", camAngles[ curCamAngle ]->bw.v2.vx, camAngles[ curCamAngle ]->bw.v2.vy); 
                    
                    //~ // FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, actorPtr) );
                
                    // If actor in camAngle->bw area of screen
                    
                    if ( checkLineW( &camAngles[ curCamAngle ]->fw.v3, &camAngles[ curCamAngle ]->fw.v2, actorPtr) >= 0  && 
                         
                         checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, actorPtr) == -1 
                    
                       ) {
            
                        if (curCamAngle > 0) {

                            curCamAngle--;

                            camPtr = camAngles[ curCamAngle ];

                            LoadTexture(camPtr->tim_data, camPtr->BGtim);

                        }
                    
                    }
                    
                }
            
            }
            
            setCameraPos(&camera, camPtr->campos->pos, camPtr->campos->rot);

        }
        
        // Flyby mode with LERP from camStart to camEnd
        
        if (camMode == 4) {                               
            
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
            
            short cameraSpeed = 40;
            
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
                
                //~ FntPrint("%d %d %d %d\n", camAngleToAct.vy, camera.pos.vx, camera.rot.vy, dist);

                // Ony move cam if position is between first camPath.vx and last camPath.vx

                if ( camAngleToAct.vy < -50 && camera.pos.vx > camPath.points[camPath.len - 1].vx ) {  
          
                    // Clamp camPath position to cameraSpeed
                    
                    camPath.pos += dist < cameraSpeed ? 0 : cameraSpeed ;
          
                }
          
                if ( camAngleToAct.vy > 50 && camera.pos.vx > camPath.points[camPath.cursor].vx ) {  
          
                    camPath.pos -= dist < cameraSpeed ? 0 : cameraSpeed;
          
                }
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                
                if (camPath.pos > (1 << precision) ){
          
                    camPath.pos = 0;
          
                    camPath.cursor ++;
          
                } 
                
                if (camPath.pos < -100 ){
          
                    camPath.pos = 1 << precision;
          
                    camPath.cursor --;
                
                }                   
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                
                if ( camPath.cursor == camPath.len - 1 || camPath.cursor < 0 ){
          
                    lerping = 0;
          
                    camPath.cursor = 0;
                }
                
            } else { 
                
                // if no key pos exists, switch to next camMode
          
                camMode ++;
            
            }
            
        }
    
    // Spatial partitioning
        
        for ( int msh = 0; msh < curNode->siblings->index; msh ++ ) {
        
            // Actor
        
            if ( !getIntCollision( *actorPtr->body , *curNode->siblings->list[msh]->plane->body).vx &&
            
                 !getIntCollision( *actorPtr->body , *curNode->siblings->list[msh]->plane->body).vz )
            {
            
                if ( curNode != curNode->siblings->list[msh] ) {
                
                    curNode = curNode->siblings->list[msh];

                    levelPtr = curNode->plane;
                }
            
            }
        
            // DONTNEED ?
            // Moveable prop
            
            //~ if ( !getIntCollision( *propPtr->body , *curNode->siblings->list[msh]->plane->body).vx &&
            
                 //~ !getIntCollision( *propPtr->body , *curNode->siblings->list[msh]->plane->body).vz ) {

                //~ if ( propPtr->node != curNode->siblings->list[ msh ]){
                    
                    //~ propPtr->node = curNode->siblings->list[ msh ];
                //~ }
                
            //~ }
            
            if ( !getIntCollision( *propPtr->body , *curNode->plane->body).vx &&
            
                 !getIntCollision( *propPtr->body , *curNode->plane->body).vz ) {
            
                propPtr->node = curNode;
            
            }
            
        }
        
    // Physics
        
        if ( physics ) {
            
            // if(time%1 == 0){
                
                 for ( int k = 0; k < sizeof(meshes)/sizeof(meshes[0]);k ++ ) {
                //~ for ( int k = 0; k < curNode->objects->index ; k ++){
                                        
                     if ( ( *meshes[k]->isRigidBody == 1 ) ) {
                    //~ if ( ( *curNode->rigidbodies->list[k]->isRigidBody == 1 ) ) {

                        //~ applyAcceleration(curNode->rigidbodies->list[k]->body);
                        
                        applyAcceleration(meshes[k]->body);
                    
                        // Get col with level                         ( modelgnd_body )        
                        
                        col_lvl = getIntCollision( *meshes[k]->body , *levelPtr->body );
                        
                        
                        col_sphere = getIntCollision( *propPtr->body, *propPtr->node->plane->body );
                        
                        // col_sphere = getIntCollision( *propPtr->body, *levelPtr->body );
                        
                        col_sphere_act = getExtCollision( *actorPtr->body, *propPtr->body );
                        
                        // If no col with ground, fall off
                
                        if ( col_lvl.vy ) {
                        
                            if ( !col_lvl.vx && !col_lvl.vz ) { 
                                
                                actorPtr->body->position.vy = actorPtr->body->min.vy;
                            
                            }
                        
                        }
                        
                        if (col_sphere.vy){
                        
                            if ( !col_sphere.vx && !col_sphere.vz ) {
                                
                                propPtr->body->position.vy = propPtr->body->min.vy;
                            
                            }
                        
                        }
                        
                        if (col_sphere_act.vx && col_sphere_act.vz ) {

                            propPtr->body->velocity.vx += actorPtr->body->velocity.vx;
                            
                            propPtr->body->velocity.vz += actorPtr->body->velocity.vz;
                            
                            if ( propPtr->body->velocity.vx ) {
                                
                                VECTOR L = angularMom(*propPtr->body);
                                
                                propPtr->rot->vz -= L.vx;
                            }
                            
                            if ( propPtr->body->velocity.vz ) {
                                
                                VECTOR L = angularMom( *propPtr->body );
                                
                                propPtr->rot->vx -= L.vz;
                            }
                        }
                        
                        meshes[k]->pos->vx = meshes[k]->body->position.vx;
                        
                        meshes[k]->pos->vy = meshes[k]->body->position.vy ;
                        
                        meshes[k]->pos->vz = meshes[k]->body->position.vz;
                        
                        
                    }
                    
                    meshes[k]->body->velocity.vy = 0;
                    
                    meshes[k]->body->velocity.vx = 0;
                    
                    meshes[k]->body->velocity.vz = 0;
                    
                }

            // }
        }
        
        if ( (camMode == 2) && (camPtr->tim_data ) ) {
      
            worldToScreen(actorPtr->pos, &actorPtr->pos2D);
        
        }

    
    // Camera setup 
        
        // position of cam relative to actor
        
        posToActor.vx = actorPtr->pos->vx + camera.pos.vx;
        
        posToActor.vz = actorPtr->pos->vz + camera.pos.vz;
        
        posToActor.vy = actorPtr->pos->vy + camera.pos.vy;
        
    // Polygon drawing
        
        static long Flag;
        
        if ( (camMode == 2) && (camPtr->tim_data ) ) {
      
            drawBG(camPtr, &nextpri, otdisc[db], &db);

            // Loop on camAngles
        
            for ( int mesh = 0 ; mesh < camAngles[ curCamAngle ]->index; mesh ++ ) {
                
                transformMesh(&camera, camAngles[curCamAngle]->objects[mesh]);
                
                drawPoly(camAngles[curCamAngle]->objects[mesh], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                
                //                                                          int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw)
            }
                
        }
        
        else {
            
        // Draw current node's plane

        drawPoly( curNode->plane, &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
        
        // Draw surrounding planes 
        
		for ( int sibling = 0; sibling < curNode->siblings->index; sibling++ ) {
        
                drawPoly(curNode->siblings->list[ sibling ]->plane, &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
        
        }
        
        // Draw adjacent planes's children
        
        for ( int sibling = 0; sibling < curNode->siblings->index; sibling++ ) {
            
            for ( int object = 0; object < curNode->siblings->list[ sibling ]->objects->index; object++ ) {
            
                long t = 0;
                
                transformMesh(&camera, curNode->siblings->list[ sibling ]->objects->list[ object ]);
                
                drawPoly( curNode->siblings->list[ sibling ]->objects->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
            
            }
        }
        
        // Draw current plane children
        
		for ( int object = 0; object < curNode->objects->index; object++ ) {
        
            transformMesh(&camera, curNode->objects->list[ object ]);

            drawPoly( curNode->objects->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
        
        }
        
        // Draw rigidbodies
        
        for ( int object = 0; object < curNode->rigidbodies->index; object++ ) {
        
            transformMesh(&camera, curNode->rigidbodies->list[ object ]);

            drawPoly( curNode->rigidbodies->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
        
        }
    
    }    
    
        // Find and apply light rotation matrix

        RotMatrix(&lgtang, &rotlgt);	

        MulMatrix0(&lgtmat, &rotlgt, &light);

        SetLightMatrix(&light);

        // Set camera

        applyCamera(&camera);
        
        // Add secondary OT to main OT
        
        AddPrims(otdisc[db], ot[db] + OTLEN - 1, ot[db]);

        //~ FntPrint("CurNode : %x\nIndex: %d", curNode, curNode->siblings->index);
        
        FntPrint("Time    : %d dt :%d\n", VSync(-1) / 60, dt);
        //~ FntPrint("%d\n", curCamAngle );
        //~ FntPrint("%x\n", primbuff[db]);
       
        //~ FntPrint("Actor    : %d %d\n", actorPtr->pos->vx, actorPtr->pos->vy);
        
        //~ FntPrint("%d %d\n", actorPtr->pos->vx, actorPtr->pos->vz);
        //~ FntPrint("%d %d\n", actorPtr->pos2D.vx + CENTERX, actorPtr->pos2D.vy + CENTERY);

        //~ FntPrint(" %d %d %d\n", wp.vx, wp.vy, wp.vz);
        
        FntFlush(-1);
		
		display( &disp[db], &draw[db], otdisc[db], primbuff[db], &nextpri, &db);
		//~ display(disp, draw, otdisc[db], primbuff[db], nextpri, db);
        
        //~ frame = VSync(-1);

	}
    return 0;
}


//void drawPoly(MESH * mesh, long * Flag, int atime){

    //long nclip, t = 0;
    
    //// mesh is POLY_GT3 ( triangle )
    
    //if (mesh->index[t].code == 4) {
        
        //POLY_GT3 * poly;                        
    
        //// len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
        
        //for ( int i = 0; i < (mesh->tmesh->len * 3); i += 3 ) {               
            
            //// If mesh is not part of precalculated background, draw them, else, discard
            
            //if ( !( *mesh->isBG ) || camMode != 2) {
            
                //poly = (POLY_GT3 *)nextpri;
                
                //// If Vertex Anim flag is set, use it
                
                //if (*mesh->isAnim){
                
                    //// If interpolation flag is set, use it
                    
                    //if(mesh->anim->interpolate){
                        
                         //// Ping pong 
                         
                         ////~ //if (mesh->anim->cursor > 4096 || mesh->anim->cursor < 0){
                         
                         ////~ //   mesh->anim->dir *= -1;
                         
                         ////~ //}
                         
                         
                         //// Fixed point math precision
                         
                         //short precision = 12;

                         //// Find next keyframe 
                         
                         //if (mesh->anim->cursor > (1 << precision)) {
                            
                            //// There are still keyframes to interpolate between
                            
                            //if ( mesh->anim->lerpCursor < mesh->anim->nframes - 1 ) {
                            
                                //mesh->anim->lerpCursor ++;
                            
                                //mesh->anim->cursor = 0;
                            
                            //}
                            
                            //// We've reached last frame, go back to first frame
                            
                            //if ( mesh->anim->lerpCursor == mesh->anim->nframes - 1 ) {

                                //mesh->anim->lerpCursor = 0;
                            
                                //mesh->anim->cursor = 0;
                            
                            //}
                         
                         //}
                         
                        //// Let's lerp between keyframes

                        //// TODO : Finish lerped animation implementation
                        
                        //// Vertex 1
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vx  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vz  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vy  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //// Vertex 2
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vx  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vz  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vy  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //// Vertex 3
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vx  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vz  << precision, mesh->anim->cursor << precision)  >> precision;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vy  << precision, mesh->anim->cursor << precision)  >> precision;
                         
                        //mesh->anim->cursor += 24 * mesh->anim->dir;
                         
                        //// Coord transformation from world space to screen space
                        
                        //nclip = RotAverageNclip3(
                                   
                                    //&mesh->tmesh->v[ mesh->index[t].order.vx ],  
                                   
                                    //&mesh->tmesh->v[ mesh->index[t].order.vz ],
                                   
                                    //&mesh->tmesh->v[ mesh->index[t].order.vy ],
                                   
                                    //( long* ) &poly->x0, ( long* ) &poly->x1, ( long* ) &poly->x2,
                                   
                                    //mesh->p,
                                   
                                    //mesh->OTz,
                                   
                                    //Flag
                                //);
                                                
                        //} else { 
                           
                        //// No interpolation
                        
                            //// Use the pre-calculated vertices coordinates from the animation data
                           
                            //nclip = RotAverageNclip3(
                                        
                                        //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vx ],
                                        
                                        //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vz ],
                                        
                                        //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vy ],
                                        
                                        //( long* ) &poly->x0, ( long* ) &poly->x1, ( long* ) &poly->x2,
                                        
                                        //mesh->p,
                                        
                                        //mesh->OTz,
                                        
                                        //Flag
                                    //);
                    //}
                        
                //} else {
                                            
                //// No animation
                
                    //// Use model's regular vertex coordinates

                    //nclip = RotAverageNclip3(

                                //&mesh->tmesh->v[ mesh->index[t].order.vx ],  
                                
                                //&mesh->tmesh->v[ mesh->index[t].order.vz ],
                                
                                //&mesh->tmesh->v[ mesh->index[t].order.vy ],
                                
                                //( long * ) &poly->x0, ( long * ) &poly->x1, ( long * ) &poly->x2,
                                
                                //mesh->p,
                                
                                //mesh->OTz,
                                
                                //Flag
                            //);
                    
                //}
                
                //// Do not draw invisible meshes 
                
                //if ( nclip > 0 && *mesh->OTz > 0 && (*mesh->p < 4096) ) {

                    
                    //SetPolyGT3( poly );
                    
                    //// If isPrism flag is set, use it

                    //// FIXME : Doesn't work with pre-rendered BGs

                    //if ( *mesh->isPrism ) { 
                        
                        //// Transparency effect :
                        
                        //// Use current DRAWENV clip as TPAGE instead of regular textures

                        //( (POLY_GT3 *) poly )->tpage = getTPage( mesh->tim->mode&0x3, 0,
                                                                 
                                                                 //draw[db].clip.x,
                                                                 
                                                                 //draw[db].clip.y
                                                       //);
                        
                        //// Use projected coordinates (results from RotAverage...) as UV coords and clamp them to 0-255,0-224 Why 224 though ?

                        //setUV3(poly,  (poly->x0 < 0 ? 0 : poly->x0 > 255 ? 255 : poly->x0),
                         
                                      //(poly->y0 < 0 ? 0 : poly->y0 > 240 ? 240 : poly->y0), 
                        
                                      //(poly->x1 < 0 ? 0 : poly->x1 > 255 ? 255 : poly->x1), 
                        
                                      //(poly->y1 < 0 ? 0 : poly->y1 > 240 ? 240 : poly->y1), 
                        
                                      //(poly->x2 < 0 ? 0 : poly->x2 > 255 ? 255 : poly->x2), 
                        
                                      //(poly->y2 < 0 ? 0 : poly->y2 > 240 ? 240 : poly->y2)
                        
                                      //);
                        
     
                    //} else {
                    
                    //// No transparency effect
                    
                        //// Use regular TPAGE
                        
                        //( (POLY_GT3 *) poly )->tpage = getTPage(mesh->tim->mode&0x3, 0,
                        
                                                         //mesh->tim->prect->x,
                        
                                                         //mesh->tim->prect->y
                        //);
                        
                        //setUV3(poly,  mesh->tmesh->u[i].vx  , mesh->tmesh->u[i].vy   + mesh->tim->prect->y,
                        
                                      //mesh->tmesh->u[i+2].vx, mesh->tmesh->u[i+2].vy + mesh->tim->prect->y,
                        
                                      //mesh->tmesh->u[i+1].vx, mesh->tmesh->u[i+1].vy + mesh->tim->prect->y);
                    //}

                    //// CLUT setup
                    //// If tim mode == 0 | 1 (4bits/8bits image), set CLUT coordinates

                    //if ( (mesh->tim->mode & 0x3 ) < 2){
                        
                        //setClut(poly,             
                        
                                //mesh->tim->crect->x,
                        
                                //mesh->tim->crect->y);
                    //}
                    
                    //if (*mesh->isSprite){ 
                                 
                        //SetShadeTex( poly, 1 );
                    
                    //}
                    //// Defaults depth color to neutral grey

                    //CVECTOR outCol  = { 128,128,128,0 };
                    
                    //CVECTOR outCol1 = { 128,128,128,0 };
                    
                    //CVECTOR outCol2 = { 128,128,128,0 };
                    
                    //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vx ], &mesh->tmesh->c[ mesh->index[t].order.vx ], *mesh->p, &outCol);

                    //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vz ], &mesh->tmesh->c[ mesh->index[t].order.vz ], *mesh->p, &outCol1);

                    //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vy ], &mesh->tmesh->c[ mesh->index[t].order.vy ], *mesh->p, &outCol2);                           
                
                    //// If transparent effect is in use, inhibate shadows
                
                    //if (*mesh->isPrism){ 
                        
                        //// Use un-interpolated (i.e: no light, no fog) colors

                        //setRGB0(poly, mesh->tmesh->c[i].r,   mesh->tmesh->c[i].g, mesh->tmesh->c[i].b);

                        //setRGB1(poly, mesh->tmesh->c[i+1].r, mesh->tmesh->c[i+1].g, mesh->tmesh->c[i+1].b);

                        //setRGB2(poly, mesh->tmesh->c[i+2].r, mesh->tmesh->c[i+2].g, mesh->tmesh->c[i+2].b);
                    
                    //} else {
                        
                        //setRGB0(poly, outCol.r, outCol.g  , outCol.b);
                        
                        //setRGB1(poly, outCol1.r, outCol1.g, outCol1.b);
                        
                        //setRGB2(poly, outCol2.r, outCol2.g, outCol2.b);
                    //} 
                           
                    //if ( (*mesh->OTz > 0) && (*mesh->OTz < OTLEN) && (*mesh->p < 4096) ) {
                        
                        //AddPrim(&ot[db][*mesh->OTz-2], poly);
                    //}
                    
                    ////~ mesh->pos2D.vx = *(&poly->x0);
                    ////~ mesh->pos2D.vy = *(&poly->x0 + 1);
                    //// mesh->pos2D.vy = poly->x0;
                    //// FntPrint("%d %d\n", *(&poly->x0), *(&poly->x0 + 1));
                    
                    //nextpri += sizeof(POLY_GT3);
                //}
            
                //t+=1;

            //}
        //}
    
    //}
    
    //// If mesh is quad
    
    //if (mesh->index[t].code == 8) {
        
        //POLY_GT4 * poly4;
        
        //for (int i = 0; i < (mesh->tmesh->len * 4); i += 4) {               
            
            //// if mesh is not part of BG, draw them, else, discard
            
            //if ( !(*mesh->isBG) || camMode != 2 ) {
            
                //poly4 = (POLY_GT4 *)nextpri;
                                
                //// Vertex Anim 

                //if (*mesh->isAnim){
                    
                    //// with interpolation

                    //if ( mesh->anim->interpolate ){
                        
                        //// ping pong
                        ////~ if (mesh->anim->cursor > 4096 || mesh->anim->cursor < 0){
                           ////~ mesh->anim->dir *= -1;
                        ////~ }
                        
                        //short precision = 12;

                        //if ( mesh->anim->cursor > 1<<precision ) {

                            //if ( mesh->anim->lerpCursor < mesh->anim->nframes - 1 ) {

                                //mesh->anim->lerpCursor ++;

                                //mesh->anim->cursor = 0;

                            //}

                            //if ( mesh->anim->lerpCursor == mesh->anim->nframes - 1 ) {

                                //mesh->anim->lerpCursor = 0;

                                //mesh->anim->cursor = 0;
                            //}
                        //}
                        
                        //// Vertex 1
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vx << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vx  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vz << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vz  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vx ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vy << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vy  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //// Vertex 2
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vx << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vx  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vz << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vz  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vz ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vy << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vy  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //// Vertex 3
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vx << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vx  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vz << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vz  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.vy ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vy << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vy  << 12, mesh->anim->cursor << 12)  >> 12;
                        
                        //// Vertex 4
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.pad ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vx << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vx  << 12, mesh->anim->cursor << 12) >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.pad ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vz << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vz  << 12, mesh->anim->cursor << 12) >> 12;
                        
                        //mesh->tmesh->v[ mesh->index[ t ].order.pad ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vy << 12 , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vy  << 12, mesh->anim->cursor << 12) >> 12;
                        
                        //mesh->anim->cursor += 2 * mesh->anim->dir;
                        
                        //// Coord transformations
                        //nclip = RotAverageNclip4(
                                
                                    //&mesh->tmesh->v[ mesh->index[t].order.pad ],  
                                    
                                    //&mesh->tmesh->v[ mesh->index[t].order.vz],
                                    
                                    //&mesh->tmesh->v[ mesh->index[t].order.vx ],
                                    
                                    //&mesh->tmesh->v[ mesh->index[t].order.vy ],
                                    
                                    //( long* )&poly4->x0, ( long* )&poly4->x1, ( long* )&poly4->x2, ( long* )&poly4->x3,
                                    
                                    //mesh->p,
                                    
                                    //mesh->OTz,
                                    
                                    //Flag
                                
                                //);
                            
                    //} else {
                        
                        //// No interpolation, use all vertices coordinates in anim data
                         
                        //nclip = RotAverageNclip4(
                            
                                    //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.pad ],
                                    
                                    //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vz ],
                                    
                                    //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vx ],
                                    
                                    //&mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vy ],
                                    
                                    //( long* )&poly4->x0, ( long* )&poly4->x1, ( long* )&poly4->x2, ( long* )&poly4->x3,
                                    
                                    //mesh->p,
                                    
                                    //mesh->OTz,
                                    
                                    //Flag
                                //);
                    //}
                            
                //} else {                        
                
                    //// No animation
                    //// Use regulare vertex coords
                    
                    //nclip = RotAverageNclip4(
                               
                                //&mesh->tmesh->v[ mesh->index[t].order.pad ],  
                               
                                //&mesh->tmesh->v[ mesh->index[t].order.vz],
                               
                                //&mesh->tmesh->v[ mesh->index[t].order.vx ],
                               
                                //&mesh->tmesh->v[ mesh->index[t].order.vy ],
                               
                                //(long*)&poly4->x0, (long*)&poly4->x1, (long*)&poly4->x2, (long*)&poly4->x3,
                               
                                //mesh->p,
                               
                                //mesh->OTz,
                               
                                //Flag
                            //);
                //}
                
                //if (nclip > 0 && *mesh->OTz > 0 && (*mesh->p < 4096)) {
             
                    //SetPolyGT4(poly4);
                        
                    //// FIXME : Polygon subdiv - is it working ?
                    
                    ////~ OTc = *mesh->OTz >> 4;
                    ////~ FntPrint("OTC:%d", OTc);
                    
                    ////~ if (OTc < 4) {
                    
                        ////~ if (OTc > 1) div4.ndiv = 1; else div4.ndiv = 2;
                            
                            ////~ DivideGT4(
                                ////~ // Vertex coord
                                ////~ &mesh->tmesh->v[ mesh->index[t].order.pad ],  
                                ////~ &mesh->tmesh->v[ mesh->index[t].order.vz ],
                                ////~ &mesh->tmesh->v[ mesh->index[t].order.vx ],
                                ////~ &mesh->tmesh->v[ mesh->index[t].order.vy ],
                                ////~ // UV coord
                                ////~ mesh->tmesh->u[i+3],
                                ////~ mesh->tmesh->u[i+2],
                                ////~ mesh->tmesh->u[i+0],
                                ////~ mesh->tmesh->u[i+1],
                                
                                ////~ // Color
                                ////~ mesh->tmesh->c[i], 
                                ////~ mesh->tmesh->c[i+1], 
                                ////~ mesh->tmesh->c[i+2], 
                                ////~ mesh->tmesh->c[i+3], 

                                ////~ // Gpu packet
                                ////~ poly4,
                                ////~ &ot[db][*mesh->OTz],
                                ////~ &div4);
                                        
                            ////~ // Increment primitive list pointer
                            ////~ nextpri  += ( (sizeof(POLY_GT4) + 3) / 4 ) * (( 1 << ( div4.ndiv )) << ( div4.ndiv ));
                            ////~ triCount = ((1<<(div4.ndiv))<<(div4.ndiv));
                    
                    ////~ } else if (OTc < 48) {
                
                    //// Transparency effect
                    
                    //if (*mesh->isPrism){ 
                        
                        //// Use current DRAWENV clip as TPAGE
                        
                        //( (POLY_GT4 *) poly4)->tpage = getTPage(mesh->tim->mode&0x3, 0,
                                                                 //draw[db].clip.x,
                                                                 //draw[db].clip.y
                                                       //);
                        
                        //// Use projected coordinates
                        
                        //setUV4( poly4, 
                              
                                //(poly4->x0 < 0? 0 : poly4->x0 > 255? 255 : poly4->x0), 
                              
                                //(poly4->y0 < 0? 0 : poly4->y0 > 224? 224 : poly4->y0), 
                              
                                //(poly4->x1 < 0? 0 : poly4->x1 > 255? 255 : poly4->x1), 
                              
                                //(poly4->y1 < 0? 0 : poly4->y1 > 224? 224 : poly4->y1), 
                              
                                //(poly4->x2 < 0? 0 : poly4->x2 > 255? 255 : poly4->x2), 
                              
                                //(poly4->y2 < 0? 0 : poly4->y2 > 224? 224 : poly4->y2),
                              
                                //(poly4->x3 < 0? 0 : poly4->x3 > 255? 255 : poly4->x3), 
                              
                                //(poly4->y3 < 0? 0 : poly4->y3 > 224? 224 : poly4->y3)
                        //);
                        
     
                    //} else {
                        
                        //// Use regular TPAGE
                        //( (POLY_GT4 *) poly4)->tpage = getTPage( 
                                                        
                                                         //mesh->tim->mode&0x3, 0,
                        
                                                         //mesh->tim->prect->x,
                        
                                                         //mesh->tim->prect->y
                                                     //);
                        
                        //// Use model UV coordinates
                        
                        //setUV4( poly4, 
                                //mesh->tmesh->u[i+3].vx, mesh->tmesh->u[i+3].vy   + mesh->tim->prect->y,
                                
                                //mesh->tmesh->u[i+2].vx, mesh->tmesh->u[i+2].vy + mesh->tim->prect->y,
                            
                                //mesh->tmesh->u[i+0].vx, mesh->tmesh->u[i+0].vy + mesh->tim->prect->y,
                            
                                //mesh->tmesh->u[i+1].vx, mesh->tmesh->u[i+1].vy + mesh->tim->prect->y
                        //);

                    //}
                    
                    //if (*mesh->isSprite){ 
                                 
                        //SetShadeTex( poly4, 1 );
                    
                    //}
                    
                        //// If tim mode  == 0 | 1, set CLUT coordinates
                        //if ( (mesh->tim->mode & 0x3) < 2 ) {
                            
                            //setClut(poly4,             
                            
                                    //mesh->tim->crect->x,
                            
                                    //mesh->tim->crect->y
                            //);
                            
                        //}
                        
                        //CVECTOR outCol  = {128,128,128,0};
                        
                        //CVECTOR outCol1 = {128,128,128,0};
                        
                        //CVECTOR outCol2 = {128,128,128,0};
                        
                        //CVECTOR outCol3 = {128,128,128,0};

                        //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.pad ]  , &mesh->tmesh->c[ mesh->index[t].order.pad ], *mesh->p, &outCol);
                        
                        //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vz ], &mesh->tmesh->c[ mesh->index[t].order.vz ], *mesh->p, &outCol1);
                        
                        //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vx ], &mesh->tmesh->c[ mesh->index[t].order.vx ], *mesh->p, &outCol2);
                        
                        //NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vy ], &mesh->tmesh->c[  mesh->index[t].order.vy ], *mesh->p, &outCol3);

                    //if (*mesh->isPrism){ 
                        
                        //setRGB0(poly4, mesh->tmesh->c[i].r, mesh->tmesh->c[i].g, mesh->tmesh->c[i].b);

                        //setRGB1(poly4, mesh->tmesh->c[i+1].r, mesh->tmesh->c[i+1].g, mesh->tmesh->c[i+1].b);

                        //setRGB2(poly4, mesh->tmesh->c[i+2].r, mesh->tmesh->c[i+2].g, mesh->tmesh->c[i+2].b);

                        //setRGB3(poly4, mesh->tmesh->c[i+3].r, mesh->tmesh->c[i+3].g, mesh->tmesh->c[i+3].b);
                    
                    //} else {
                        
                        //setRGB0(poly4, outCol.r, outCol.g  , outCol.b);

                        //setRGB1(poly4, outCol1.r, outCol1.g, outCol1.b);

                        //setRGB2(poly4, outCol2.r, outCol2.g, outCol2.b);

                        //setRGB3(poly4, outCol3.r, outCol3.g, outCol3.b);
                    //} 
                           
                    //if ( (*mesh->OTz > 0) && (*mesh->OTz < OTLEN) && (*mesh->p < 4096) ) {

                        //AddPrim( &ot[ db ][ *mesh->OTz-3 ], poly4 );      
                    //}
                    
                    //nextpri += sizeof( POLY_GT4 );
                    
                //}
            
            //t += 1;

            //}
        //}
    //} 

//};


void callback() {
    
    u_short pad = PadRead(0);
    
    static u_short lastPad;
    
    static short forceApplied = 0;
    
    int div = 32;
    
    static int lerpValues[4096 >> 7];
    
    static short cursor = 0;
    
    //~ static short curCamAngle = 0;
    
    if( !lerpValues[0] ) {
        
        for ( long long i = 0; i < div ; i++ ){

            lerpValues[(div-1)-i] = lerp(-24, -264, easeIn(i));
        
        }
    }
    
    if( timer ) {
        
        timer--;
    
    }    

    if( cursor>0 ) {
        
        cursor--;
    
    }    

    if ( pad & PADR1 && !timer ) {
        
        if (!camPtr->tim_data){
            
            if(camMode < 6){ 
                
                    camMode ++;
            
                    lerping = 0;
                
            } else {
            
                setCameraPos(&camera, camPtr->campos->pos, camPtr->campos->rot);
            
                camPath.cursor = 0;
            
                camMode = 0;
            
                lerping = 0;
            }

        } else {
            
            if (curCamAngle > 4) {

                curCamAngle = 0;

            }

            if (curCamAngle < 5) {

                curCamAngle++;

                camPtr = camAngles[ curCamAngle ];

                LoadTexture(camPtr->tim_data, camPtr->BGtim);

            } 
        }

        lastPad = pad;

        timer = 10;
    }
        
    if ( !(pad & PADR1) && lastPad & PADR1 ) {
        
        //~ pressed = 0;
    
    }
    
    if ( pad & PADL2 ) {
    
        lgtang.vy += 32;
    
    }
    
    if ( pad & PADL1 ) {
    
        lgtang.vz += 32;
    
    }
    
    if ( pad & PADRup && !timer ){
    
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
    
    if ( !(pad & PADRdown) && lastPad & PADRdown ) {
        //~ lastPad = pad;
    }
    
    if ( pad & PADRleft && !timer ) {
        
        if (actorPtr->anim->interpolate){
        
            actorPtr->anim->interpolate = 0;
        
        } else {
        
            actorPtr->anim->interpolate = 1;
        
        }
        
        timer = 10;
        
        lastPad = pad;
    }
        
    if ( pad & PADLup ) {
        
        actorPtr->body->gForce.vz = getVectorTo(fVecActor, *actorPtr->pos).vz >> 8 ;
        
        actorPtr->body->gForce.vx = -getVectorTo(fVecActor, *actorPtr->pos).vx >> 8 ;
        
        lastPad = pad;
    }
    
    if ( !(pad & PADLup) && lastPad & PADLup) {

        actorPtr->body->gForce.vz = 0;

        actorPtr->body->gForce.vx = 0;
    }
    
    if ( pad & PADLdown ) {

        actorPtr->body->gForce.vz = -getVectorTo(fVecActor, *actorPtr->pos).vz >> 8 ;

        actorPtr->body->gForce.vx = getVectorTo(fVecActor, *actorPtr->pos).vx >> 8 ;

        lastPad = pad;
    }
    
    if ( !(pad & PADLdown) && lastPad & PADLdown) {

        actorPtr->body->gForce.vz = 0;

        actorPtr->body->gForce.vx = 0;

        lastPad = pad;

    }
    
    if ( pad & PADLleft ) {

        actorPtr->rot->vy -= 32;

        lastPad = pad;

    }
    
    if ( pad & PADLright ) {

        actorPtr->rot->vy += 32;

        lastPad = pad;
    }
    
    if ( cursor ) {
        
        actorPtr->body->position.vy = lerpValues[cursor];}
    
};
