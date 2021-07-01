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
#define addVector2(v0, v1, v2)                                         \
    (v2)->vx = (v0)->vx + (v1)->vx,                                    \
    (v2)->vy = (v0)->vy + (v1)->vy,                                    \
    (v2)->vz = (v0)->vz + (v1)->vz                                     \
    
// substract vector
#define subVector(v0, v1) \
    (v0).vx - (v1).vx,  \
    (v0).vy - (v1).vy,  \
    (v0).vz - (v1).vz   

#define normalizeVector(v)                                             \
    ((v)->vx << 12) >> 8,                                              \
    ((v)->vy << 12) >> 8,                                              \
    ((v)->vz << 12) >> 8
