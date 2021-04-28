#pragma once

#include <sys/types.h>
#include <stdio.h>

#define OPEN   0
#define CLOSE  1 
#define SEEK   2
#define READ   3
#define WRITE  4
#define CREATE 5
#define LOAD   6

int waitForSIODone( int * flag );

void PCload( u_long * loadAddress, u_short * flagAddress, const char * filename );

int PCopen(const char * filename, int attributes );

int PCcreate(const char * filename, int attributes );

int PCclose( int fd );

int PCseek( int fd, int offset, int accessMode );

int PCread( int fd, int len, char * buffer );

