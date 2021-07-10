// 3dcam
// With huge help from :
// @NicolasNoble : https://discord.com/channels/642647820683444236/646765703143227394/796876392670429204
// @Lameguy64
// @Impiaa
// @paul        
 /*        PSX screen coordinate system 
 *
 *                           Z+
 *                          /
 *                         /
 *                        +------X+
 *                       /|
 *                      / |
 *                     /  Y+
 *                   eye        */
// Blender debug mode
// bpy. app. debug = True 
#define _WCHAR_T
#include "../include/psx.h"
#include "../include/pad.h"
#include "../include/math.h"
#include "../include/camera.h"
#include "../include/physics.h"
#include "../include/graphics.h"
#include "../include/space.h"

#define USECD

// START OVERLAY
extern u_long load_all_overlays_here;
extern u_long __lvl0_end;
extern u_long __lvl1_end;
u_long overlaySize = 0;

#include "../levels/level0.h"
#include "../levels/level1.h"

// Levels
u_char level = 1;
u_short levelWas = 0;
u_short levelHasChanged = 0;
// Overlay
static char* overlayFile;
// Display and draw environments, double buffered
DISPENV disp[2];
DRAWENV draw[2];
//~ // OT for BG/FG discrimination
u_long otdisc[2][OT2LEN] = {0};
// Main OT
u_long      ot[2][OTLEN]  = {0};                // Ordering table (contains addresses to primitives)
char    primbuff[2][PRIMBUFFLEN] = {0};         // Primitive list // That's our prim buffer
int         primcnt=0;                      // Primitive counter
char * nextpri = primbuff[0];                   // Primitive counter
char            db  = 0;                        // Current buffer counter
// Lighting
CVECTOR BGc = {128, 128, 128, 0};               // Default Far color - This can be set in each level.
VECTOR BKc = {128, 128, 128, 0};                // Back color   
VECTOR FC = FOG_COLOR;                     // Far (Fog) color   
SVECTOR lgtang = {0, 0, 0};
MATRIX rotlgt, light;
short vs;
CAMERA camera = {0};
// Physics
u_long time = 0;
u_long timeS = 0;
//Pads
Controller_Buffer controllers[2];   // Buffers for reading controllers
Controller_Data theControllers[8];  // Processed controller data
int pressed = 0;
u_short timer = 0;
// Cam stuff 
int camMode = ACTOR;                // Cam mode, see defines.h, l.6
VECTOR angle     = {250,0,0,0};
VECTOR angleCam  = {0,0,0,0};
int dist      = 150; 
int lerping    = 0;
short curCamAngle = 0;
// Actor's forward vector (used for dualshock)
VECTOR fVecActor = {0,0,0,0};
u_long triCount = 0;
LEVEL curLvl = {0};
LEVEL * loadLvl;
// Callback variables
u_short lastPad;
int lerpValues[4096 >> 7];
short cursor = 0;
short angleCamTimer = 0;
short forceApplied = 0;
// Callback function is used for pads
void callback();
int main() {
    // Set matrices pointers to scratchpad 
    camera.mat = dc_camMat;
    camera.pos = dc_camPos;
    camera.rot = dc_camRot;
    // Load level file according to level, l.39
    if ( level == 0 ){
        overlayFile = "\\level0.bin;1";
        overlaySize = __lvl0_end;
        loadLvl     = &level0;
    } else if ( level == 1) {
        overlayFile = "\\level1.bin;1";
        overlaySize = __lvl1_end;
        loadLvl     = &level1;
    }
    // Load overlay from cd
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
    // Copy light matrices / vector to scratchpad
    setDCLightEnv(curLvl.cmat, curLvl.lgtmat, &lgtang);
    // Init dislay, Gte..
    init(disp, draw, db, curLvl.BGc, curLvl.BKc, &FC);
    // Init Pads
    InitPAD(controllers[0].pad, 34, controllers[1].pad, 34);
    StartPAD();
    // Generate Cos table
    generateTable();
    // Set function 'callback' on v-sync
    VSyncCallback(callback);
    // Load textures
    for (int k = 0; k < *curLvl.meshes_length ; k++){
        // Check data exists
        if (curLvl.meshes[k]->tim_data){
            LoadTexture(curLvl.meshes[k]->tim_data, curLvl.meshes[k]->tim);
        }
    }
    // Load current BG if exists
    if (curLvl.camPtr->tim_data){
        LoadTexture(curLvl.camPtr->tim_data, curLvl.camPtr->BGtim);
    }
    // Physics/collisions
    short physics = 1;
    VECTOR col = {0};
    // Cam stuff 
    VECTOR posToActor    = {0, 0, 0, 0};      // position of camera relative to actor    
    VECTOR camAngleToAct = {0, 0, 0, 0};      // rotation angles for the camera to point at actor
    // Animation timing
    // time % timediv == animation time
    // Time divisor
    short timediv = 1;
    // Animation time, see l.206
    int atime = 0;
    // Polycount
    for (int k = 0; k < *curLvl.meshes_length; k++){
            triCount += curLvl.meshes[k]->tmesh->len;
    }
    // Set camera starting pos
    setCameraPos(&camera, &curLvl.camPtr->campos->pos, &curLvl.camPtr->campos->rot);
    // Find curCamAngle if using pre-calculated BGs
    if (camMode == 2) {                              
        if (curLvl.camPtr->tim_data){
            curCamAngle = 1;
        }
    }
    // Main loop
    while ( VSync(VSYNC) ) {
        //~ timeS = VSync(-1) / 60;
        // Check if level has changed
        // TODO : Proper level system / loader
        if ( levelWas != level ){
            // If so, load other level
            switch ( level ){
                case 0:
                    overlayFile = "\\level0.bin;1";
                    overlaySize = __lvl0_end;
                    loadLvl     = &level0;
                    // Copy light matrices / vector to scratchpad
                    break;
                case 1:
                    overlayFile = "\\level1.bin;1";
                    overlaySize = __lvl1_end;
                    loadLvl     = &level1;
                    // Copy light matrices / vector to scratchpad

                    break;
                default:
                    overlayFile = "\\level0.bin;1";
                    loadLvl     = &level0;
                    break;
            }
            #ifdef USECD
              LoadLevelCD( overlayFile, &load_all_overlays_here );
            #endif
            SwitchLevel( &curLvl, loadLvl);
            setLightEnv(draw, curLvl.BGc, curLvl.BKc);
            levelWas = level;
        }
        FntPrint("Ovl:%s\nLvl : %x\nLvl: %d %d \n%x", overlayFile, &level, level, levelWas, loadLvl);
        // Clear the main OT
        ClearOTagR(otdisc[db], OT2LEN);
        // Clear Secondary OT
        ClearOTagR(ot[db], OTLEN);
        time ++;
        // atime is used for animations timing
        timediv = 1;
        // If timediv is > 1, animation time will be slower 
        if (time % timediv == 0){
            atime ++;
        }
        // Angle between camera and actor
        // using atantable (faster)
        camAngleToAct.vy = (patan(-posToActor.vx, -posToActor.vz) / 16) - 3076 ;
        camAngleToAct.vx = patan(dist, posToActor.vy) >> 4;
        // Find Actor's forward vector
        setVector(  &fVecActor,
                    curLvl.actorPtr->pos.vx + (nsin(curLvl.actorPtr->rot.vy/2)),
                    curLvl.actorPtr->pos.vy, 
                    curLvl.actorPtr->pos.vz - (ncos(curLvl.actorPtr->rot.vy/2))
                );
    // Camera modes
        if(camMode != 2) {
            camera.rot->vy = camAngleToAct.vy;
            // using csin/ccos, no need for theta
            camera.rot->vx = camAngleToAct.vx;   
        }
        if(camMode < 4 ) {
            lerping = 0;
        }
        // Camera follows actor
        if(camMode == 0) {
            dist = 200;
            setVector(camera.pos, -(camera.x/ONE), -(camera.y/ONE), -(camera.z/ONE));
            angle.vy = -(curLvl.actorPtr->rot.vy / 2) + angleCam.vy;
            // Camera horizontal and vertical position
            getCameraZY(&camera.z, &camera.y, curLvl.actorPtr->pos.vz, curLvl.actorPtr->pos.vy, angle.vx, dist);
            getCameraXZ(&camera.x, &camera.z, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vz, angle.vy, dist);
        }
        // Camera rotates continuously around actor
        if (camMode == 1) {
            // Set distance between cam and actor
            dist = 150;
            // Set camera position
            setVector(camera.pos, -(camera.x/ONE), 100, -(camera.z/ONE));
            // Find new camera position
            getCameraXZ(&camera.x, &camera.z, curLvl.actorPtr->pos.vx, curLvl.actorPtr->pos.vz, angle.vy, dist);
            // Set rotation amount
            angle.vy += 10;
        }
        // Fixed Camera with actor tracking
        if (camMode == 3) {                              
            // Using precalc sqrt
            dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz) );
            // Set camera position
            setVector(camera.pos, 190, 100, 180);
        }
        // Fixed Camera angle
        if (camMode == 2) {                              
            // If BG images exist
            if (curLvl.camPtr->tim_data){
                 checkLineW( &curLvl.camAngles[ curCamAngle ]->fw.v3, &curLvl.camAngles[ curCamAngle ]->fw.v2, curLvl.actorPtr);
                if ( curLvl.camAngles[ curCamAngle ]->fw.v0.vx ) {
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
            setCameraPos(&camera, &curLvl.camPtr->campos->pos, &curLvl.camPtr->campos->rot);
        }
        // Flyby mode with LERP from camStart to camEnd
        if (camMode == 4) {                               
            // If key pos exist for camera
            if (curLvl.camPath->len) {
                // Lerping sequence has not begun
                if (!lerping){
                    // Set cam start position ( first key pos )
                    copyVector(camera.pos, &curLvl.camPath->points[curLvl.camPath->cursor]);
                    // Lerping sequence is starting
                    lerping = 1;
                    // Set cam pos index to 0
                    curLvl.camPath->pos = 0;
                    }
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                // Fixed point precision 2^12 == 4096
                int precision = 12;
                setVector( camera.pos, 
                           lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vx << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vx << precision, curLvl.camPath->pos << precision) >> precision,
                           lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vy << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vy << precision, curLvl.camPath->pos << precision) >> precision,
                           lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vz << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vz << precision, curLvl.camPath->pos << precision) >> precision
                );
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
                    copyVector(camera.pos, &curLvl.camPath->points[curLvl.camPath->cursor]);
                    // Lerping sequence is starting
                    lerping = 1;
                    // Set cam pos index to 0
                    curLvl.camPath->pos = 0;
                    }
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor.vx * posToActor.vx ) + (posToActor.vz * posToActor.vz));
                // Fixed point precision 2^12 == 4096
                short precision = 12;
                setVector(  camera.pos, 
                            lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vx << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vx << precision, curLvl.camPath->pos << precision) >> precision,
                            lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vy << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vy << precision, curLvl.camPath->pos << precision) >> precision,
                            lerpD(curLvl.camPath->points[curLvl.camPath->cursor].vz << precision, curLvl.camPath->points[curLvl.camPath->cursor+1].vz << precision, curLvl.camPath->pos << precision) >> precision
                );
                // Ony move cam if position is between first curLvl.camPath->vx and last curLvl.camPath->vx
                if ( camAngleToAct.vy < -50 && camera.pos->vx > curLvl.camPath->points[curLvl.camPath->len - 1].vx ) {  
                    // Clamp curLvl.camPath position to cameraSpeed
                    curLvl.camPath->pos += dist < cameraSpeed ? 0 : cameraSpeed ;
                }
                if ( camAngleToAct.vy > 50 && camera.pos->vx > curLvl.camPath->points[curLvl.camPath->cursor].vx ) {  
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
                     if ( curLvl.meshes[k]->isRigidBody == 1 ) {
                        applyAcceleration( curLvl.meshes[k]->body );
                        // Get col between actor and level
                        if ( curLvl.meshes[k]->isActor ){
                            checkBodyCol( curLvl.meshes[k]->body , curLvl.levelPtr->body );
                        }
                        // Get col between props and level
                        if ( curLvl.meshes[k]->isProp ){
                            checkBodyCol( curLvl.meshes[k]->body , curLvl.meshes[k]->node->plane->body );
                        }
                        // Only evaluate collision if actor is on same plane as prop
                        if ( curLvl.curNode == curLvl.propPtr->node ){
                            // Get col between actor and props
                            col = getExtCollision( *curLvl.meshes[k]->body, *curLvl.propPtr->body );
                            if (col.vx && col.vz ) {
                                setVector( &curLvl.propPtr->body->velocity,
                                           curLvl.meshes[k]->body->velocity.vx,
                                           0,
                                           curLvl.meshes[k]->body->velocity.vz
                                          );
                                // If prop is spherical, make it roll
                                applyAngMom(curLvl);
                            }
                            
                        }
                        // Synchronize mesh to body position
                        copyVector(&curLvl.meshes[k]->pos, &curLvl.meshes[k]->body->position);
                    }
                    setVector(&curLvl.meshes[k]->body->velocity, 0, 0, 0);
                }
        }
        // Get actor's screen coordinates (used with fixed BGs)
        if ( (camMode == 2) && (curLvl.camPtr->tim_data ) ) {
            worldToScreen( &curLvl.actorPtr->pos, &curLvl.actorPtr->pos2D );
        }
    // Camera setup 
        // Get position of cam relative to actor
        addVector2(&curLvl.actorPtr->pos, camera.pos, &posToActor);
            
    // Polygon drawing
        if (curLvl.curNode){
            if ( (camMode == 2) && (curLvl.camPtr->tim_data ) ) {
                drawBG(curLvl.camPtr, &nextpri, otdisc[db], &db);
                // Loop on camAngles
                for ( int mesh = 0 ; mesh < curLvl.camAngles[ curCamAngle ]->index; mesh ++ ) {
                    enlightMesh(&curLvl, curLvl.camAngles[curCamAngle]->objects[mesh], dc_lgtangp);
                    transformMesh(&camera, curLvl.camAngles[curCamAngle]->objects[mesh]);
                    drawPoly(curLvl.camAngles[curCamAngle]->objects[mesh], atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                }
            }
            else {
                // Draw current node's plane
                drawPoly( curLvl.curNode->plane, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                // Draw surrounding planes 
                for ( int sibling = 0; sibling < curLvl.curNode->siblings->index; sibling++ ) {
                    drawPoly(curLvl.curNode->siblings->list[ sibling ]->plane, atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                }
                // Draw adjacent planes's children
                for ( int sibling = 0; sibling < curLvl.curNode->siblings->index; sibling++ ) {
                    for ( int object = 0; object < curLvl.curNode->siblings->list[ sibling ]->objects->index; object++ ) {
                        long t = 0;
                        enlightMesh(&curLvl, curLvl.curNode->siblings->list[ sibling ]->objects->list[ object ], dc_lgtangp);
                        transformMesh(&camera, curLvl.curNode->siblings->list[ sibling ]->objects->list[ object ]);
                        drawPoly( curLvl.curNode->siblings->list[ sibling ]->objects->list[ object ], atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                    }
                }
                // Draw current plane children
                for ( int object = 0; object < curLvl.curNode->objects->index; object++ ) {
                    enlightMesh(&curLvl, curLvl.curNode->objects->list[ object ], dc_lgtangp);
                    transformMesh(&camera, curLvl.curNode->objects->list[ object ]);
                    drawPoly( curLvl.curNode->objects->list[ object ], atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                }
                // Draw rigidbodies
                for ( int object = 0; object < curLvl.curNode->rigidbodies->index; object++ ) {
                    enlightMesh(&curLvl, curLvl.curNode->rigidbodies->list[ object ], dc_lgtangp);
                    transformMesh(&camera, curLvl.curNode->rigidbodies->list[ object ]);
                    drawPoly( curLvl.curNode->rigidbodies->list[ object ], atime, &camMode, &nextpri, ot[db], &db, &draw[db]);
                }
            }    
        }
        // Update global light matrix - use scratchpad 
        RotMatrix_gte(dc_lgtangp, dc_lgtmatp);
        gte_MulMatrix0(dc_lvllgtmatp, dc_lgtmatp, dc_lgtmatp);
        gte_SetLightMatrix(dc_lgtmatp);
        // Set camera
        applyCamera(&camera);
        // Add secondary OT to main OT
        AddPrims(otdisc[db], ot[db] + OTLEN - 1, ot[db]);
        FntPrint("Time    : %d\n", time);
        FntPrint("#Tri    : %d\n", triCount);
        FntPrint("# : %d %d\n", sizeof(VECTOR), sizeof(CAMERA) );
        FntFlush(-1);
        display( &disp[db], &draw[db], otdisc[db], primbuff[db], &nextpri, &db);
    }
    return 0;
}
void callback() {
    // Pad 1
    read_controller( &theControllers[0], &controllers[0].pad[0], 0 );  // Read controllers
    // Pad 2
    read_controller( &theControllers[1], &controllers[1].pad[0], 1 );
    u_char PADL = ~theControllers[0].button1;
    u_char PADR = ~theControllers[0].button2;
    //~ static u_short lastPad;
    //~ static int lerpValues[4096 >> 7];
    //~ static short cursor = 0;
    //~ static short angleCamTimer = 0;
    //~ static short forceApplied = 0;
    int div = 32;

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
                setCameraPos(&camera, &curLvl.camPtr->campos->pos, &curLvl.camPtr->campos->rot);
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
        dc_lgtangp->vy += 32;
    }
    if ( PADR & PadShldL1 ) {
        dc_lgtangp->vz += 32;
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
            curLvl.actorPtr->body->gForce.vz = getVectorTo(fVecActor, curLvl.actorPtr->pos).vz *  (128 - theControllers[0].analog3 ) >> 14 ;
            curLvl.actorPtr->body->gForce.vx = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vx * (128 - theControllers[0].analog3 ) >> 14 ;
            lastPad = PADL;
        }
        // Analog stick L down
        if ( theControllers[0].analog3 > 168 && theControllers[0].analog3 <= 255 ) {
            curLvl.actorPtr->body->gForce.vz = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vz *  ( theControllers[0].analog3 - 128 ) >> 14 ;
            curLvl.actorPtr->body->gForce.vx = getVectorTo(fVecActor, curLvl.actorPtr->pos).vx * ( theControllers[0].analog3 - 128 ) >> 14 ;
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
        curLvl.actorPtr->body->gForce.vz = getVectorTo(fVecActor, curLvl.actorPtr->pos).vz >> 7 ;
        curLvl.actorPtr->body->gForce.vx = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vx >> 7;
        lastPad = PADL;
    }
    if ( !(PADL & PadUp) && lastPad & PadUp) {
        curLvl.actorPtr->body->gForce.vz = 0;
        curLvl.actorPtr->body->gForce.vx = 0;
        lastPad = PADL;
    }
    if ( PADL & PadDown ) {
        curLvl.actorPtr->body->gForce.vz = -getVectorTo(fVecActor, curLvl.actorPtr->pos).vz >> 7 ;
        curLvl.actorPtr->body->gForce.vx = getVectorTo(fVecActor, curLvl.actorPtr->pos).vx >> 7 ;
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
        #ifndef USECD
            printf("load:%p:%08x:%s", &load_all_overlays_here, &level, overlayFile);
        //~ PCload( &load_all_overlays_here, &levelHasChanged, overlayFile );
        #endif
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
        if ( theControllers[0].analog0 >= 0 && theControllers[0].analog0 < (128 - DS_DZ/2) ) {
            angleCam.vy -= ( 16 * ( 128 - theControllers[0].analog0 ) ) >> 7 ;
            angleCamTimer = 120;
        }
        if ( theControllers[0].analog0 > (128 + DS_DZ/2)  && theControllers[0].analog0 <= 255 ) {
            angleCam.vy += ( 16 * ( theControllers[0].analog0 - 128 ) ) >> 7 ;
            angleCamTimer = 120;
        }
        if ( theControllers[0].analog0 >= 0 && theControllers[0].analog0 < (128 - DS_DZ/2) ) {
            angleCam.vy -= ( 16 * ( 128 - theControllers[0].analog0 ) ) >> 7 ;
            angleCamTimer = 120;
        }
        if ( theControllers[0].analog0 > (128 + DS_DZ/2) && theControllers[0].analog0 <= 255) {
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
    // Print controller infos
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
