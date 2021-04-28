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

#define _WCHAR_T

#include "psx.h"
#include "pad.h"
#include "math.h"
#include "camera.h"
#include "physics.h"
#include "graphics.h"
#include "space.h"
#include "pcdrv.h"

//~ #define USECD

// START OVERLAY

extern u_long load_all_overlays_here;

extern u_long __lvl0_end;

extern u_long __lvl1_end;

u_long overlaySize = 0;

//~ #define LLEVEL 0

//#define USE_POINTER

//~ #if LLEVEL == 0

    //~ static const char*const overlayFile = "\\level0.bin;1";

//~ #else

    //~ static const char*const overlayFile = "\\level1.bin;1";

//~ #endif

//~ #ifdef USE_POINTER

//~ #if LEVEL == 0
  //~ #include "levels/level.h"
//~ #else
  //~ #include "levels/level1.h"
//~ #endif

//~ #else
  //~ #define str (char*)(&__load_start_ovly0)
//~ #endif

// END OVERLAY

#include "levels/level0.h"

#include "levels/level1.h"

// Level

u_short level = 1;

u_short levelWas = 0;

u_short levelHasChanged = 0;

static char* overlayFile;

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

MATRIX		rotlgt;	

SVECTOR	    lgtang = {0, 0, 0};	

MATRIX		light;
	
short vs;

CAMERA camera = {0};

// physics

long time = 0;

//Pad

Controller_Buffer controllers[2];   // Buffers for reading controllers

Controller_Data theControllers[8];  // Processed controller data

int pressed = 0;

u_short timer = 0;

// Cam stuff 

int camMode = 0;

VECTOR angle     = {250,0,0,0};

VECTOR angleCam  = {0,0,0,0};

int dist      = 150; 

int lerping    = 0;

short curCamAngle = 0;

// Inverted Cam coordinates for Forward Vector calc

VECTOR InvCamPos = {0,0,0,0};

VECTOR fVecActor = {0,0,0,0};

u_long triCount = 0;

// Default level : Initialize everything to 0

MATRIX cmat     = {0}, lgtmat   = {0};

MESH   actorPtr = {0}, levelPtr = {0} , propPtr = {0}, meshes[] = {0};

int    meshes_length = 0;

NODE   curNode  = {0};

CAMPATH camPath = {0};

CAMANGLE camPtr = {0}, camAngles[] = {0};

MESH   meshPlan = {0};

VECTOR modelPlan_pos = {0};

LEVEL curLvl = {
    
    &cmat,
	
    &lgtmat,
	
    (MESH **)&meshes,
	
    &meshes_length,
	
    &actorPtr,
	
    &levelPtr,
	
    &propPtr,
	
    &camPtr,
	
    &camPath,
	
    (CAMANGLE **)&camAngles,
	
    &curNode,
	
    &meshPlan
};

LEVEL * loadLvl;

// Pad 

void callback();

