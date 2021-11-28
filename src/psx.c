#include <psx.h>
#include <sound.h>


void setDCLightEnv(MATRIX * curLevelCMat, MATRIX * curLevelLgtMat, SVECTOR * curLevelLgtAng){
    memcpy( dc_lvlcmatp, curLevelCMat, sizeof(MATRIX));
    memcpy( dc_lvllgtmatp, curLevelLgtMat, sizeof(MATRIX));
    memcpy( dc_lgtangp, curLevelLgtAng, sizeof(SVECTOR));
};

void setLightEnv(DRAWENV draw[2], CVECTOR * BGc, VECTOR * BKc){
    // Set Draw area color
    setRGB0(&draw[0], BGc->r, BGc->g, BGc->b);
    setRGB0(&draw[1], BGc->r, BGc->g, BGc->b);
    // Set Farcolor from here
    SetFarColor( BGc->r, BGc->g, BGc->b );
    // Set Ambient color
    SetBackColor( BKc->vx, BKc->vy, BKc->vz );
    // Set Light matrix
    SetColorMatrix(dc_lvlcmatp);
};

void init(DISPENV disp[2], DRAWENV draw[2], short db, CVECTOR * BGc, VECTOR * BKc, VECTOR * FC) {
    ResetCallback();
    // Init pad
    //~ PadInit(0);
    //~ InitPAD(controllers[0].pad, 34, controllers[1].pad, 34);
    //~ StartPAD();
    // Init SPU
    SpuInit();
    // Reset the GPU
    ResetGraph( 0 );
    // Initialize and setup the GTE
    InitGeom();
    SetGeomOffset( CENTERX, CENTERY );        // x, y offset
    SetGeomScreen( FOV );                     // Distance between eye and screen  - Camera FOV
    SetDispMask(1);
    // Set the display and draw environments
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    // If PAL , add 8 pix vertical offset ((256 - 240) /2)
    if ( VMODE ) {
        SetVideoMode(MODE_PAL);
        disp[0].screen.y += 8;
        disp[1].screen.y += 8;
    }
    // Set Draw area color
    setLightEnv(draw, BGc, BKc);
    // Set Draw area clear flag
    draw[0].isbg = 1;
    draw[1].isbg = 1;
    // Set the disp and draw env
    PutDispEnv(&disp[db]);
    PutDrawEnv(&draw[db]);
    // Init font system
    FntLoad(FNT_VRAM_X, FNT_VRAM_Y);
    FntOpen( FNT_SCR_X,
             FNT_SCR_Y, 
             FNT_SCR_W,
             FNT_SCR_H,
             FNT_SCR_BG,
             FNT_SCR_MAX_CHAR
            );
    SetFarColor(FC->vx, FC->vy, FC->vz);
    // TODO : Move this to level files
    SetFogNearFar( FOG_NEAR, FOG_FAR, SCREENXRES );
};
void ScrRst(void){
    RECT scr;
    VSync( 0 ); // Wait for current drawing to finish
    SetDispMask( 0 ); // Set mask to not displayed
    ResetGraph( 1 ); // Cancel current drawing
    // Clear FB
    setRECT(&scr, 0, 0, SCREENXRES, SCREENYRES);
    ClearImage(&scr, CLEAR_COLOR_R, CLEAR_COLOR_G, CLEAR_COLOR_B );
    DrawSync( 0 );
};
void display(DISPENV * disp, DRAWENV * draw, u_long * otdisc, char * primbuff, char ** nextprim, char * db){
    // https://stackoverflow.com/questions/3526503/how-to-set-pointer-reference-through-a-function
    DrawSync(0);
    VSync(VSYNC);  // Using VSync 2 insures constant framerate. 0 makes the fr polycount dependant.
    ResetGraph(1);
    PutDispEnv(disp);
    PutDrawEnv(draw);
    //~ SetDispMask(1);
    // Main OT
    DrawOTag(otdisc + OT2LEN - 1);
    *db = !*db;
    *nextprim = primbuff;
};
void LvlPtrSet(LEVEL * curLevel, LEVEL * level){
    curLevel->BGc = level->BGc;
    curLevel->BKc = level->BKc;
    curLevel->cmat = level->cmat;
    curLevel->lgtmat  = level->lgtmat;
    curLevel->meshes  = level->meshes;
    curLevel->meshes_length = level->meshes_length;
    curLevel->actorPtr = level->actorPtr;
    curLevel->levelPtr = level->levelPtr;
    curLevel->propPtr  = level->propPtr;
    curLevel->camPtr   = level->camPtr;
    curLevel->camPath  = level->camPath;
    curLevel->camAngles = level->camAngles;
    curLevel->curNode   = level->curNode; // Blank
    curLevel->VAG   = level->VAG;
    curLevel->XA   = level->XA;
    curLevel->levelSounds   = level->levelSounds;
    
    //~ curLevel->actorPtr->body = level->actorPtr->body;
    // Move these to drawPoly()
    //~ curLevel->meshPlan  = level->meshPlan;
    //~ FntPrint("%x %x", curLevel->meshes, level->meshes);
};
int LoadLevelCD(const char*const LevelName, u_long * LoadAddress){
    int cdread = 0, cdsync = 1;
    cdread = CdReadFile( (char *)(LevelName), LoadAddress, 0);
    cdsync = CdReadSync(0, 0);
    // return loaded size
    return cdread;
};
void SwitchLevel( LEVEL * curLevel, LEVEL * loadLevel ){
    //~ ScrRst();
    LvlPtrSet( curLevel, loadLevel);
    // XA
    getXAoffset(curLevel);
    // Reload textures
    for (int k = 0; k < *curLevel->meshes_length ; k++){
        // Check data exists
        if (curLevel->meshes[k]->tim_data){
            LoadTexture(curLevel->meshes[k]->tim_data, curLevel->meshes[k]->tim);
        }
    }
    // BG texture
    if (curLevel->camPtr->tim_data){
        LoadTexture(curLevel->camPtr->tim_data, curLevel->camPtr->BGtim);
    }
    // TODO : per-level lgtang
    SVECTOR lgtang = {0,0,0,0};
    // Light environment
    setDCLightEnv(curLevel->cmat, curLevel->lgtmat, &lgtang);
    // Reset physics
    // TODO : put in a function
    copyVector(&curLevel->actorPtr->body->position, &loadLevel->actorPtr->body->position );
    copyVector(&curLevel->actorPtr->pos, &curLevel->actorPtr->body->position);
    copyVector(&curLevel->propPtr->body->position, &loadLevel->propPtr->body->position );
    copyVector(&curLevel->propPtr->pos, &curLevel->propPtr->body->position);
    applyVector( &curLevel->actorPtr->body->position, 0, 100, 0, -=);
    applyVector( &curLevel->actorPtr->body->velocity, 0, 0, 0, =);
    applyVector( &curLevel->actorPtr->body->gForce, 0, 0, 0, =);
    applyVector( &curLevel->propPtr->body->position, 0, 100, 0, -=);
    applyVector( &curLevel->propPtr->body->velocity, 0, 0, 0, =);
    applyVector( &curLevel->propPtr->body->gForce, 0, 0, 0, =);
    
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
};
