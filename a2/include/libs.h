#ifndef MULLINSN_LIBS
#define MULLINSN_LIBS

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define MAXBUFFER 1024

typedef struct
{
    int fileSize;
    int chunkSize;
    char fileName[64];
} FileInfo;

#endif