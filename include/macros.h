// MACROS 
// swap(x, y, buffer)
#define SWAP(a,b,c)         {(c)=(a); (a)=(b); (b)=(c);} 

// dotproduct of two vectors
#define dotProduct(v0, v1)  \
    (v0).vx * (v1).vx +     \
    (v0).vy * (v1).vy +  \
    (v0).vz * (v1).vz

// return min value
#define min(a,b)    \
        (a)-(b)>0?(b):(a)
// return max value
#define max(a,b)    \
        (a)-(b)>0?(a):(b)
// add 2 vector and store in a third vector
#define addVector2(v0, v1, v2)      \
    (v2)->vx = (v0)->vx + (v1)->vx, \
    (v2)->vy = (v0)->vy + (v1)->vy, \
    (v2)->vz = (v0)->vz + (v1)->vz, \
    (v2)->pad = (v0)->pad + (v1)->pad \
    
// substract vector
#define subVector(v0, v1) \
    (v0).vx - (v1).vx,  \
    (v0).vy - (v1).vy,  \
    (v0).vz - (v1).vz   

#define normalizeVector(v)                                             \
    ((v)->vx << 12) >> 8,                                              \
    ((v)->vy << 12) >> 8,                                              \
    ((v)->vz << 12) >> 8

// GTE Macros

#define gte_RotAverageNclip4(r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12)   \
    {   gte_ldv3(r1,r2,r3);                                            \
        gte_rtpt();                                                    \
        gte_stflg(r11);                                                \
        gte_nclip();                                                   \
        gte_ldv0(r4);                                                  \
        gte_stopz(r12);                                                \
        gte_stsxy3(r5,r6,r7);                                          \
        gte_rtps();                                                    \
        gte_stsxy(r8);                                                 \
        gte_stdp(r9);                                                  \
        gte_avsz4();                                                   \
        gte_stotz(r10);  }

// convert Little endian to Big endian
#define SWAP_ENDIAN32(x) (((x)>>24) | (((x)>>8) & 0xFF00) | (((x)<<8) & 0x00FF0000) | ((x)<<24))
