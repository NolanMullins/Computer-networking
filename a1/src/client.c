#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "libs.h"

#define timing 0
#define debug 1

void error(char* msg);
void sendFile(char* file, int socket);

int bufMAX = MAXBUFFER;

int main(int argc, char* argv[])
{
    if (argc < 2 || strchr(argv[1], ':')==NULL)
        error("IP:Port required");
    if (argc < 3)
        error("Need file");
    if (argc > 3)
        bufMAX = strtol(argv[3], NULL, 10);

    //Get IP and Port number
    int i=-1;
    char ip[INET6_ADDRSTRLEN];
    int port = 0;
    while(argv[1][++i]!=':')
        ip[i]=argv[1][i];
    ip[i] = '\0';
    while(argv[1][++i] != '\0')
        port = port*10+argv[1][i]-'0';

    //start timing 
    struct timespec start, finish;
	double elapsed;

	clock_gettime(CLOCK_MONOTONIC, &start);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(ip, &addr.sin_addr); 

    int s = socket(AF_INET, SOCK_STREAM, 0);    

    if (connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) 
        error("Could not connect");

    sendFile(argv[2], s);

    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    if (timing)
        printf("time taken: %lf\n", elapsed);

    close(s);
    return 0;
}

void error(char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

int readBuffer(char* str, int buffer, FILE* f) 
{
    if (f == NULL)
        return -1;
    char c;
    int len = 0;
    while (len < buffer && (c = fgetc(f)) != EOF) 
        str[len++] = c;
    str[len] = '\0';
    return len;
}

void sendFile(char* file, int socket) 
{
    FILE* f;
	if ((f = fopen(file, "r")) == NULL)
		error("No File Found");
    //Message details
    char buffer[bufMAX+1];
    memset(buffer, 0, sizeof(buffer));
    int bytesSent = 0;
    //while (fgets(buffer, MAXBUFFER, f))
    while (readBuffer(buffer, bufMAX, f) > 0)
    {
        int sent = -1, sts = 0, attempts = -1;
        while (sent != sts && ++attempts < 5)
        {
            sts = send(socket, buffer, strlen(buffer), 0);
            if (sts < 0)
                sent = 0;
            else
                sent = sts;
            /*recv(socket, &sent, sizeof(sent), 0);
            if (sent != sts)
                printf("%d == %d?\n",sent, sts);*/
        }
        if (attempts >= 5)
        {
            close(socket);
            error("Could not send data to server, aborting\n");
        }
        bytesSent += sent;
    }

    if (debug) 
        printf("Sent %d bytes\n", bytesSent);
}

