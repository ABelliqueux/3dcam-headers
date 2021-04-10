#include "psx.h"

void init(DISPENV disp[2], DRAWENV draw[2], short db, MATRIX * cmat, CVECTOR * BGc, VECTOR * BKc) {
    
    ResetCallback();
    
    // Reset the GPU before doing anything and the controller
	
    ResetGraph(0);

    PadInit(0);
	
	// Initialize and setup the GTE
	
    InitGeom();
	
    SetGeomOffset( CENTERX, CENTERY );        // x, y offset
	
    SetGeomScreen( FOV );            // Distance between eye and screen  - Camera FOV
	
    // Set the display and draw environments
	
    SetDefDispEnv(&disp[0], 0, 0         , SCREENXRES, SCREENYRES);
	
    SetDefDispEnv(&disp[1], 0, SCREENYRES, SCREENXRES, SCREENYRES);
    
	
    SetDefDrawEnv(&draw[0], 0, SCREENYRES, SCREENXRES, SCREENYRES);
	
    SetDefDrawEnv(&draw[1], 0, 0, SCREENXRES, SCREENYRES);
    
    
    // If PAL 
    
    if ( VMODE ) {
        
        SetVideoMode(MODE_PAL);
        
        disp[0].screen.y += 8;
        
        disp[1].screen.y += 8;
    }
	
    // Set Draw area color
    
    setRGB0(&draw[0], BGc->r, BGc->g, BGc->b);
    
    setRGB0(&draw[1], BGc->r, BGc->g, BGc->b);

    // Set Draw area clear flag

    draw[0].isbg = 1;
    
    draw[1].isbg = 1;

    // Set the disp and draw env

    PutDispEnv(&disp[db]);

	PutDrawEnv(&draw[db]);
		
	// Init font system

	FntLoad(FNT_POS_X, FNT_POS_Y);

	FntOpen(16, 90, 240, 180, 0, 512);

    // Lighting setup
    
    SetColorMatrix(cmat);
    
    SetBackColor(BKc->vx,BKc->vy,BKc->vz);
    
    SetFarColor(BGc->r, BGc->g, BGc->b);
    
    SetFogNearFar(1200, 1600, SCREENXRES);
    
};

void display(DISPENV * disp, DRAWENV * draw, u_long * otdisc, char * primbuff, char ** nextprim, char * db){

    // https://stackoverflow.com/questions/3526503/how-to-set-pointer-reference-through-a-function

    //~ //DrawSync(0);
    
    VSync(2);  // Using VSync 2 insures constant framerate. 0 makes the fr polycount dependant.

    ResetGraph(1);

    PutDispEnv(disp);
    
    PutDrawEnv(draw);

    SetDispMask(1);
    
    // Main OT
    DrawOTag(otdisc + OT2LEN - 1);
    
    *db = !*db;

    *nextprim = primbuff;
    
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