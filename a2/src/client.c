#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs.h"

#define debug 1

void error(const char* msg);
void sendFile(char* file, int socket);

int bufMAX = MAXBUFFER;

int main(int argc, char* argv[])
{
    if (argc < 2 || strchr(argv[1], ':')==NULL)
        error("IP:Port required");
    if (argc < 3 || access(argv[2], F_OK) == -1)
        error("No file / file not found");
    if (argc > 3)
        bufMAX = strtol(argv[3], NULL, 10);
    

    //Get IP and Port number
    int i=-1;
    char ip[INET6_ADDRSTRLEN], port[10];
    while(argv[1][++i]!=':')
        ip[i]=argv[1][i];
    ip[i] = '\0';
    int j = 0;
    while(argv[1][++i] != '\0')
        port[j++] = argv[1][i]; 
    port[j] = '\0';

    struct addrinfo hints, *addrInfo, *info;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    int sts;
    if ((sts = getaddrinfo(ip, port, &hints, &info)) != 0)
        error(gai_strerror(sts));

    int s;
    for(addrInfo = info; addrInfo != NULL; addrInfo = addrInfo->ai_next) {
        if ((s = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }

    if (connect(s, addrInfo->ai_addr, sizeof(struct sockaddr_in)) < 0) 
        error("Could not connect");

    sendFile(argv[2], s);

    close(s);
    return 0;
}

void error(const char* msg)
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

