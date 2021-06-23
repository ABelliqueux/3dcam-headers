#include "../include/pcdrv.h"

int waitForSIODone( int * flag ){
    // This should wait for a signal from the SIO to tell when it's done 
    // Returns val < 0 if wrong
    uint timeOut = 1000;
    int result  = 0;
    for ( uint t = 0; t < timeOut; t ++){
        if ( * flag == 1 ){
            result = * flag;
            break;
        }
    }
    return result;
};
void PCload( u_long * loadAddress, u_short * flagAddress, const char * filename ) {
  // Send filename , load address, and flag address
  // flag address points to an int16 that when set to 1 enable lvl/pointers/screen refresh
  // Returns 1 if all good, 0 else
  //~ int flag = 0;
  printf("load:%08x:%08x:%s", loadAddress, flagAddress, filename);
  //~ return ; // If all is well, returns a positive int . If -1, wrong
};
//~ int PCopen(const char * filename, int attributes) {
  //~ // Send filename and attributes (ro,wo,rw..)
  //~ // expects an int referring to a file descriptor on the PC 
  //~ int fd = 0; // File Descriptor https://en.wikipedia.org/wiki/File_descriptor
  //~ printf("open:%s:%i:%08x", filename, attributes, &fd);
  //~ waitForSIODone(0);
  //~ return fd; // If all is well, returns a positive int . If -1, wrong
//~ };
//~ int PCcreate(const char * filename, int attributes) {
  //~ // Send filename and attributes (ro,wo,rw..)
  //~ // expects an int referring to a file descriptor on the PC 
  //~ int fd = 0; // File Descriptor https://en.wikipedia.org/wiki/File_descriptor
  //~ printf("create:%s:%i:%08x", filename, attributes, &fd);
  //~ waitForSIODone(5); // Should return int fd, -1 else
  //~ return fd; // If all is well, returns a positive int . If -1, wrong
//~ };
//~ int PCclose( int fd ) {
  //~ // Send the close command and fd as int
  //~ printf("close:%d", fd);
  //~ return waitForSIODone(1); // Should return 1 if ok, 0 if wrong, or -1 if wrong ?
//~ };
//~ int PCseek( int fd, int offset, int accessMode){
  //~ // Move file pointer in fd at offset
  //~ // Access mode can be 0 abs, 1 rel to start, 2 rel to end 
  //~ printf("seek:%i:%08x:%i", fd, offset, accessMode);
  //~ return waitForSIODone(2);
//~ };
//~ int PCread( int fd, int len, char * buffer ){
  //~ // Read len bytes of fd and put them in buff
  //~ int count = 0;
  //~ printf("read:%i:%08x:%i", fd, len, buffer, count);
  //~ waitForSIODone(3);
  //~ return count;
//~ };
