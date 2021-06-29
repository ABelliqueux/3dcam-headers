#include "../include/graphics.h"
#include "../include/math.h"
void enlightMesh(LEVEL * curLvl, MESH * mesh, SVECTOR * lgtang){
    // Update light rotation on actor
    MATRIX      rotlgt, rotmesh, light; 
    // Find rotmat from actor angle
    RotMatrix_gte(&mesh->rot, &rotmesh);     
    RotMatrix_gte(lgtang, &rotlgt);
    MulMatrix0(&rotmesh, &rotlgt, &rotlgt);
    MulMatrix0(curLvl->lgtmat, &rotlgt, &light);
    SetLightMatrix(&light);
};
void transformMesh(CAMERA * camera, MESH * mesh){
    MATRIX mat;
    // Apply rotation matrix
    RotMatrix_gte(&mesh->rot, &mat);            
    // Apply translation matrix
    TransMatrix(&mat, &mesh->pos);
    // Compose matrix with cam
    CompMatrix(&camera->mat, &mat, &mat);  
    // Set default rotation and translation matrices
    SetRotMatrix(&mat);                             
    SetTransMatrix(&mat);                           
//~ }
};
//TODO : Break this monster in tiny bits ?
void drawPoly(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw) {
    long nclip, t = 0;
    // FIXME : t is not incremented, thus always 0. It works if the mesh only has tris or quads, but won't work with mixed meshes.
    // mesh is POLY_GT3 ( triangle )
    for (int i = 0; i < (mesh->totalVerts);) {
        if (mesh->index[t].code == 4) {
            t = drawTri(mesh, Flag, atime, camMode, nextpri, ot, db, draw, t, i);
            i += 3;
        }
        // If mesh is quad
        if (mesh->index[t].code == 8) {
            t = drawQuad(mesh, Flag, atime, camMode, nextpri, ot, db, draw, t, i);
            i += 4;
        }
    }
};
void set3VertexLerPos(MESH * mesh, long t){
    // Find and set 3 interpolated vertex value
    // TODO : Finish lerp anim implementation
    // Fixed point math precision
    short precision = 12;
    // Vertex 1
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vx].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vx].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    // Vertex 2
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vz].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vz].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    // Vertex 3
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vx = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vx << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vz = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vz << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vy = lerpD( mesh->anim->data[mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[t].order.vy].vy << precision , mesh->anim->data[(mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[t].order.vy].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->anim->cursor += 24 * mesh->anim->dir;
};
void set4VertexLerPos(MESH * mesh, long t){
    // Find and set 4 interpolated vertex value
    short precision = 12;
    // Vertex 1
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vx << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vz << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vx ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vx ].vy << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vx ].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    // Vertex 2
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vx << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vz << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vz ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vz ].vy << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vz ].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    // Vertex 3
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vx << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vx  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vz << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vz  << precision, mesh->anim->cursor << precision)  >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.vy ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.vy ].vy << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.vy ].vy  << precision, mesh->anim->cursor << precision)  >> precision;
    // Vertex 4
    mesh->tmesh->v[ mesh->index[ t ].order.pad ].vx = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vx << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vx  << precision, mesh->anim->cursor << precision) >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.pad ].vz = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vz << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vz  << precision, mesh->anim->cursor << precision) >> precision;
    mesh->tmesh->v[ mesh->index[ t ].order.pad ].vy = lerpD( mesh->anim->data[ mesh->anim->lerpCursor * mesh->anim->nvert + mesh->index[ t ].order.pad ].vy << precision , mesh->anim->data[ (mesh->anim->lerpCursor + 1) * mesh->anim->nvert + mesh->index[ t ].order.pad ].vy  << precision, mesh->anim->cursor << precision) >> precision;
    mesh->anim->cursor += 2 * mesh->anim->dir;
}
long interpolateTri(POLY_GT3 * poly, MESH * mesh, long t, long * Flag){
    long nclip = 0;
     // Ping pong 
     //~ //if (mesh->anim->cursor > 4096 || mesh->anim->cursor < 0){
     //~ //   mesh->anim->dir *= -1;
     //~ //}
     // Find next keyframe 
     if (mesh->anim->cursor > (1 << 12)) {
        // There are still keyframes to interpolate between
        if ( mesh->anim->lerpCursor < mesh->anim->nframes - 1 ) {
            mesh->anim->lerpCursor ++;
            mesh->anim->cursor = 0;
        }
        // We've reached last frame, go back to first frame
        if ( mesh->anim->lerpCursor == mesh->anim->nframes - 1 ) {
            mesh->anim->lerpCursor = 0;
            mesh->anim->cursor = 0;
        }
     }
    // Find and set interpolated vertex value
    set3VertexLerPos(mesh, t);
    // Coord transformation from world space to screen space
    nclip = RotAverageNclip3(
                &mesh->tmesh->v[ mesh->index[t].order.vx ],  
                &mesh->tmesh->v[ mesh->index[t].order.vz ],
                &mesh->tmesh->v[ mesh->index[t].order.vy ],
                ( long* ) &poly->x0, ( long* ) &poly->x1, ( long* ) &poly->x2,
                &mesh->p,
                &mesh->OTz,
                Flag
            );
    return nclip;
};
long interpolateQuad(POLY_GT4 * poly4, MESH * mesh, long t, long * Flag){
    long nclip = 0;
    // ping pong
    //~ if (mesh->anim->cursor > 4096 || mesh->anim->cursor < 0){
       //~ mesh->anim->dir *= -1;
    //~ }
    short precision = 12;
    if ( mesh->anim->cursor > 1<<precision ) {
        if ( mesh->anim->lerpCursor < mesh->anim->nframes - 1 ) {
            mesh->anim->lerpCursor ++;
            mesh->anim->cursor = 0;
        }
        if ( mesh->anim->lerpCursor == mesh->anim->nframes - 1 ) {
            mesh->anim->lerpCursor = 0;
            mesh->anim->cursor = 0;
        }
    }
    // Find and set interpolated vertex value
    set4VertexLerPos(mesh, t);
    // Coord transformations
    nclip = RotAverageNclip4(
                &mesh->tmesh->v[ mesh->index[t].order.pad ],  
                &mesh->tmesh->v[ mesh->index[t].order.vz],
                &mesh->tmesh->v[ mesh->index[t].order.vx ],
                &mesh->tmesh->v[ mesh->index[t].order.vy ],
                ( long* )&poly4->x0, ( long* )&poly4->x1, ( long* )&poly4->x2, ( long* )&poly4->x3,
                &mesh->p,
                &mesh->OTz,
                Flag
            );
    return nclip;
};
void set3Prism(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, int i){
    // Transparency effect :
    // Use current DRAWENV clip as TPAGE instead of regular textures
    ( (POLY_GT3 *) poly )->tpage = getTPage( 2, 0,
                                             0,
                                             256
                                   );
    // Use projected coordinates (results from RotAverage...) as UV coords and clamp them to 0-255,0-224 -> 240 - 16
    setUV3( poly,  
            (poly->x0 < 0 ? 0 : poly->x0 > 255 ? 255 : poly->x0),
            (poly->y0 < 0 ? 0 : poly->y0 > 224 ? 224 : poly->y0), 
            (poly->x1 < 0 ? 0 : poly->x1 > 255 ? 255 : poly->x1), 
            (poly->y1 < 0 ? 0 : poly->y1 > 224 ? 224 : poly->y1), 
            (poly->x2 < 0 ? 0 : poly->x2 > 255 ? 255 : poly->x2), 
            (poly->y2 < 0 ? 0 : poly->y2 > 224 ? 224 : poly->y2)
            );
    
    setRGB0(poly, mesh->tmesh->c[i].r,   mesh->tmesh->c[i].g, mesh->tmesh->c[i].b);
    setRGB1(poly, mesh->tmesh->c[i+2].r, mesh->tmesh->c[i+2].g, mesh->tmesh->c[i+2].b);
    setRGB2(poly, mesh->tmesh->c[i+1].r, mesh->tmesh->c[i+1].g, mesh->tmesh->c[i+1].b);
};
void set4Prism(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, int i){
    ( (POLY_GT4 *) poly4)->tpage = getTPage( 2, 0,
                                             0,
                                             256
                                   );
    // Use projected coordinates
    setUV4( poly4, 
            (poly4->x0 < 0? 0 : poly4->x0 > 255? 255 : poly4->x0), 
            (poly4->y0 < 0? 0 : poly4->y0 > 224? 224 : poly4->y0), 
            (poly4->x1 < 0? 0 : poly4->x1 > 255? 255 : poly4->x1), 
            (poly4->y1 < 0? 0 : poly4->y1 > 224? 224 : poly4->y1), 
            (poly4->x2 < 0? 0 : poly4->x2 > 255? 255 : poly4->x2), 
            (poly4->y2 < 0? 0 : poly4->y2 > 224? 224 : poly4->y2),
            (poly4->x3 < 0? 0 : poly4->x3 > 255? 255 : poly4->x3), 
            (poly4->y3 < 0? 0 : poly4->y3 > 224? 224 : poly4->y3)
    );
    
    setRGB0(poly4, mesh->tmesh->c[i+3].r, mesh->tmesh->c[i+3].g, mesh->tmesh->c[i+3].b);
    setRGB1(poly4, mesh->tmesh->c[i+2].r, mesh->tmesh->c[i+2].g, mesh->tmesh->c[i+2].b);
    setRGB2(poly4, mesh->tmesh->c[i+0].r, mesh->tmesh->c[i+0].g, mesh->tmesh->c[i+0].b);
    setRGB3(poly4, mesh->tmesh->c[i+1].r, mesh->tmesh->c[i+1].g, mesh->tmesh->c[i+1].b);
};
void set3Tex(POLY_GT3 * poly, MESH * mesh, DRAWENV * draw, long t, int i){
    CVECTOR outCol  = { 0,0,0,0 };
    CVECTOR outCol1 = { 0,0,0,0 };
    CVECTOR outCol2 = { 0,0,0,0 };
    // No transparency effect
    // Use regular TPAGE
    ( (POLY_GT3 *) poly )->tpage = getTPage(mesh->tim->mode&0x3, 0,
                                     mesh->tim->prect->x,
                                     mesh->tim->prect->y
    );
    setUV3(poly,  mesh->tmesh->u[i].vx  , mesh->tmesh->u[i].vy   + mesh->tim->prect->y,
                  mesh->tmesh->u[i+2].vx, mesh->tmesh->u[i+2].vy + mesh->tim->prect->y,
                  mesh->tmesh->u[i+1].vx, mesh->tmesh->u[i+1].vy + mesh->tim->prect->y);
                  
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vx ], &mesh->tmesh->c[ i+0 ], mesh->p, &outCol);
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vz ], &mesh->tmesh->c[ i+2 ], mesh->p, &outCol1);
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vy ], &mesh->tmesh->c[ i+1 ], mesh->p, &outCol2);
    setRGB0(poly, outCol.r, outCol.g  , outCol.b);
    setRGB1(poly, outCol1.r, outCol1.g, outCol1.b);
    setRGB2(poly, outCol2.r, outCol2.g, outCol2.b);
};
void set4Tex(POLY_GT4 * poly4, MESH * mesh, DRAWENV * draw, long t, int i){
    
    CVECTOR outCol  = {0,0,0,0};
    CVECTOR outCol1 = {0,0,0,0};
    CVECTOR outCol2 = {0,0,0,0};
    CVECTOR outCol3 = {0,0,0,0};
    // Use regular TPAGE
    ( (POLY_GT4 *) poly4)->tpage = getTPage( 
                                    mesh->tim->mode&0x3, 0,
                                    mesh->tim->prect->x,
                                    mesh->tim->prect->y
                                 );
    // Use model UV coordinates
    setUV4( poly4, 
            mesh->tmesh->u[i+3].vx, mesh->tmesh->u[i+3].vy + mesh->tim->prect->y,
            mesh->tmesh->u[i+2].vx, mesh->tmesh->u[i+2].vy + mesh->tim->prect->y,
            mesh->tmesh->u[i+0].vx, mesh->tmesh->u[i+0].vy + mesh->tim->prect->y,
            mesh->tmesh->u[i+1].vx, mesh->tmesh->u[i+1].vy + mesh->tim->prect->y
    );
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.pad ] , &mesh->tmesh->c[ i+3 ], mesh->p, &outCol);
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vz ]  , &mesh->tmesh->c[ i+2 ], mesh->p, &outCol1);
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vx ]  , &mesh->tmesh->c[ i+0 ], mesh->p, &outCol2);
    NormalColorDpq(&mesh->tmesh->n[ mesh->index[t].order.vy ]  , &mesh->tmesh->c[ i+1 ], mesh->p, &outCol3);
    setRGB0(poly4, outCol.r, outCol.g  , outCol.b);
    setRGB1(poly4, outCol1.r, outCol1.g, outCol1.b);
    setRGB2(poly4, outCol2.r, outCol2.g, outCol2.b);
    setRGB3(poly4, outCol3.r, outCol3.g, outCol3.b);
};
void set4Subdiv(void){
    //(MESH * mesh, char ** nextpri, u_long * ot, long t, int i)
    // FIXME : Polygon subdiv - is it working ?
    // In main.c, l.141
    //~ DIVPOLYGON4 div4 = { 0 };
    //~ div4.pih = SCREENXRES;
    //~ div4.piv = SCREENYRES;
    //~ div4.ndiv = 2;
    //~ long OTc = 0;
    //~ DIVPOLYGON3 div3 = { 0 };
    //~ div3.pih = SCREENXRES;
    //~ div3.piv = SCREENYRES;
    //~ div3.ndiv = 1;
    //
    //~ long OTc = *mesh->OTz >> 4;
    //~ FntPrint("OTC:%d", OTc);
    //~ if (OTc < 4) {
        //~ if (OTc > 1) div4.ndiv = 1; else div4.ndiv = 2;
            //~ DivideGT4(
                //~ // Vertex coord
                //~ &mesh->tmesh->v[ mesh->index[t].order.pad ],  
                //~ &mesh->tmesh->v[ mesh->index[t].order.vz ],
                //~ &mesh->tmesh->v[ mesh->index[t].order.vx ],
                //~ &mesh->tmesh->v[ mesh->index[t].order.vy ],
                //~ // UV coord
                //~ mesh->tmesh->u[i+3],
                //~ mesh->tmesh->u[i+2],
                //~ mesh->tmesh->u[i+0],
                //~ mesh->tmesh->u[i+1],
                //~ // Color
                //~ mesh->tmesh->c[i], 
                //~ mesh->tmesh->c[i+1], 
                //~ mesh->tmesh->c[i+2], 
                //~ mesh->tmesh->c[i+3], 
                //~ // Gpu packet
                //~ poly4,
                //~ &ot[db][*mesh->OTz],
                //~ &div4);
            //~ // Increment primitive list pointer
            //~ *nextpri  += ( (sizeof(POLY_GT4) + 3) / 4 ) * (( 1 << ( div4.ndiv )) << ( div4.ndiv ));
            //~ triCount = ((1<<(div4.ndiv))<<(div4.ndiv));
    //~ } else if (OTc < 48) {
        //~ return 0;
    //~ }
    
};
long drawQuad(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw, int t, int i) {
    long nclip = 0;
    // If mesh is quad
    POLY_GT4 * poly4; 
    //~ for (int i = 0; i < (mesh->tmesh->len * 4); i += 4) { 
        // if mesh is not part of BG, draw them, else, discard
        if ( !(mesh->isBG) || *camMode != 2 ) {
            poly4 = (POLY_GT4 *)*nextpri;
            
            // Vertex Anim 
            if (mesh->isAnim){
                // with interpolation
                if ( mesh->anim->interpolate ){
                    interpolateQuad(poly4, mesh, t, Flag);
                } else {
                    // No interpolation, use all vertices coordinates in anim data
                    nclip = RotAverageNclip4(
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.pad ],
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vz ],
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vx ],
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vy ],
                                ( long* )&poly4->x0, ( long* )&poly4->x1, ( long* )&poly4->x2, ( long* )&poly4->x3,
                                &mesh->p,
                                &mesh->OTz,
                                Flag
                            );
                }
            } else {                        
                // No animation
                // Use regular vertex coords
                nclip = RotAverageNclip4(
                            &mesh->tmesh->v[ mesh->index[t].order.pad ],  
                            &mesh->tmesh->v[ mesh->index[t].order.vz],
                            &mesh->tmesh->v[ mesh->index[t].order.vx ],
                            &mesh->tmesh->v[ mesh->index[t].order.vy ],
                            (long*)&poly4->x0, (long*)&poly4->x1, (long*)&poly4->x2, (long*)&poly4->x3,
                            &mesh->p,
                            &mesh->OTz,
                            Flag
                        );
            }
            if (nclip > 0 && mesh->OTz > 0 && (mesh->p < 4096)) {
                SetPolyGT4(poly4);
                // If tim mode  == 0 | 1, set CLUT coordinates
                if ( (mesh->tim->mode & 0x3) < 2 ) {
                    setClut(poly4,             
                            mesh->tim->crect->x,
                            mesh->tim->crect->y
                    );
                }
                if (mesh->isSprite){ 
                    SetShadeTex( poly4, 1 );
                }
                // Transparency effect
                if (mesh->isPrism){ 
                    set4Prism(poly4, mesh, draw, i);
                } else {
                    set4Tex(poly4, mesh, draw, t, i);
                }
                if ( (mesh->OTz > 0) /*&& (*mesh->OTz < OTLEN)*/ && (mesh->p < 4096) ) {
                    AddPrim( &ot[ mesh->OTz-3 ], poly4 );      
                }
                *nextpri += sizeof( POLY_GT4 );
            }
        t++;
        return t;
        }
    //~ }
};
long drawTri(MESH * mesh, long * Flag, int atime, int * camMode, char ** nextpri, u_long * ot, char * db, DRAWENV * draw, int t, int i) {
    long nclip = 0;
    // mesh is POLY_GT3 ( triangle )
    POLY_GT3 * poly;                        
    // len member == # vertices, but here it's # of triangle... So, for each tri * 3 vertices ...
    //~ for ( int i = 0; i < (mesh->tmesh->len * 3); i += 3 ) {               
        // If mesh is not part of precalculated background, draw them, else, discard
        if ( !( mesh->isBG ) || *camMode != 2) {
            poly = (POLY_GT3 *)*nextpri;
            // If Vertex Anim flag is set, use it
            if (mesh->isAnim){
                // If interpolation flag is set, use it
                if(mesh->anim->interpolate){
                    nclip = interpolateTri(poly, mesh, t, Flag);
                } else { 
                // No interpolation
                    // Use the pre-calculated vertices coordinates from the animation data
                    nclip = RotAverageNclip3(
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vx ],
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vz ],
                                &mesh->anim->data[ atime % mesh->anim->nframes * mesh->anim->nvert + mesh->index[t].order.vy ],
                                ( long* ) &poly->x0, ( long* ) &poly->x1, ( long* ) &poly->x2,
                                &mesh->p,
                                &mesh->OTz,
                                Flag
                            );
                }
            } else {
            // No animation
                // Use model's regular vertex coordinates
                nclip = RotAverageNclip3(
                            &mesh->tmesh->v[ mesh->index[t].order.vx ],  
                            &mesh->tmesh->v[ mesh->index[t].order.vz ],
                            &mesh->tmesh->v[ mesh->index[t].order.vy ],
                            ( long * ) &poly->x0, ( long * ) &poly->x1, ( long * ) &poly->x2,
                            &mesh->p,
                            &mesh->OTz,
                            Flag
                        );
            }
            // Do not draw invisible meshes 
            if ( nclip > 0 && mesh->OTz > 0 && (mesh->p < 4096) ) {
                SetPolyGT3( poly );
                // CLUT setup
                // If tim mode == 0 | 1 (4bits/8bits image), set CLUT coordinates
                if ( (mesh->tim->mode & 0x3 ) < 2){
                    setClut(poly,             
                            mesh->tim->crect->x,
                            mesh->tim->crect->y);
                }
                if ( mesh->isSprite ) { 
                    SetShadeTex( poly, 1 );
                }
                // If isPrism flag is set, use it
                // FIXME : Doesn't work with pre-rendered BGs
                if ( mesh->isPrism ) { 
                    set3Prism(poly, mesh, draw, i);
                } else {
                    set3Tex(poly, mesh, draw, t, i);
                }
                if ( (mesh->OTz > 0) /*&& (*mesh->OTz < OTLEN)*/ && (mesh->p < 4096) ) {
                    AddPrim(&ot[ mesh->OTz-2 ], poly);
                }
                *nextpri += sizeof(POLY_GT3);
            }
        t++;
        return t;
        }
    //~ }
};
void drawBG(CAMANGLE * camPtr, char ** nextpri, u_long * otdisc, char * db) {
    // Draw BG image in two SPRT since max width == 256 px 
    SPRT * sprt;                        
    DR_TPAGE * tpage;
    // Left part
    sprt = ( SPRT * ) *nextpri;
    setSprt( sprt );
    setRGB0( sprt, 128, 128, 128 );
    setXY0( sprt, 0, 0 );
    setWH( sprt, 256, SCREENYRES );
    setUV0( sprt, 0, 0 );
    setClut( sprt, 
             camPtr->BGtim->crect->x, 
             camPtr->BGtim->crect->y
    );
    addPrim( otdisc[ OT2LEN - 1 ], sprt );        
    *nextpri += sizeof( SPRT );
    // Change TPAGE
    tpage = (DR_TPAGE *) *nextpri;
    setDrawTPage(
        tpage, 0, 1,   
        getTPage( 
            camPtr->BGtim->mode & 0x3, 0,       
            camPtr->BGtim->prect->x,
            camPtr->BGtim->prect->y
        )
    );
    addPrim( otdisc[OT2LEN-1], tpage);
    *nextpri += sizeof(DR_TPAGE); 
    // Right part
    sprt = ( SPRT * ) *nextpri;
    setSprt( sprt );
    setRGB0( sprt, 128, 128, 128 );
    setXY0( sprt, SCREENXRES - ( SCREENXRES - 256 ), 0 );
    setWH( sprt, SCREENXRES - 256, SCREENYRES );
    setUV0( sprt, 0, 0 );
    setClut( sprt, 
             camPtr->BGtim->crect->x,
             camPtr->BGtim->crect->y
    );
    addPrim( otdisc[ OT2LEN-1 ], sprt );        
    *nextpri += sizeof( SPRT );
    tpage = ( DR_TPAGE * ) *nextpri;
    // Change TPAGE
    setDrawTPage(
        tpage, 0, 1,
        getTPage(
            camPtr->BGtim->mode & 0x3, 0,       
            // X offset width depends on TIM's mode
            camPtr->BGtim->prect->x + ( 64 << ( camPtr->BGtim->mode & 0x3 ) ), 
            camPtr->BGtim->prect->y
        )
    );
    addPrim( otdisc[ OT2LEN-1 ], tpage );
    *nextpri += sizeof( DR_TPAGE ); 
};
