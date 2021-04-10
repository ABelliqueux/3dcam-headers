// MACROS 

// swap(x, y, buffer)

#define SWAP(a,b,c)			{(c)=(a); (a)=(b); (b)=(c);} 

// dotproduct of two vectors

#define dotProduct(v0, v1)  \
	(v0).vx * (v1).vx +	    \
	(v0).vy * (v1).vy +	 \
	(v0).vz * (v1).vz

// min value

#define min(a,b)    \
        (a)-(b)>0?(b):(a)
// max

#define max(a,b)    \
        (a)-(b)>0?(a):(b)

// substract vector

#define subVector(v0, v1) \
	(v0).vx - (v1).vx,	\
	(v0).vy - (v1).vy,	\
	(v0).vz - (v1).vz	


#define normalizeVector(v)                                             \
    ((v)->vx << 12) >> 8,                                              \
    ((v)->vy << 12) >> 8,                                              \
    ((v)->vz << 12) >> 8
