#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libs.h"

void error(char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        error("Port required");

    //Port number
    int i=-1, port = 0;
    while(argv[1][++i] != '\0')
        port = port*10+argv[1][i]-'0';
    
    int len;
    char buffer[MAXBUFFER + 1];

    struct sockaddr_in dest, server;
    int s;
    socklen_t socketSize = sizeof(struct sockaddr_in);
    memset(&server, 0, sizeof(server));

    //server info
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(s, (struct sockaddr*)&server, sizeof(struct sockaddr)) != 0)
    {
        printf("Unable to open TCP socket on port: %d\n", port);
        printf("%s\n", strerror(errno));
        close(s);
        return 0;
    }

    listen(s, 10);
    int connectionSocket;
    while ((connectionSocket = accept(s, (struct sockaddr*)&dest, &socketSize)) > 0)
    {
        printf("Recieved message from %s\n", inet_ntoa(dest.sin_addr));
    }

    close(s);
    return 0;
}
