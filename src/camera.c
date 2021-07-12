#include "../include/psx.h"
#include "../include/camera.h"
#include "../include/math.h"
#include "../include/physics.h"
#include "../include/space.h"

void getCameraXZ(int * x, int * z, int actorX, int actorZ, int angle, int distance) {
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj    
    *x = (actorX << 12) + (distance * nsin(angle));
    *z = (actorZ << 12) - (distance * ncos(angle));
};
void getCameraXZY(int * x, int * z, int * y, int actorX, int actorZ, int actorY, int angle, int angleX, int distance) {
    // Using Nic's Costable : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
    //                        https://godbolt.org/z/q6cMcj    
    *x = (actorX << 12) + (distance * nsin(angle));
    *z = (actorZ << 12) - (distance * ( ( ncos(angle) * ncos(angleX) ) >> 12 ) );
    *y = (actorY << 12) - (distance * nsin(angleX));
};
void getCameraZY( int * z, int * y, int actorZ, int actorY, int angleX, int distance) {
    *z = (actorZ << 12) - (distance * ncos(angleX));
    *y = (actorY << 12) - (distance * nsin(angleX));
};
// @Will : you might want to use sin/cos to move the camera in a circle but you could do that by moving it along it’s tangent and then clamping the distance
void applyCamera( CAMERA * cam ) {
    //~ VECTOR vec;                                         // Vector that holds the output values of the following instructions
    RotMatrix_gte(dc_camRot, dc_camMat);                // Convert rotation angle in psx units (360° == 4096) to rotation matrix)
    gte_ApplyMatrix(dc_camMat, dc_camPos, dc_wrklvector);          // Multiply matrix by vector pos and output to vec
    TransMatrix(dc_camMat, dc_wrklvector);                       // Apply transform vector
    gte_SetRotMatrix(dc_camMat);                            // Set Rotation matrix
    gte_SetTransMatrix(dc_camMat);                          // Set Transform matrix
};
void setCameraPos( CAMERA * camera, SVECTOR * pos, SVECTOR * rot ) {
    copyVector(camera->pos, pos);
    copyVector(camera->rot, rot);
};
void setCameraMode(LEVEL * curLvl, CAMERA * camera, VECTOR * camAngleToAct, VECTOR * posToActor, VECTOR * angle, VECTOR * angleCam, short curCamAngle, int camMode, int * lerping){
   int dist = 0;
   short cameraSpeed = 40;

   //~ if(camMode != 2) {
    //~ camera->rot->vy = camAngleToAct->vy;
    //~ // using csin/ccos, no need for theta
    //~ camera->rot->vx = camAngleToAct->vx;   
    //~ }
    if(camMode < 4 ) {
        *lerping = 0;
    }
    switch (camMode){
        // Camera follows actor
        case 0 :
            dist = 200;
            setVector(camera->pos, -(camera->x/ONE), -(camera->y/ONE), -(camera->z/ONE));
            angle->vy = -(curLvl->actorPtr->rot.vy / 2) + angleCam->vy;
            // Camera horizontal and vertical position
            getCameraZY(&camera->z, &camera->y, curLvl->actorPtr->pos.vz, curLvl->actorPtr->pos.vy, angle->vx, dist);
            getCameraXZ(&camera->x, &camera->z, curLvl->actorPtr->pos.vx, curLvl->actorPtr->pos.vz, angle->vy, dist);
            break;
            
        // Camera rotates continuously around actor
        case 1 :
            // Set distance between cam and actor
            dist = 150;
            // Set camera position
            setVector(camera->pos, -(camera->x/ONE), 100, -(camera->z/ONE));
            // Find new camera position
            getCameraXZ(&camera->x, &camera->z, curLvl->actorPtr->pos.vx, curLvl->actorPtr->pos.vz, angle->vy, dist);
            // Set rotation amount
            angle->vy += 10;
            break;
            
         // Fixed Camera angle
        case 2 :                          
            // If BG images exist
            if (curLvl->camPtr->tim_data){
                checkLineW( &curLvl->camAngles[ curCamAngle ]->fw.v3, &curLvl->camAngles[ curCamAngle ]->fw.v2, curLvl->actorPtr);
                if ( curLvl->camAngles[ curCamAngle ]->fw.v0.vx ) {
                    // If actor in camAngle->fw area of screen
                    if ( checkLineW( &curLvl->camAngles[ curCamAngle ]->fw.v3, &curLvl->camAngles[ curCamAngle ]->fw.v2, curLvl->actorPtr) == -1  && 
                         ( checkLineW( &curLvl->camAngles[ curCamAngle ]->bw.v2, &curLvl->camAngles[ curCamAngle ]->bw.v3, curLvl->actorPtr) >= 0 
                           ) 
                       ) { 
                        if (curCamAngle < 5) {
                            curCamAngle++;
                            curLvl->camPtr = curLvl->camAngles[ curCamAngle ];
                            LoadTexture(curLvl->camPtr->tim_data, curLvl->camPtr->BGtim);
                        }
                    }
                }
                if ( curLvl->camAngles[ curCamAngle ]->bw.v0.vx ) {
                    // If actor in camAngle->bw area of screen
                    if ( checkLineW( &curLvl->camAngles[ curCamAngle ]->fw.v3, &curLvl->camAngles[ curCamAngle ]->fw.v2, curLvl->actorPtr) >= 0  && 
                         checkLineW( &curLvl->camAngles[ curCamAngle ]->bw.v2, &curLvl->camAngles[ curCamAngle ]->bw.v3, curLvl->actorPtr) == -1 
                       ) {
                        if (curCamAngle > 0) {
                            curCamAngle--;
                            curLvl->camPtr = curLvl->camAngles[ curCamAngle ];
                            LoadTexture(curLvl->camPtr->tim_data, curLvl->camPtr->BGtim);
                        }
                    }
                }
                worldToScreen( &curLvl->actorPtr->pos, &curLvl->actorPtr->pos2D );
            }
            setCameraPos(camera, &curLvl->camPtr->campos->pos, &curLvl->camPtr->campos->rot);
            break;
            
        // Fixed Camera with actor tracking
        case 3 :
            // Using precalc sqrt
            dist = psqrt( (posToActor->vx * posToActor->vx ) + (posToActor->vz * posToActor->vz) );
            // Set camera position
            setVector(camera->pos, 190, 100, 180);
            break;
        // Flyby mode with LERP from camStart to camEnd
        case 4 :                        
            // If key pos exist for camera
            if (curLvl->camPath->len) {
                // Lerping sequence has not begun
                if (!lerping){
                    // Set cam start position ( first key pos )
                    copyVector(camera->pos, &curLvl->camPath->points[curLvl->camPath->cursor]);
                    // Lerping sequence is starting
                    *lerping = 1;
                    // Set cam pos index to 0
                    curLvl->camPath->pos = 0;
                    }
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor->vx * posToActor->vx ) + (posToActor->vz * posToActor->vz));
                // Fixed point precision 2^12 == 4096
                int precision = 12;
                setVector( camera->pos, 
                           lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vx << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vx << precision, curLvl->camPath->pos << precision) >> precision,
                           lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vy << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vy << precision, curLvl->camPath->pos << precision) >> precision,
                           lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vz << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vz << precision, curLvl->camPath->pos << precision) >> precision
                );
                // Linearly increment the lerp factor
                curLvl->camPath->pos += 20;
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (curLvl->camPath->pos > (1 << precision) ){
                    curLvl->camPath->pos = 0;
                    curLvl->camPath->cursor ++;
                }
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( curLvl->camPath->cursor == curLvl->camPath->len - 1 ){
                    lerping = 0;
                    curLvl->camPath->cursor = 0;
                }
            } else { 
                // if no key pos exists, switch to next camMode
                camMode ++; 
            }
            break;
        // Camera "on a rail" - cam is tracking actor, and moving with constraints on all axis
        case 5 :
            // track actor. If theta (actor/cam rotation angle) is above or below an arbitrary angle, 
            // move cam so that the angle doesn't increase/decrease anymore.
            if (curLvl->camPath->len) {
            // Lerping sequence has not begun
                if (!lerping){
                    // Set cam start position ( first key pos )
                    copyVector(camera->pos, &curLvl->camPath->points[curLvl->camPath->cursor]);
                    // Lerping sequence is starting
                    *lerping = 1;
                    // Set cam pos index to 0
                    curLvl->camPath->pos = 0;
                    }
                // Pre calculated sqrt ( see psqrt() )
                dist = psqrt( (posToActor->vx * posToActor->vx ) + (posToActor->vz * posToActor->vz));
                // Fixed point precision 2^12 == 4096
                short precision = 12;
                setVector(  camera->pos, 
                            lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vx << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vx << precision, curLvl->camPath->pos << precision) >> precision,
                            lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vy << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vy << precision, curLvl->camPath->pos << precision) >> precision,
                            lerpD(curLvl->camPath->points[curLvl->camPath->cursor].vz << precision, curLvl->camPath->points[curLvl->camPath->cursor+1].vz << precision, curLvl->camPath->pos << precision) >> precision
                );
                // Ony move cam if position is between first curLvl->camPath->vx and last curLvl->camPath->vx
                if ( camAngleToAct->vy < -50 && camera->pos->vx > curLvl->camPath->points[curLvl->camPath->len - 1].vx ) {  
                    // Clamp curLvl->camPath position to cameraSpeed
                    curLvl->camPath->pos += dist < cameraSpeed ? 0 : cameraSpeed ;
                }
                if ( camAngleToAct->vy > 50 && camera->pos->vx > curLvl->camPath->points[curLvl->camPath->cursor].vx ) {  
                    curLvl->camPath->pos -= dist < cameraSpeed ? 0 : cameraSpeed;
                }
                // If camera has reached next key pos, reset pos index, move cursor to next key pos
                if (curLvl->camPath->pos > (1 << precision) ){
                    curLvl->camPath->pos = 0;
                    curLvl->camPath->cursor ++;
                } 
                if (curLvl->camPath->pos < -100 ){
                    curLvl->camPath->pos = 1 << precision;
                    curLvl->camPath->cursor --;
                }                   
                // Last key pos is reached, reset cursor to first key pos, lerping sequence is over
                if ( curLvl->camPath->cursor == curLvl->camPath->len - 1 || curLvl->camPath->cursor < 0 ){
                    *lerping = 0;
                    curLvl->camPath->cursor = 0;
                }
            } else { 
                // if no key pos exists, switch to next camMode
                camMode ++;
            }
            break;
    }
};
