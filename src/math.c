#include "../include/math.h"

// Stolen from grumpycoder
// this is from here : https://github.com/grumpycoders/Balau/blob/master/tests/test-Handles.cc#L20-L102
// precalc costable
static int m_cosTable[512];                     
static const unsigned int DC_2PI = 2048;        
static const unsigned int DC_PI  = 1024;
static const unsigned int DC_PI2 = 512;
// f(n) = cos(n * 2pi / 2048) <- 2048 is == DC_2PI value
// f(n) = 2 * f(1) * f(n - 1) - f(n - 2)
void generateTable(void){
    m_cosTable[0] = 16777216;               // 2^24 * cos(0 * 2pi / 2048) => 2^24 * 1 = 2^24 : here, 2^24 defines the precision we want after the decimal point
    static const long long C = 16777137;    // 2^24 * cos(1 * 2pi / 2048) = C = f(1);
    m_cosTable[1] = C;
    for (int i = 2; i < 512; i++){
        m_cosTable[i] = ((C * m_cosTable[i - 1]) >> 23) - m_cosTable[i - 2];
        m_cosTable[511] = 0;
    }
};
int ncos(unsigned int t) {
    t %= DC_2PI;
    int r;
    if (t < DC_PI2) {
        r = m_cosTable[t];
    } else if (t < DC_PI) {
        r = -m_cosTable[DC_PI - 1 - t];
    } else if (t < (DC_PI + DC_PI2)) {
        r = -m_cosTable[t - DC_PI];
    } else {
        r = m_cosTable[DC_2PI - 1 - t];
    };
    return r >> 12;
};
// sin(x) = cos(x - pi / 2)
int nsin(unsigned int t) {
    t %= DC_2PI;
    if (t < DC_PI2){
        return ncos(t + DC_2PI - DC_PI2);
    };
    return ncos(t - DC_PI2);
};
// https://github.com/Arsunt/TR2Main/blob/411cacb35914c616cb7960c0e677e00c71c7ee88/3dsystem/phd_math.cpp#L432
long long patan(long x, long y){
    long long result;
    int swapBuf;
    int flags = 0;
    // if either x or y are 0, return 0
    if( x == 0 && y == 0){
        return 0;
    }
    if( x < 0 ) {
        flags |= 4; 
        x = -x;
    }
    if ( y < 0 ) {
        flags |= 2;
        y = -y;
    }
    if ( y > x ) {
        flags |= 1;
        SWAP(x, y ,swapBuf);
    }
    result = AtanBaseTable[flags] + AtanAngleTable[0x800 * y / x];
    if ( result < 0 ){
        result = -result;
    return result;
    }
};
u_int psqrt(u_int n){
    u_int result = 0;
    u_int base = 0x40000000;
    u_int basedResult;
    for( ; base != 0; base >>= 2 ) {
        for( ; base != 0; base >>= 2 ) {
            basedResult = base + result;
            result >>= 1;
            if( basedResult > n ) {
                break;
            }
            n -= basedResult;
            result |= base;
        }
    }
    return result;
};
// From : https://github.com/grumpycoders/pcsx-redux/blob/7438e9995833db5bc1e14da735bbf9dc78300f0b/src/mips/shell/math.h
int32_t dMul(int32_t a, int32_t b) {
    long long r = a;
    r *= b;
    return r >> 24;
};
// standard lerp function
//  s = source, an arbitrary number up to 2^24
//  d = destination, an arbitrary number up to 2^24
//  p = position, a number between 0 and 256, inclusive
//  p = 0 means output = s
//  p = 256 means output = d
uint32_t lerpU(uint32_t start, uint32_t dest, unsigned pos) {
    return (start * (256 - pos) + dest * pos) >> 8; 
};
int32_t lerpS(int32_t start, int32_t dest, unsigned pos) {
    return (start * (256 - pos) + dest * pos) >> 8;
};
// start, dest and pos have to be << x, then the result has to be >> x where x defines precision: 
// precision = 2^24 - 2^x
// << x : 0 < pos < precision
// https://discord.com/channels/642647820683444236/646765703143227394/811318550978494505
// my angles are between 0 and 2048 (full circle), so 2^11 for the range of angles; with numbers on a 8.24 representation, a 1.0 angle (or 2pi) means it's 2^24, so to "convert" my angles from 8.24 to my internal discrete cos, I only have to shift by 13
int32_t lerpD(int32_t start, int32_t dest, int32_t pos) {
     return dMul(start, 16777216 - pos) + dMul(dest, pos);
};
long long lerpL(long long start, long long dest, long long pos) {
    return dMul( (start << 12), 16777216 - (pos << 12) ) + dMul((dest << 12), (pos << 12) ) >> 12;
};
int lerp(int start, int end, int factor){
    // lerp interpolated cam movement
    // InBetween = Value 1 + ( ( Value2 - Value1 ) * lerpValue ) ;
    // lerpValue should be a int between 17 and 256.
    return ( ( start ) + ( ( end - start ) * factor ) ) >> 12;
};
long long easeIn(long long i){
    return ((i << 7) * (i << 7) * (i << 7) / 32 ) >> 19;
};
int easeOut(int i){
    return (4096 >> 7) - ((4096 - (i << 7)) * (4096 - (i << 7))) >> 12;
};
//~ int easeInOut(int i, int div){
    //~ return lerp(easeIn(i, div), easeOut(i) , i);
//~ };
SVECTOR SVlerp(SVECTOR start, SVECTOR end, int factor){
    SVECTOR output = {0,0,0,0};
    output.vx = lerp(start.vx, end.vx, factor);
    output.vy = lerp(start.vy, end.vy, factor);
    output.vz = lerp(start.vz, end.vz, factor);
    return output;
};
VECTOR getVectorTo( VECTOR actor, VECTOR target ) {
    // Returns a normalized vector that points from actor to target
    VECTOR direction = { subVector(target, actor) };
    VECTOR Ndirection = {0,0,0,0};
    u_int distSq = (direction.vx * direction.vx) + (direction.vz * direction.vz);
    direction.pad = psqrt(distSq);
    VectorNormal(&direction, &Ndirection);
    return Ndirection ;
};
