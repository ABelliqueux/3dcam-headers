#include <space.h>

// From 'psyq/addons/graphics/ZIMEN/CLIP.C'
void worldToScreen( VECTOR * worldPos, VECTOR * screenPos ) {
    int distToScreen;    // corresponds to FOV
    MATRIX  curRot;      // current rotation matrix
    // Get current matrix and projection */
    distToScreen = ReadGeomScreen();    
    ReadRotMatrix(&curRot);
    // Get Rotation, Translation coordinates, apply perspective correction
    // Muliply world coordinates vector by current rotation matrix, store in screenPos
    ApplyMatrixLV(&curRot, worldPos, screenPos);
    // Get world translation vectors from rot and add to screenPos vx, vy, vz
    applyVector(screenPos, curRot.t[0], curRot.t[1], curRot.t[2], +=);
    // Correct perspective
    //~ screenPos -> vx = screenPos -> vx * distToScreen / ( screenPos -> vz + 1 ) ; // Add 1 to avoid division by 0
    //~ screenPos -> vy = screenPos -> vy * distToScreen / ( screenPos -> vz + 1 ) ; 
    //~ screenPos -> vz = distToScreen ;
};
void screenToWorld( VECTOR * screenPos, VECTOR * worldPos ) {
    int distToScreen;           // corresponds to FOV
    MATRIX  curRot, invRot;    // current rotation matrix, transpose matrix
    VECTOR Trans;               // working translation vector
    // Get current matrix and projection
    distToScreen = ReadGeomScreen();    
    ReadRotMatrix( &curRot );
    PushMatrix();               // Store matrix on the stack (slow!)
    //// worldTrans = invRot * (screenPos - Rot.t)
    // Get world translation
    Trans.vx = screenPos->vx - curRot.t[0]; // Substract world translation from screenpos
    Trans.vy = screenPos->vy - curRot.t[1];
    Trans.vz = screenPos->vz - curRot.t[2];
    // We want the inverse of the current rotation matrix.
    //
    // Inverse matrix : M^-1 = 1 / detM * T(M)
    // We know that the determinant of a rotation matrix is 1, thus:
    // M^-1 = T(M) 
    //
    // Get transpose of current rotation matrix
    // > The transpose of a matrix is a new matrix whose rows are the columns of the original.
    // https://www.quora.com/What-is-the-geometric-interpretation-of-the-transpose-of-a-matrix
    TransposeMatrix( &curRot, &invRot );
    // Multiply the transpose of current rotation matrix by the current translation vector
    ApplyMatrixLV( &invRot, &Trans, worldPos );
    // Get original rotation matrix back
    PopMatrix();
};
int cliptest3( short *v1 ) {
    if( v1[0]<0 && v1[2]<0 && v1[4]<0 ) return 0;
    if( v1[1]<0 && v1[3]<0 && v1[5]<0 ) return 0;
    if( v1[0] > SCREENXRES && v1[2] > SCREENXRES && v1[4] > SCREENXRES) return 0;
    if( v1[1] > SCREENYRES && v1[3] > SCREENYRES && v1[5] > SCREENYRES) return 0;
    return 1;
};
