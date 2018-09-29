#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs.h"
#include <arpa/inet.h>

void error(char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2 || strchr(argv[1], ':')==NULL)
        error("IP:Port required");

    //Get IP and Port number
    int i=-1;
    char ip[INET6_ADDRSTRLEN];
    int port = 0;
    while(argv[1][++i]!=':')
        ip[i]=argv[1][i];
    ip[i] = '\0';
    while(argv[1][++i] != '\0')
        port = port*10+argv[1][i]-'0';

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(ip, &addr.sin_addr); 

    int s = socket(AF_INET, SOCK_STREAM, 0);    

    //TODO look at errors here
    if (connect(s, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) < 0) 
        error("could not connect");

    //Message details
    char* msg = "Hello internet";

    //TODO check errors
    int sts = send(s, msg, strlen(msg), 0);
    if (sts < 0)
        error("could not send message");
    printf("Sent %d bytes\n", sts);

    close(s);
    return 0;
}

