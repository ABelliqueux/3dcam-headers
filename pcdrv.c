int waitForSIODone( int command ){
    
    // This should wait for a signal from the SIO to send when it's done  
    // command can be 0 open, 1 close, 2 seek, 3 read, 4 write
    // Returns val < 0 if wrong

};

#define OPEN   0
#define CLOSE  1 
#define SEEK   2
#define READ   3
#define WRITE  4
#define CREATE 5

int open(const char * filename, int attributes) {
    
  // Send filename and attributes (ro,wo,rw..)
  // expects an int referring to a file descriptor on the PC 

  int fd = 0; // File Descriptor https://en.wikipedia.org/wiki/File_descriptor

  printf("open:%s:%i:%08x", filename, attributes, &fd);
  
  waitForSIODone(0)
  
  return fd; // If all is well, returns a positive int . If -1, wrong

};

int create(const char * filename, int attributes) {
    
  // Send filename and attributes (ro,wo,rw..)
  // expects an int referring to a file descriptor on the PC 

  int fd = 0; // File Descriptor https://en.wikipedia.org/wiki/File_descriptor

  printf("create:%s:%i:%08x", filename, attributes, &fd);
  
  waitForSIODone(5); // Should return int fd, -1 else
  
  return fd; // If all is well, returns a positive int . If -1, wrong

};

int close( int fd ) {
  
  // Send the close command and fd as int

  printf("close:%s:%08x", filename, fd);
  
  return waitForSIODone(1); // Should return 1 if ok, 0 if wrong, or -1 if wrong ?

};

int seek( int fd, int offset, int accessMode){
  
  // Move file pointer in fd at offset
  // Access mode can be 0 abs, 1 rel to start, 2 rel to end 

  printf("seek:%i:%08x:%i", fd, offset, accessMode);
  
  return waitForSIODone(2);
    
};

int read( int fd, int len, char * buffer ){
  
  // Read len bytes of fd and put them in buff

  int count = 0;

  printf("read:%i:%08x:%i", fd, len, buffer, count);
  
  waitForSIODone(3);
  
  return count;
    
};

