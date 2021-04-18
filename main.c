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

//~ #define USECD

// START OVERLAY

extern u_long load_all_overlays_here;
extern u_long __lvl0_end;
extern u_long __lvl1_end;

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

short level = 0;

short levelHasChanged = 0;

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

int camMode = 2;

//Pad

Controller_Buffer controllers[2];   // Buffers for reading controllers

Controller_Data theControllers[8];  // Processed controller data

int pressed = 0;

u_short timer = 0;

// Cam stuff 

int angle     = 0;

int angleCam  = 0;

int lerping    = 0;

short curCamAngle = 0;

// Inverted Cam coordinates for Forward Vector calc

VECTOR InvCamPos = {0,0,0,0};

VECTOR fVecActor = {0,0,0,0};

u_long triCount = 0;

// These are set to the corresponding address in the LEVEL struct of current loaded level

MATRIX * cmat, * lgtmat;

MESH * actorPtr, * levelPtr, * propPtr, ** meshes;

int * meshes_length;

NODE * curNode;

CAMPATH * camPath;

CAMANGLE * camPtr, ** camAngles;

// Get rid of those

MESH * meshPlan;

VECTOR * modelPlan_pos;

// Zero level : Initialize everything to 0

MATRIX Zcmat = {0}, Zlgtmat = {0};

MESH ZactorPtr = {0}, ZlevelPtr = {0} , ZpropPtr = {0}, Zmeshes[] = {0};

int Zmeshes_length = 0;

NODE ZcurNode = {0};

CAMPATH ZcamPath = {0};

CAMANGLE ZcamPtr = {0}, ZcamAngles[] = {0};

// Get rid of those

MESH ZmeshPlan = {0};

VECTOR ZmodelPlan_pos = {0};

LEVEL zero = {
    &Zcmat,
	&Zlgtmat,
	&Zmeshes,
	&Zmeshes_length,
	&ZactorPtr,
	&ZlevelPtr,
	&ZpropPtr,
	&ZcamPtr,
	&ZcamPath,
	&ZcamAngles,
	0,
	&ZmeshPlan
};

// Pad 

void callback();