int main() {

    if ( level == 0 ){
        
        overlayFile = "\\level0.bin;1";
        
        overlaySize = __lvl0_end;
        
        loadLvl     = &level0;
        
    } else if ( level == 1) {

        overlayFile = "\\level1.bin;1";
        
        overlaySize = __lvl1_end;
        
        loadLvl     = &level1;
    }
    
    // Load overlay

    #ifdef USECD
    
        CdInit();
    
        LoadLevelCD(overlayFile, &load_all_overlays_here);
    
    #endif
    
    // TODO : Add switch case to get the correct pointers
    // Get needed pointers from level file

    if ( level == 0 ) {
    
        LvlPtrSet( &curLvl, &level0);
                
    } else if ( level == 1) {

        LvlPtrSet( &curLvl, &level1);

    } 
    
    levelWas = level;
        
    // Overlay 
    
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
    
	init(disp, draw, db, curLvl.cmat, &BGc, &BKc);
    
    InitPAD(controllers[0].pad, 34, controllers[1].pad, 34);
    
    StartPAD();
    
    generateTable();

    VSyncCallback(callback);

    // Load textures
    
    for (int k = 0; k < *curLvl.meshes_length ; k++){
    
        LoadTexture(curLvl.meshes[k]->tim_data, curLvl.meshes[k]->tim);
    
    }
    
    // Load current BG
    
    if (curLvl.camPtr->tim_data){

        LoadTexture(curLvl.camPtr->tim_data, curLvl.camPtr->BGtim);

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
    
    //~ int angle     = 0;                      //PSX units = 4096 == 360° = 2Pi
    
                        //PSX units 

    short timediv = 1;

    int atime = 0;
    
    // Polycount
    
    for (int k = 0; k < *curLvl.meshes_length; k++){
    
            triCount += curLvl.meshes[k]->tmesh->len;
    
    }
    
    // Set camera starting pos
    
    setCameraPos(&camera, curLvl.camPtr->campos->pos, curLvl.camPtr->campos->rot);
    
    // Find curCamAngle if using pre-calculated BGs

    
    if (camMode == 2) {                              
          
        if (curLvl.camPtr->tim_data){
    
            curCamAngle = 1;
    
    
        }
    }
    
	// Main loop
	
    //~ while (1) {
    
    while ( VSync(1) ) {
        
        if ( levelWas != level ){
            
            switch ( level ){
    
                case 0:
                    
                    overlayFile = "\\level0.bin;1";
                    
                    overlaySize = __lvl0_end;
                    
                    loadLvl     = &level0;
                    
                    break;

                case 1:

                    overlayFile = "\\level1.bin;1";
                    
                    overlaySize = __lvl1_end;
    
                    loadLvl     = &level1;
    
                    break;
            
                default:
                
                    overlayFile = "\\level0.bin;1";
                    
                    loadLvl     = &level0;
                    
                    break;
            
            }
            
            #ifdef USECD
            
              LoadLevelCD( overlayFile, &load_all_overlays_here );
            
            #endif
            
            SwitchLevel( overlayFile, &load_all_overlays_here, &curLvl, loadLvl);
                         
            //~ levelHasChanged = 0;
            levelWas = level;
        }
        
        FntPrint("Ovl:%s\nLvl : %x\nLvl: %d %d \n%x", overlayFile, &level, level, levelWas, loadLvl);

        //~ FntPrint("%x\n", curLvl.actorPtr->tim);
        
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

        //~ curLvl.meshPlan.rot->vx = -( (objAngleToCam.vx >> 4) - 3076 ) ;
        
        //~ curLvl.meshPlan.rot->vx = (( (objAngleToCam.vx >> 4) - 3076 ) * ( (objAngleToCam.vz >> 4) - 3076 ) >> 12) * (nsin(posToCam.vz) >> 10 < 0 ? -1 : 1);
        
        //~ curLvl.meshPlan.rot->vx = ( (objAngleToCam.vx >> 4) - 3076 ) * ( (objAngleToCam.vz >> 4) - 3076 ) >> 12 ;
        
        curLvl.meshPlan->rot.vy = -( (objAngleToCam.vy >> 4) + 1024 ) ;

        //~ posToCam = getVectorTo(*curLvl.meshPlan.pos, camera.pos);
        
        //~ posToCam = getVectorTo(camera.pos, *curLvl.meshPlan.pos);

        posToCam.vx = -camera.pos.vx - curLvl.meshPlan->pos.vx ;
        
        posToCam.vz = -camera.pos.vz - curLvl.meshPlan->pos.vz ;
        
        posToCam.vy = -camera.pos.vy - curLvl.meshPlan->pos.vy ;
        
        //~ psqrt(posToCam.vx * posToCam.vx + posToCam.vy * posToCam.vy);
        
        // Actor Forward vector for 3d relative orientation

        fVecActor = curLvl.actorPtr->pos;
        
        fVecActor.vx = curLvl.actorPtr->pos.vx + (nsin(curLvl.actorPtr->rot.vy/2));
        
        fVecActor.vz = curLvl.actorPtr->pos.vz - (ncos(curLvl.actorPtr->rot.vy/2));

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
            
            dist = 200;
            
            camera.pos.vx = -(camera.x/ONE);
            
            camera.pos.vy = -(camera.y/ONE);
                    
            camera.pos.vz = -(camera.z/ONE);
            
            //~ InvCamPos.vx = camera.x/ONE;
            
            //~ InvCamPos.vz = camera.z/ONE;
        
            //~ applyVector(&InvCamPos, -1,-1,-1, *=);
            
            angle.vy = -(curLvl.actorPtr->rot.vy / 2) + angleCam.vy;
            
            //~ angle.vx += 10;
            
            //~ FntPrint("cos %d", (ncos(angle.vy) * ncos(angle.vx)) >> 12);
            //~ angle = curLvl.actorPtr->rot->vy;

            // Camera horizontal position

            getCameraZY(&camera.z, &camera.y, curLvl.actorPtr->pos.vz, curLvl.actorPtr->pos.vy, angle.vx, dist);

            getCameraXZ(&camera.x, &camera.z, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vz, angle.vy, dist);
            
            //~ getCameraXZY(&camera.x, &camera.z, &camera.y, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vz, curLvl.actorPtr->pos.vy, angle.vy, angle.vx, dist);
            

        //~ void getCameraXZY(int * x, int * z, int * y, int actorX, int actorZ, int actorY, int angle, int angleX, int distance) {

            // Camera vertical position
            
            //~ getCameraXZ(&camera.x, &camera.y, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vy, angle, dist);

            // FIXME! camera lerping to pos
            //~ angle += lerp(camera.rot.vy, -curLvl.actorPtr->rot->vy, 128);
            //~ angle = lerpD(camera.rot.vy << 12, curLvl.actorPtr->rot->vy << 12, 1024 << 12) >> 12;
            
            
            
        }
        
        // Camera rotates continuously around actor
        
        if (camMode == 1) {                      
            
            dist = 150;
           
            camera.pos.vx = -(camera.x/ONE);
           
            //~ camera.pos.vy = -(camera.y/ONE);
            
            camera.pos.vy = 100;
           
            camera.pos.vz = -(camera.z/ONE);
            
            //~ fVecActor = *curLvl.actorPtr->pos;
            
            //~ fVecActor.vx = curLvl.actorPtr->pos->vx + (nsin(curLvl.actorPtr->rot->vy));
            //~ fVecActor.vz = curLvl.actorPtr->pos->vz - (ncos(curLvl.actorPtr->rot->vy));
            
            
            getCameraXZ(&camera.x, &camera.z, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vz, angle.vy, dist);
           
            angle.vy += 10;
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
            
            if (curLvl.camPtr->tim_data){
            
                 checkLineW( &curLvl.camAngles[ curCamAngle ]->fw.v3, &curLvl.camAngles[ curCamAngle ]->fw.v2, curLvl.actorPtr);
            
                if ( curLvl.camAngles[ curCamAngle ]->fw.v0.vx ) {
                    
                    //~ FntPrint("BL x : %d, y : %d\n", camAngles[ curCamAngle ]->fw.v3.vx, camAngles[ curCamAngle ]->fw.v3.vy); 
                    //~ FntPrint("BR x : %d, y : %d\n", camAngles[ curCamAngle ]->fw.v2.vx, camAngles[ curCamAngle ]->fw.v2.vy); 
                    
                    //~ FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->fw.v3, &camAngles[ curCamAngle ]->fw.v2, curLvl.actorPtr) );
                    
                    //~ FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, curLvl.actorPtr) );
                    // If actor in camAngle->fw area of screen
                    
                    if ( checkLineW( &curLvl.camAngles[ curCamAngle ]->fw.v3, &curLvl.camAngles[ curCamAngle ]->fw.v2, curLvl.actorPtr) == -1  && 
                         
                         ( checkLineW( &curLvl.camAngles[ curCamAngle ]->bw.v2, &curLvl.camAngles[ curCamAngle ]->bw.v3, curLvl.actorPtr) >= 0 
                    
                           ) 
                    
                       ) { 
                    
                        if (curCamAngle < 5) {

                            curCamAngle++;

                            curLvl.camPtr = curLvl.camAngles[ curCamAngle ];

                            LoadTexture(curLvl.camPtr->tim_data, curLvl.camPtr->BGtim);

                        }
                    
                    }
                    
                }
                
                if ( curLvl.camAngles[ curCamAngle ]->bw.v0.vx ) {
                    
                    //~ FntPrint("BL x : %d, y : %d\n", camAngles[ curCamAngle ]->bw.v3.vx, camAngles[ curCamAngle ]->bw.v3.vy); 
                    
                    //~ FntPrint("BR x : %d, y : %d\n", camAngles[ curCamAngle ]->bw.v2.vx, camAngles[ curCamAngle ]->bw.v2.vy); 
                    
                    //~ // FntPrint("Pos : %d\n", checkLineW( &camAngles[ curCamAngle ]->bw.v2, &camAngles[ curCamAngle ]->bw.v3, curLvl.actorPtr) );
                
                    // If actor in camAngle->bw area of screen
                    
                    if ( checkLineW( &curLvl.camAngles[ curCamAngle ]->fw.v3, &curLvl.camAngles[ curCamAngle ]->fw.v2, curLvl.actorPtr) >= 0  && 
                         
                         checkLineW( &curLvl.camAngles[ curCamAngle ]->bw.v2, &curLvl.camAngles[ curCamAngle ]->bw.v3, curLvl.actorPtr) == -1 
                    
                       ) {
            
                        if (curCamAngle > 0) {

                            curCamAngle--;

                            curLvl.camPtr = curLvl.camAngles[ curCamAngle ];

                            LoadTexture(curLvl.camPtr->tim_data, curLvl.camPtr->BGtim);

                        }
                    
                    }
                    
                }
            
            }
            
            setCameraPos(&camera, curLvl.camPtr->campos->pos, curLvl.camPtr->campos->rot);

        }
        
        // Flyby mode with LERP from camStart to camEnd
        
        if (camMode == 4) {                               
            
            // If key pos exist for camera
           
            if (curLvl.camPath->len) {
                
                // Lerping sequence has not begun
           
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
           
                    camera.pos.vx = curLvl.camPath->points[curLvl.camPath->cursor].vx;
           
                    camera.pos.vy = curLvl.camPath->points[curLvl.camPath->cursor].vy;
           
                    camera.pos.vz = curLvl.camPath->points[curLvl.camPath->cursor].vz;
                    
                    // Lerping sequence is starting
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    curLvl.camPath->pos = 0;
                    
                    }
                    
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
           
                int precision = 12;
                
                camera.pos.vx = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vx << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vx << precision, curLvl.camPath->pos << precision) >> precision;
           
                camera.pos.vy = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vy << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vy << precision, curLvl.camPath->pos << precision) >> precision;
           
                camera.pos.vz = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vz << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vz << precision, curLvl.camPath->pos << precision) >> precision;
                
                //~ FntPrint("Cam %d, %d\n", (int32_t)curLvl.camPath->points[curLvl.camPath->cursor].vx, curLvl.camPath->points[curLvl.camPath->cursor+1].vx);
                //~ FntPrint("Cam %d, %d, %d\n", camera.pos.vx, camera.pos.vy, camera.pos.vz);
                //~ FntPrint("Theta y: %d x: %d\n", theta.vy, theta.vx);
                //~ FntPrint("Pos: %d Cur: %d\nTheta y: %d x: %d\n", curLvl.camPath->pos, curLvl.camPath->cursor, theta.vy, theta.vx);

                // Linearly increment the lerp factor
           
                curLvl.camPath->pos += 20;
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (curLvl.camPath->pos > (1 << precision) ){
           
                    curLvl.camPath->pos = 0;
           
                    curLvl.camPath->cursor ++;
           
                }                    
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( curLvl.camPath->cursor == curLvl.camPath->len - 1 ){
           
                    lerping = 0;
           
                    curLvl.camPath->cursor = 0;
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
            
            if (curLvl.camPath->len) {
         
            // Lerping sequence has not begun
         
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
         
                    camera.pos.vx = curLvl.camPath->points[curLvl.camPath->cursor].vx;
         
                    camera.pos.vy = curLvl.camPath->points[curLvl.camPath->cursor].vy;
         
                    camera.pos.vz = curLvl.camPath->points[curLvl.camPath->cursor].vz;
                    
                    // Lerping sequence is starting
                    
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    
                    curLvl.camPath->pos = 0;
                    
                    }
                
                // Pre calculated sqrt ( see psqrt() )
                
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
                
                short precision = 12;
                                    
                camera.pos.vx = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vx << precision, curLvl.camPath->points[curLvl.camPath->cursor + 1].vx << precision, curLvl.camPath->pos << precision) >> precision;
         
                camera.pos.vy = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vy << precision, curLvl.camPath->points[curLvl.camPath->cursor + 1].vy << precision, curLvl.camPath->pos << precision) >> precision;
         
                camera.pos.vz = lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vz << precision, curLvl.camPath->points[curLvl.camPath->cursor + 1].vz << precision, curLvl.camPath->pos << precision) >> precision;
                
                //~ FntPrint("%d %d %d %d\n", camAngleToAct.vy, camera.pos.vx, camera.rot.vy, dist);

                // Ony move cam if position is between first curLvl.camPath->vx and last curLvl.camPath->vx

                if ( camAngleToAct.vy < -50 && camera.pos.vx > curLvl.camPath->points[curLvl.camPath->len - 1].vx ) {  
          
                    // Clamp curLvl.camPath position to cameraSpeed
                    
                    curLvl.camPath->pos += dist < cameraSpeed ? 0 : cameraSpeed ;
          
                }
          
                if ( camAngleToAct.vy > 50 && camera.pos.vx > curLvl.camPath->points[curLvl.camPath->cursor].vx ) {  
          
                    curLvl.camPath->pos -= dist < cameraSpeed ? 0 : cameraSpeed;
          
                }
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                
                if (curLvl.camPath->pos > (1 << precision) ){
          
                    curLvl.camPath->pos = 0;
          
                    curLvl.camPath->cursor ++;
          
                } 
                
                if (curLvl.camPath->pos < -100 ){
          
                    curLvl.camPath->pos = 1 << precision;
          
                    curLvl.camPath->cursor --;
                
                }                   
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                
                if ( curLvl.camPath->cursor == curLvl.camPath->len - 1 || curLvl.camPath->cursor < 0 ){
          
                    lerping = 0;
          
                    curLvl.camPath->cursor = 0;
                }
                
            } else { 
                
                // if no key pos exists, switch to next camMode
          
                camMode ++;
            
            }
            
        }
    
    // Spatial partitioning
                
        if (curLvl.curNode){
        
            for ( int msh = 0; msh < curLvl.curNode->siblings->index; msh ++ ) {
            
                // Actor
            
                if ( !getIntCollision( *curLvl.actorPtr->body , *curLvl.curNode->siblings->list[msh]->plane->body).vx &&
                
                     !getIntCollision( *curLvl.actorPtr->body , *curLvl.curNode->siblings->list[msh]->plane->body).vz )
                {
                
                    if ( curLvl.curNode != curLvl.curNode->siblings->list[msh] ) {
                    
                        curLvl.curNode = curLvl.curNode->siblings->list[msh];

                        curLvl.levelPtr = curLvl.curNode->plane;
                    }
                
                }
            
                // DONTNEED ?
                // Moveable prop
                
                //~ if ( !getIntCollision( *propPtr->body , *curLvl.curNode->siblings->list[msh]->plane->body).vx &&
                
                     //~ !getIntCollision( *propPtr->body , *curLvl.curNode->siblings->list[msh]->plane->body).vz ) {

                    //~ if ( propPtr->node != curLvl.curNode->siblings->list[ msh ]){
                        
                        //~ propPtr->node = curLvl.curNode->siblings->list[ msh ];
                    //~ }
                    
                //~ }
                
                if ( !getIntCollision( *curLvl.propPtr->body , *curLvl.curNode->plane->body).vx &&
                
                     !getIntCollision( *curLvl.propPtr->body , *curLvl.curNode->plane->body).vz ) {
                
                    curLvl.propPtr->node = curLvl.curNode;
                
                }
                
            }
        }
        
    // Physics
        
        if ( physics ) {
            
            // if(time%1 == 0){
                
                 for ( int k = 0; k < *curLvl.meshes_length; k ++ ) {
                //~ for ( int k = 0; k < curLvl.curNode->objects->index ; k ++){
                                        
                     if ( ( curLvl.meshes[k]->isRigidBody == 1 ) ) {
                    //~ if ( ( *curLvl.curNode->rigidbodies->list[k]->isRigidBody == 1 ) ) {

                        //~ applyAcceleration(curLvl.curNode->rigidbodies->list[k]->body);
                        
                        applyAcceleration( curLvl.meshes[k]->body );
                    
                        // Get col with level                         ( modelgnd_body )        
                        
                        col_lvl = getIntCollision( *curLvl.meshes[k]->body , *curLvl.levelPtr->body );
                        
                        
                        col_sphere = getIntCollision( *curLvl.propPtr->body, *curLvl.propPtr->node->plane->body );
                        
                        // col_sphere = getIntCollision( *propPtr->body, *levelPtr->body );
                        
                        col_sphere_act = getExtCollision( *curLvl.actorPtr->body, *curLvl.propPtr->body );
                        
                        // If no col with ground, fall off
                
                        if ( col_lvl.vy ) {
                        
                            if ( !col_lvl.vx && !col_lvl.vz ) { 
                                
                                curLvl.actorPtr->body->position.vy = curLvl.actorPtr->body->min.vy;
                            
                            }
                        
                        }
                        
                        if (col_sphere.vy){
                        
                            if ( !col_sphere.vx && !col_sphere.vz ) {
                                
                                curLvl.propPtr->body->position.vy = curLvl.propPtr->body->min.vy;
                            
                            }
                        
                        }
                        
                        if (col_sphere_act.vx && col_sphere_act.vz ) {

                            curLvl.propPtr->body->velocity.vx += curLvl.actorPtr->body->velocity.vx;
                            
                            curLvl.propPtr->body->velocity.vz += curLvl.actorPtr->body->velocity.vz;
                            
                            if ( curLvl.propPtr->body->velocity.vx ) {
                                
                                VECTOR L = angularMom( *curLvl.propPtr->body );
                                
                                curLvl.propPtr->rot.vz -= L.vx;
                            }
                            
                            if ( curLvl.propPtr->body->velocity.vz ) {
                                
                                VECTOR L = angularMom( *curLvl.propPtr->body );
                                
                                curLvl.propPtr->rot.vx -= L.vz;
                            }
                        }
                        
                        curLvl.meshes[k]->pos.vx = curLvl.meshes[k]->body->position.vx;
                        
                        curLvl.meshes[k]->pos.vy = curLvl.meshes[k]->body->position.vy ;
                        
                        curLvl.meshes[k]->pos.vz = curLvl.meshes[k]->body->position.vz;
                        
                        
                    }
                    
                    curLvl.meshes[k]->body->velocity.vy = 0;
                    
                    curLvl.meshes[k]->body->velocity.vx = 0;
                    
                    curLvl.meshes[k]->body->velocity.vz = 0;
                    
                }

            // }
        }
        
        if ( (camMode == 2) && (curLvl.camPtr->tim_data ) ) {
      
            worldToScreen( &curLvl.actorPtr->pos, &curLvl.actorPtr->pos2D );
        
        }

    
    // Camera setup 
        
        // position of cam relative to actor
        
        posToActor.vx = curLvl.actorPtr->pos.vx + camera.pos.vx;
        
        posToActor.vz = curLvl.actorPtr->pos.vz + camera.pos.vz;
        
        posToActor.vy = curLvl.actorPtr->pos.vy + camera.pos.vy;
        
    // Polygon drawing
        
        if (curLvl.curNode){
            
            static long Flag;
            
            if ( (camMode == 2) && (curLvl.camPtr->tim_data ) ) {
          
                drawBG(curLvl.camPtr, &nextpri, otdisc[db], &db);

                // Loop on camAngles
            
                for ( int mesh = 0 ; mesh < curLvl.camAngles[ curCamAngle ]->index; mesh ++ ) {
                    
                    transformMesh(&camera, curLvl.camAngles[curCamAngle]->objects[mesh]);
                    
                    drawPoly(curLvl.camAngles[curCamAngle]->objects[mesh], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                    
                    //                                                          int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw)
                }
                    
            }
            
            else {
                
                // Draw current node's plane

                drawPoly( curLvl.curNode->plane, &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                
                // Draw surrounding planes 
                
                for ( int sibling = 0; sibling < curLvl.curNode->siblings->index; sibling++ ) {
                
                    drawPoly(curLvl.curNode->siblings->list[ sibling ]->plane, &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                
                }
                
                // Draw adjacent planes's children
                
                for ( int sibling = 0; sibling < curLvl.curNode->siblings->index; sibling++ ) {
                    
                    for ( int object = 0; object < curLvl.curNode->siblings->list[ sibling ]->objects->index; object++ ) {
                    
                        long t = 0;
                        
                        transformMesh(&camera, curLvl.curNode->siblings->list[ sibling ]->objects->list[ object ]);
                        
                        drawPoly( curLvl.curNode->siblings->list[ sibling ]->objects->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                    
                    }
                }
                
                // Draw current plane children
                
                for ( int object = 0; object < curLvl.curNode->objects->index; object++ ) {
                
                    transformMesh(&camera, curLvl.curNode->objects->list[ object ]);

                    drawPoly( curLvl.curNode->objects->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                
                }
                
                // Draw rigidbodies
                
                for ( int object = 0; object < curLvl.curNode->rigidbodies->index; object++ ) {
                
                    transformMesh(&camera, curLvl.curNode->rigidbodies->list[ object ]);

                    drawPoly( curLvl.curNode->rigidbodies->list[ object ], &Flag, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                
                }
        
            }    
        }
        
        // Find and apply light rotation matrix

        RotMatrix(&lgtang, &rotlgt);	

        MulMatrix0(curLvl.lgtmat, &rotlgt, &light);

        SetLightMatrix(&light);

        // Set camera

        applyCamera(&camera);
        
        // Add secondary OT to main OT
        
        AddPrims(otdisc[db], ot[db] + OTLEN - 1, ot[db]);

        //~ FntPrint("curLvl.curNode : %x\nIndex: %d", curLvl.curNode, curLvl.curNode->siblings->index);
        
        FntPrint("Time    : %d dt :%d\n", VSync(-1) / 60, dt);
        
        //~ FntPrint("%d\n", curCamAngle );
        //~ FntPrint("%x\n", primbuff[db]);
       
        //~ FntPrint("Actor    : %d %d\n", curLvl.actorPtr->pos->vx, curLvl.actorPtr->pos->vy);
        
        //~ FntPrint("%d %d\n", curLvl.actorPtr->pos->vx, curLvl.actorPtr->pos->vz);
        //~ FntPrint("%d %d\n", curLvl.actorPtr->pos2D.vx + CENTERX, curLvl.actorPtr->pos2D.vy + CENTERY);

        //~ FntPrint(" %d %d %d\n", wp.vx, wp.vy, wp.vz);
        
        FntFlush(-1);
		
		display( &disp[db], &draw[db], otdisc[db], primbuff[db], &nextpri, &db);
		//~ display(disp, draw, otdisc[db], primbuff[db], nextpri, db);
        
        //~ frame = VSync(-1);

	}
    return 0;
}

void callback() {
    
    // Pad 1

    read_controller( &theControllers[0], &controllers[0].pad[0], 0 );  // Read controllers
    
    // Pad 2
    
    read_controller( &theControllers[1], &controllers[1].pad[0], 1 );
    
    //~ u_short pad = PadRead(0);
    
    //~ u_short pad = 0;
    
    u_char PADL = ~theControllers[0].button1;
    
    u_char PADR = ~theControllers[0].button2;
    
    static u_short lastPad;
    
    static short forceApplied = 0;
    
    int div = 32;
    
    static int lerpValues[4096 >> 7];
    
    static short cursor = 0;
    
    static short angleCamTimer = 0;
    
    //~ static short curCamAngle = 0;
    
    if( !lerpValues[0] ) {
        
        for ( long long i = 0; i < div ; i++ ){

            lerpValues[(div-1)-i] = lerp(-24, -264, easeIn(i));
        
        }
    }
    
    if( timer ) {
        
        timer--;
    
    }    

    if( cursor ) {
        
        cursor--;
    
    }
    
    if (angleCam.vy > 2048 || angleCam.vy < -2048) {
    
        angleCam.vy = 0;
    
    }

    if ( PADR & PadShldR1 && !timer ) {
        
        if (!curLvl.camPtr->tim_data){
            
            if(camMode < 6){ 
                
                    camMode ++;
            
                    lerping = 0;
                
            } else {
            
                setCameraPos(&camera, curLvl.camPtr->campos->pos, curLvl.camPtr->campos->rot);
            
                curLvl.camPath->cursor = 0;
            
                camMode = 0;
            
                lerping = 0;
            }

        } else {
            
            if (curCamAngle > 4) {

                curCamAngle = 0;

            }

            if (curCamAngle < 5) {

                curCamAngle++;

                curLvl.camPtr = curLvl.camAngles[ curCamAngle ];

                LoadTexture(curLvl.camPtr->tim_data, curLvl.camPtr->BGtim);

            } 
        }

        lastPad = PADR;

        timer = 10;
    }
        
    //~ if ( !(PADR & PadShldR1) && lastPad & PadShldR1 ) {
        
        //pressed = 0;
    
    //~ }
    
    if ( PADR & PadShldL2 ) {
    
        lgtang.vy += 32;
    
    }
    
    if ( PADR & PadShldL1 ) {
    
        lgtang.vz += 32;
    
    }
        
    if ( PADR & PadUp && !timer ){
    
        if (curLvl.actorPtr->isPrism){
    
            curLvl.actorPtr->isPrism = 0;
    
        } else {
     
            curLvl.actorPtr->isPrism = 1;
     
        }
     
        timer = 10;
     
        lastPad = PADR;
    }
    
    if ( PADR & PadDown && !timer ){
    
        if (curLvl.actorPtr->body->gForce.vy >= 0 && curLvl.actorPtr->body->position.vy >= curLvl.actorPtr->body->min.vy  ){
    
                forceApplied -= 150;
    
        }
    
        cursor = div - 15;
        
        timer = 30;
        
        lastPad = PADR;
    }
    
    if ( !(PADR & PadDown) && lastPad & PadDown ) {
        //~ lastPad = pad;
    }
    
    if ( PADR & PadLeft && !timer ) {
        
        if (curLvl.actorPtr->anim->interpolate){
        
            curLvl.actorPtr->anim->interpolate = 0;
        
        } else {
        
            curLvl.actorPtr->anim->interpolate = 1;
        
        }
        
        timer = 10;
        
        lastPad = PADR;
    }
        
    if (theControllers[0].type == 0x73){
        
        // Analog stick L up
            
        if ( theControllers[0].analog3 >= 0 && theControllers[0].analog3 < 108 ) {
            
            curLvl.actorPtr->body->gForce.vz = getVectorTo(fVecActor, curLvl.actorPtr->pos).vz *  (128 - theControllers[0].analog3 ) >> 15 ;
            
            curLvl.actorPtr->body->gForce.vx = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vx * (128 - theControllers[0].analog3 ) >> 15 ;
            
            lastPad = PADL;
        }

        // Analog stick L down
            
        if ( theControllers[0].analog3 > 168 && theControllers[0].analog3 <= 255 ) {
            
            curLvl.actorPtr->body->gForce.vz = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vz *  ( theControllers[0].analog3 - 128 ) >> 15 ;
            
            curLvl.actorPtr->body->gForce.vx = getVectorTo(fVecActor, curLvl.actorPtr->pos).vx * ( theControllers[0].analog3 - 128 ) >> 15 ;
            
            lastPad = PADL;
        }
        
        // Analog stick L dead zone
        
        if ( theControllers[0].analog3 > 108 && theControllers[0].analog3 < 148 ) {
            
            curLvl.actorPtr->body->gForce.vz = 0;

            curLvl.actorPtr->body->gForce.vx = 0;
            
        }
        
        // Analog stick L left
        
        if ( theControllers[0].analog2 >= 0 && theControllers[0].analog2 < 108 ) {
            
            curLvl.actorPtr->rot.vy -= ( 40 * ( 128 - theControllers[0].analog2 ) ) >> 7 ;
        
        }
        
        // Analog stick L right
        
        if ( theControllers[0].analog2 > 148 && theControllers[0].analog2 <= 255 ) {
            
            curLvl.actorPtr->rot.vy += ( 40 * ( theControllers[0].analog2 - 128 ) ) >> 7 ;
        
        }
        
    }
    
    if ( PADL & PadUp ) {
        
        curLvl.actorPtr->body->gForce.vz = getVectorTo(fVecActor, curLvl.actorPtr->pos).vz >> 8 ;
        
        curLvl.actorPtr->body->gForce.vx = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vx >> 8 ;
        
        lastPad = PADL;
    }
    
    if ( !(PADL & PadUp) && lastPad & PadUp) {

        curLvl.actorPtr->body->gForce.vz = 0;

        curLvl.actorPtr->body->gForce.vx = 0;
        
        lastPad = PADL;
    }
    
    if ( PADL & PadDown ) {

        curLvl.actorPtr->body->gForce.vz = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vz >> 8 ;

        curLvl.actorPtr->body->gForce.vx = getVectorTo(fVecActor, curLvl.actorPtr->pos).vx >> 8 ;

        lastPad = PADL;
    }
    
    if ( !( PADL & PadDown ) && lastPad & PadDown) {

        curLvl.actorPtr->body->gForce.vz = 0;

        curLvl.actorPtr->body->gForce.vx = 0;

        lastPad = PADL;

    }
    
    if ( PADL & PadLeft ) {

        curLvl.actorPtr->rot.vy -= 32;

        lastPad = PADL;

    }
    
    if ( PADL & PadRight ) {

        curLvl.actorPtr->rot.vy += 32;

        lastPad = PADL;
    }
    
    if ( PADL & PadSelect && !timer ) {
        
        //~ if (!levelHasChanged){
        
        //~ #ifndef USECD
        
        printf("load:%p:%08x:%s", &load_all_overlays_here, &level, overlayFile);
        //~ PCload( &load_all_overlays_here, &levelHasChanged, overlayFile );
        
        //~ #endif
    
        
        #ifdef USECD
            
            level = !level;
            
            //~ levelHasChanged = 1;
    
        #endif
            
        //~ }
        
        timer = 30;
        
        lastPad = PADL;
        
        
    }
    
    if( theControllers[0].type == 0x73 && camMode == 0){
        
        // Cam control - horizontal
        
        if ( theControllers[0].analog0 >= 0 && theControllers[0].analog0 < 108) {
        
            angleCam.vy -= ( 16 * ( 128 - theControllers[0].analog0 ) ) >> 7 ;
        
            angleCamTimer = 120;
        
        }

        if ( theControllers[0].analog0 > 148 && theControllers[0].analog0 <= 255) {
        
            angleCam.vy += ( 16 * ( theControllers[0].analog0 - 128 ) ) >> 7 ;
            
            angleCamTimer = 120;
        
        }
        
        
        if ( theControllers[0].analog0 >= 0 && theControllers[0].analog0 < 108) {
        
            angleCam.vy -= ( 16 * ( 128 - theControllers[0].analog0 ) ) >> 7 ;
        
            angleCamTimer = 120;
        
        }

        if ( theControllers[0].analog0 > 148 && theControllers[0].analog0 <= 255) {
        
            angleCam.vy += ( 16 * ( theControllers[0].analog0 - 128 ) ) >> 7 ;
            
            angleCamTimer = 120;
        
        }
        
        // Timer to lerp cam back behind actor
        
        if ( angleCamTimer ){
        
            angleCamTimer --;
            
        }
        
        if (!angleCamTimer && angleCam.vy){
            
            angleCam.vy += lerp( angleCam.vy, 0, 64 ) == 0 ? 1 : lerp( angleCam.vy, 0, 64 );
        
        }
        
    
    } 
    
    //~ FntPrint("level :%d", level);
    
    //~ FntPrint("angleCam :%d %d\n", angleCam.vy, lerp( angleCam.vy, 0, 64) );
    
    //~ FntPrint( "Pad 1 : %02x\nButtons:%02x %02x, Stick:%02d %02d %02d %02d\n",
            //~ theControllers[0].type,             // Controller type : 0x00 == none,  0x41 == standard, 0x73 == analog/dualshock, 0x12 == mouse, 0x23 == steering wheel, 0x63 == gun, 0x53 == analog joystick
            //~ theControllers[0].button1,          // 
            //~ theControllers[0].button2,
            //~ theControllers[0].analog0,  // R3 hor  : left: 0 7F right: 7F FF   dz 78 83
            //~ theControllers[0].analog1,  // R3 vert :  up : 0 7F down : 7F FF : dz 83 86
            //~ theControllers[0].analog2,  // L3 hor : left : 0 7F right: 7F FF : dz 69 81 68 - 8E 
            //~ theControllers[0].analog3 ); // L3 vert : up : 0 7F down : 7F FF : dz 74 8D
    
    if ( cursor ) {
        
        curLvl.actorPtr->body->position.vy = lerpValues[cursor];}
    
};