int main() {

    if ( level == 0 ){
        
        overlayFile = "\\level0.bin;1";
        
    } else if ( level == 1) {

        overlayFile = "\\level1.bin;1";
        
        }

    // Use Struct to hold level's pointers

    //~ LEVEL curLevel = {
        //~ cmat,
        //~ lgtmat,
        //~ meshes,
        //~ meshes_length,
        //~ actorPtr,
        //~ levelPtr,
        //~ propPtr,
        //~ camPtr,
        //~ camPath,
        //~ camAngles,
        //~ curNode,
        //~ meshPlan
    //~ };
    
    // Zeroing the level pointers
    
    cmat = zero.cmat;

    lgtmat  = zero.lgtmat;

    meshes  = (MESH **)zero.meshes;

    meshes_length = zero.meshes_length;

    actorPtr = zero.actorPtr;

    levelPtr = zero.levelPtr;

    propPtr  = zero.propPtr;
    
    camPtr   = zero.camPtr;
    
    camPath  = zero.camPath;
    
    camAngles = (CAMANGLE **)zero.camAngles;
    
    curNode   = zero.curNode; // Blank
    
    // Move these to drawPoly()

    meshPlan  = zero.meshPlan;
    
    
    // Load overlay

    #ifdef USECD
    
        CdInit();
    
        LoadLevel(overlayFile, &load_all_overlays_here);
    
    #endif
    
    // TODO : Add switch case to get the correct pointers
    // Get needed pointers from level file

    if ( level == 0 ) {
    
        cmat = level0.cmat;
    
        lgtmat  = level0.lgtmat;
    
        meshes  = (MESH **)level0.meshes;
    
        meshes_length = level0.meshes_length;
    
        actorPtr = level0.actorPtr;

        levelPtr = level0.levelPtr;

        propPtr  = level0.propPtr;
        
        camPtr   = level0.camPtr;
        
        camPath  = level0.camPath;
        
        camAngles = (CAMANGLE **)level0.camAngles;
        
        curNode   = level0.curNode; // Blank
        
        // Move these to drawPoly()
    
        meshPlan  = level0.meshPlan;
        
        //~ LvlPtrSet( &curLevel, &level0);
                
    } else if ( level == 1) {
    
        cmat    = level1.cmat;
    
        lgtmat  = level1.lgtmat;
    
        meshes  = (MESH **)level1.meshes;
    
        meshes_length = level1.meshes_length;
    
        actorPtr = level1.actorPtr;

        levelPtr = level1.levelPtr;

        propPtr  = level1.propPtr;
        
        camPtr   = level1.camPtr;
        
        camPath  = level1.camPath;
        
        camAngles = (CAMANGLE **)level1.camAngles;
        
        curNode   = level1.curNode;
        
        // Move these to drawPoly()
    
        meshPlan  = level1.meshPlan;
        
        //~ modelPlan_pos = level1_meshPlan->pos;
        
    } 
        
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
    
	init(disp, draw, db, cmat, &BGc, &BKc);
    
    InitPAD(controllers[0].pad, 34, controllers[1].pad, 34);
    
    StartPAD();
    
    generateTable();

    VSyncCallback(callback);

    // Load textures
    
    for (int k = 0; k < *meshes_length ; k++){
    
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
    
    //~ int angle     = 0;                      //PSX units = 4096 == 360° = 2Pi
    
    int dist      = 0;                      //PSX units 

    short timediv = 1;

    int atime = 0;
    
    // Polycount
    
    for (int k = 0; k < *meshes_length; k++){
    
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
        
        if (levelHasChanged){
    
            if ( level == 0 ) {
                
                //~ cmat = zero.cmat;

                //~ lgtmat  = zero.lgtmat;

                //~ meshes  = (MESH **)zero.meshes;

                //~ meshes_length = zero.meshes_length;

                //~ actorPtr = zero.actorPtr;

                //~ levelPtr = zero.levelPtr;

                //~ propPtr  = zero.propPtr;
                
                //~ camPtr   = zero.camPtr;
                
                //~ camPath  = zero.camPath;
                
                //~ camAngles = (CAMANGLE **)zero.camAngles;
                
                //~ curNode   = zero.curNode; // Blank
                
                // Move these to drawPoly()

                //~ meshPlan  = zero.meshPlan;
                
                ScrRst();
                
                overlayFile = "\\level0.bin;1";

                LoadLevel(overlayFile, &load_all_overlays_here);
            
                cmat = level0.cmat;
            
                lgtmat  = level0.lgtmat;
            
                meshes  = level0.meshes;
            
                meshes_length = level0.meshes_length;
            
                actorPtr = level0.actorPtr;

                levelPtr = level0.levelPtr;

                propPtr  = level0.propPtr;
                
                camPtr   = level0.camPtr;
                
                camPath  = level0.camPath;
                
                camAngles = level0.camAngles;
                
                curNode   = level0.curNode; // Blank
                
                // Move these to drawPoly()
            
                meshPlan  = level0.meshPlan;
                
                //~ LvlPtrSet( &curLevel, &level0);
                
                for (int k = 0; k < *meshes_length ; k++){
    
                    LoadTexture(meshes[k]->tim_data, meshes[k]->tim);
    
                }
                        
            } else if ( level == 1) {
                
                //~ cmat = zero.cmat;

                //~ lgtmat  = zero.lgtmat;

                //~ meshes  = (MESH **)zero.meshes;

                //~ meshes_length = zero.meshes_length;

                //~ actorPtr = zero.actorPtr;

                //~ levelPtr = zero.levelPtr;

                //~ propPtr  = zero.propPtr;
                
                //~ camPtr   = zero.camPtr;
                
                //~ camPath  = zero.camPath;
                
                //~ camAngles = (CAMANGLE **)zero.camAngles;
                
                //~ curNode   = zero.curNode; // Blank
                
                //~ // Move these to drawPoly()

                //~ meshPlan  = zero.meshPlan;
        
                //~ ScrRst();
                
                overlayFile = "\\level1.bin;1";

                LoadLevel(overlayFile, &load_all_overlays_here);
            
                cmat    = level1.cmat;
            
                lgtmat  = level1.lgtmat;
            
                meshes  = level1.meshes;
            
                meshes_length = level1.meshes_length;
            
                actorPtr = level1.actorPtr;

                levelPtr = level1.levelPtr;

                propPtr  = level1.propPtr;
                
                camPtr   = level1.camPtr;
                
                camPath  = level1.camPath;
                
                camAngles = level1.camAngles;
                
                curNode   = level1.curNode;
                
                // Move these to drawPoly()
            
                meshPlan  = level1.meshPlan;
                
                //~ modelPlan_pos = level1_meshPlan->pos;
                
                for (int k = 0; k < *meshes_length ; k++){
    
                    LoadTexture(meshes[k]->tim_data, meshes[k]->tim);
    
                }
                
            }
            
            levelHasChanged = 0;
        
        }
        
        
        FntPrint("%x\n", actorPtr->tim);
        
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
        
        meshPlan->rot.vy = -( (objAngleToCam.vy >> 4) + 1024 ) ;

        //~ posToCam = getVectorTo(*meshPlan.pos, camera.pos);
        
        //~ posToCam = getVectorTo(camera.pos, *meshPlan.pos);

        posToCam.vx = -camera.pos.vx - meshPlan->pos.vx ;
        
        posToCam.vz = -camera.pos.vz - meshPlan->pos.vz ;
        
        posToCam.vy = -camera.pos.vy - meshPlan->pos.vy ;
        
        //~ psqrt(posToCam.vx * posToCam.vx + posToCam.vy * posToCam.vy);
        
        // Actor Forward vector for 3d relative orientation

        fVecActor = actorPtr->pos;
        
        fVecActor.vx = actorPtr->pos.vx + (nsin(actorPtr->rot.vy/2));
        
        fVecActor.vz = actorPtr->pos.vz - (ncos(actorPtr->rot.vy/2));

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
            
            angle = -(actorPtr->rot.vy / 2) + angleCam;
            
            //~ angle = actorPtr->rot->vy;

            getCameraXZ(&camera.x, &camera.z, actorPtr->pos.vx, actorPtr->pos.vz, angle, dist);

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
            
            
            getCameraXZ(&camera.x, &camera.z, actorPtr->pos.vx, actorPtr->pos.vz, angle, dist);
           
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
           
            if (camPath->len) {
                
                // Lerping sequence has not begun
           
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
           
                    camera.pos.vx = camPath->points[camPath->cursor].vx;
           
                    camera.pos.vy = camPath->points[camPath->cursor].vy;
           
                    camera.pos.vz = camPath->points[camPath->cursor].vz;
                    
                    // Lerping sequence is starting
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    camPath->pos = 0;
                    
                    }
                    
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
           
                int precision = 12;
                
                camera.pos.vx = lerpD(camPath->points[camPath->cursor].vx << precision, camPath->points[camPath->cursor+1].vx << precision, camPath->pos << precision) >> precision;
           
                camera.pos.vy = lerpD(camPath->points[camPath->cursor].vy << precision, camPath->points[camPath->cursor+1].vy << precision, camPath->pos << precision) >> precision;
           
                camera.pos.vz = lerpD(camPath->points[camPath->cursor].vz << precision, camPath->points[camPath->cursor+1].vz << precision, camPath->pos << precision) >> precision;
                
                //~ FntPrint("Cam %d, %d\n", (int32_t)camPath->points[camPath->cursor].vx, camPath->points[camPath->cursor+1].vx);
                //~ FntPrint("Cam %d, %d, %d\n", camera.pos.vx, camera.pos.vy, camera.pos.vz);
                //~ FntPrint("Theta y: %d x: %d\n", theta.vy, theta.vx);
                //~ FntPrint("Pos: %d Cur: %d\nTheta y: %d x: %d\n", camPath->pos, camPath->cursor, theta.vy, theta.vx);

                // Linearly increment the lerp factor
           
                camPath->pos += 20;
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (camPath->pos > (1 << precision) ){
           
                    camPath->pos = 0;
           
                    camPath->cursor ++;
           
                }                    
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( camPath->cursor == camPath->len - 1 ){
           
                    lerping = 0;
           
                    camPath->cursor = 0;
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
            
            if (camPath->len) {
         
            // Lerping sequence has not begun
         
                if (!lerping){
                    
                    // Set cam start position ( first key pos )
         
                    camera.pos.vx = camPath->points[camPath->cursor].vx;
         
                    camera.pos.vy = camPath->points[camPath->cursor].vy;
         
                    camera.pos.vz = camPath->points[camPath->cursor].vz;
                    
                    // Lerping sequence is starting
                    
                    lerping = 1;
                    
                    // Set cam pos index to 0
                    
                    camPath->pos = 0;
                    
                    }
                
                // Pre calculated sqrt ( see psqrt() )
                
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                
                // Fixed point precision 2^12 == 4096
                
                short precision = 12;
                                    
                camera.pos.vx = lerpD(camPath->points[camPath->cursor].vx << precision, camPath->points[camPath->cursor + 1].vx << precision, camPath->pos << precision) >> precision;
         
                camera.pos.vy = lerpD(camPath->points[camPath->cursor].vy << precision, camPath->points[camPath->cursor + 1].vy << precision, camPath->pos << precision) >> precision;
         
                camera.pos.vz = lerpD(camPath->points[camPath->cursor].vz << precision, camPath->points[camPath->cursor + 1].vz << precision, camPath->pos << precision) >> precision;
                
                //~ FntPrint("%d %d %d %d\n", camAngleToAct.vy, camera.pos.vx, camera.rot.vy, dist);

                // Ony move cam if position is between first camPath->vx and last camPath->vx

                if ( camAngleToAct.vy < -50 && camera.pos.vx > camPath->points[camPath->len - 1].vx ) {  
          
                    // Clamp camPath position to cameraSpeed
                    
                    camPath->pos += dist < cameraSpeed ? 0 : cameraSpeed ;
          
                }
          
                if ( camAngleToAct.vy > 50 && camera.pos.vx > camPath->points[camPath->cursor].vx ) {  
          
                    camPath->pos -= dist < cameraSpeed ? 0 : cameraSpeed;
          
                }
                
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                
                if (camPath->pos > (1 << precision) ){
          
                    camPath->pos = 0;
          
                    camPath->cursor ++;
          
                } 
                
                if (camPath->pos < -100 ){
          
                    camPath->pos = 1 << precision;
          
                    camPath->cursor --;
                
                }                   
                
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                
                if ( camPath->cursor == camPath->len - 1 || camPath->cursor < 0 ){
          
                    lerping = 0;
          
                    camPath->cursor = 0;
                }
                
            } else { 
                
                // if no key pos exists, switch to next camMode
          
                camMode ++;
            
            }
            
        }
    
    // Spatial partitioning
                
        if (curNode){
        
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
        }
        
    // Physics
        
        if ( physics ) {
            
            // if(time%1 == 0){
                
                 for ( int k = 0; k < *meshes_length; k ++ ) {
                //~ for ( int k = 0; k < curNode->objects->index ; k ++){
                                        
                     if ( ( meshes[k]->isRigidBody == 1 ) ) {
                    //~ if ( ( *curNode->rigidbodies->list[k]->isRigidBody == 1 ) ) {

                        //~ applyAcceleration(curNode->rigidbodies->list[k]->body);
                        
                        applyAcceleration( meshes[k]->body );
                    
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
                                
                                VECTOR L = angularMom( *propPtr->body );
                                
                                propPtr->rot.vz -= L.vx;
                            }
                            
                            if ( propPtr->body->velocity.vz ) {
                                
                                VECTOR L = angularMom( *propPtr->body );
                                
                                propPtr->rot.vx -= L.vz;
                            }
                        }
                        
                        meshes[k]->pos.vx = meshes[k]->body->position.vx;
                        
                        meshes[k]->pos.vy = meshes[k]->body->position.vy ;
                        
                        meshes[k]->pos.vz = meshes[k]->body->position.vz;
                        
                        
                    }
                    
                    meshes[k]->body->velocity.vy = 0;
                    
                    meshes[k]->body->velocity.vx = 0;
                    
                    meshes[k]->body->velocity.vz = 0;
                    
                }

            // }
        }
        
        if ( (camMode == 2) && (camPtr->tim_data ) ) {
      
            worldToScreen( &actorPtr->pos, &actorPtr->pos2D );
        
        }

    
    // Camera setup 
        
        // position of cam relative to actor
        
        posToActor.vx = actorPtr->pos.vx + camera.pos.vx;
        
        posToActor.vz = actorPtr->pos.vz + camera.pos.vz;
        
        posToActor.vy = actorPtr->pos.vy + camera.pos.vy;
        
    // Polygon drawing
        
        if (curNode){
            
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
        }
        
        // Find and apply light rotation matrix

        RotMatrix(&lgtang, &rotlgt);	

        MulMatrix0(lgtmat, &rotlgt, &light);

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

    if ( PADR & PadShldR1 && !timer ) {
        
        if (!camPtr->tim_data){
            
            if(camMode < 6){ 
                
                    camMode ++;
            
                    lerping = 0;
                
            } else {
            
                setCameraPos(&camera, camPtr->campos->pos, camPtr->campos->rot);
            
                camPath->cursor = 0;
            
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
    
        if (actorPtr->isPrism){
    
            actorPtr->isPrism = 0;
    
        } else {
     
            actorPtr->isPrism = 1;
     
        }
     
        timer = 10;
     
        lastPad = PADR;
    }
    
    if ( PADR & PadDown && !timer ){
    
        if (actorPtr->body->gForce.vy >= 0 && actorPtr->body->position.vy >= actorPtr->body->min.vy  ){
    
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
        
        if (actorPtr->anim->interpolate){
        
            actorPtr->anim->interpolate = 0;
        
        } else {
        
            actorPtr->anim->interpolate = 1;
        
        }
        
        timer = 10;
        
        lastPad = PADR;
    }
        
    // Analog stick L up
        
    if ( theControllers[0].analog3 >= 0 && theControllers[0].analog3 < 108 ) {
        
        actorPtr->body->gForce.vz = getVectorTo(fVecActor, actorPtr->pos).vz *  (128 - theControllers[0].analog3 ) >> 15 ;
        
        actorPtr->body->gForce.vx = -getVectorTo(fVecActor, actorPtr->pos).vx * (128 - theControllers[0].analog3 ) >> 15 ;
        
        lastPad = PADL;
    }

    // Analog stick L down
        
    if ( theControllers[0].analog3 > 148 && theControllers[0].analog3 <= 255 ) {
        
        actorPtr->body->gForce.vz = -getVectorTo(fVecActor, actorPtr->pos).vz *  ( theControllers[0].analog3 - 128 ) >> 15 ;
        
        actorPtr->body->gForce.vx = getVectorTo(fVecActor, actorPtr->pos).vx * ( theControllers[0].analog3 - 128 ) >> 15 ;
        
        lastPad = PADL;
    }
    
    // Analog stick L dead zone
    
    if ( theControllers[0].analog3 > 108 && theControllers[0].analog3 < 148 ) {
        
        actorPtr->body->gForce.vz = 0;

        actorPtr->body->gForce.vx = 0;
        
    }
    
    // Analog stick L left
    
    if ( theControllers[0].analog2 >= 0 && theControllers[0].analog2 < 108 ) {
        
        actorPtr->rot.vy -= ( 40 * ( 128 - theControllers[0].analog2 ) ) >> 7 ;
    
    }
    
    // Analog stick L right
    
    if ( theControllers[0].analog2 > 148 && theControllers[0].analog2 <= 255 ) {
        
        actorPtr->rot.vy += ( 40 * ( theControllers[0].analog2 - 128 ) ) >> 7 ;
    
    }
    
    if ( PADL & PadUp ) {
        
        actorPtr->body->gForce.vz = getVectorTo(fVecActor, actorPtr->pos).vz >> 8 ;
        
        actorPtr->body->gForce.vx = -getVectorTo(fVecActor, actorPtr->pos).vx >> 8 ;
        
        lastPad = PADL;
    }
    
    if ( !(PADL & PadUp) && lastPad & PadUp) {

        actorPtr->body->gForce.vz = 0;

        actorPtr->body->gForce.vx = 0;
        
        lastPad = PADL;
    }
    
    if ( PADL & PadDown ) {

        actorPtr->body->gForce.vz = -getVectorTo(fVecActor, actorPtr->pos).vz >> 8 ;

        actorPtr->body->gForce.vx = getVectorTo(fVecActor, actorPtr->pos).vx >> 8 ;

        lastPad = PADL;
    }
    
    if ( !( PADL & PadDown ) && lastPad & PadDown) {

        actorPtr->body->gForce.vz = 0;

        actorPtr->body->gForce.vx = 0;

        lastPad = PADL;

    }
    
    if ( PADL & PadLeft ) {

        actorPtr->rot.vy -= 32;

        lastPad = PADL;

    }
    
    if ( PADL & PadRight ) {

        actorPtr->rot.vy += 32;

        lastPad = PADL;
    }
    
    if ( PADL & PadSelect && !timer ) {
        
        if (!levelHasChanged){
        
            level = !level;
            
            levelHasChanged = 1;
        }
        timer = 30;
        
        lastPad = PADL;
        
        
    }
    
    if(camMode == 0){
        
        if ( theControllers[0].analog0 >= 0 && theControllers[0].analog0 < 108) {
        
            //~ angleCam -= 16;
            angleCam -= ( 16 * ( 128 - theControllers[0].analog0 ) ) >> 7 ;
        
        }

        if ( theControllers[0].analog0 > 148 && theControllers[0].analog0 <= 255) {
        
            //~ angleCam += 16;
            angleCam += ( 16 * ( theControllers[0].analog0 - 128 ) ) >> 7 ;
        
        }
    
    } 
    
    //~ FntPrint("level :%d", level);
    
    FntPrint("PADL :%d \n", angleCam );
    
    FntPrint( "Pad 1 : %02x\nButtons:%02x %02x, Stick:%02d %02d %02d %02d\n",
            theControllers[0].type,             // Controller type : 00 == none,  41 == standard, 73 == analog/dualshock, 12 == mouse, 23 == steering wheel, 63 == gun, 53 == analog joystick
            theControllers[0].button1,          // 
            theControllers[0].button2,
            theControllers[0].analog0,  // R3 hor  : left: 0 7F right: 7F FF   dz 78 83
            theControllers[0].analog1,  // R3 vert :  up : 0 7F down : 7F FF : dz 83 86
            theControllers[0].analog2,  // L3 hor : left : 0 7F right: 7F FF : dz 69 81 68 - 8E 
            theControllers[0].analog3 ); // L3 vert : up : 0 7F down : 7F FF : dz 74 8D
    
    if ( cursor ) {
        
        actorPtr->body->position.vy = lerpValues[cursor];}
    
};
