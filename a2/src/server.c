#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <signal.h>
#include <pthread.h>

#include "libs.h"
#include "networkStuff.h"

#define debug 1

typedef struct 
{
    int socket;
    char* addr;
} ConnectionData;

void error(const char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

int s;

void rageHandler(int signum) 
{
    close(s);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
        error("Port required");

    //Port number
    int i=-1, port = 0;
    while(argv[1][++i] != '\0')
        port = port*10+argv[1][i]-'0';
    
    struct sockaddr_in dest, server;
    socklen_t socketSize = sizeof(struct sockaddr_in);
    memset(&server, 0, sizeof(server));

    //server info
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    s = socket(AF_INET, SOCK_STREAM, 0);

    //Handle outside termination of program
    signal(SIGINT, rageHandler);

    if (bind(s, (struct sockaddr*)&server, sizeof(struct sockaddr)) != 0)
    {
        printf("Unable to open TCP socket on port: %d\n", port);
        printf("%s\n", strerror(errno));
        close(s);
        return 0;
    }

    //Get ip addr of server
    struct ifaddrs *ifaddr, *ifa;
    char host[NI_MAXHOST];

    if (debug > 0)
        if (getifaddrs(&ifaddr) == -1) {
            perror("getifaddrs");
            exit(EXIT_FAILURE);
        }

    int n;
    if (debug > 1)
        for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
            printf("loop\n");
            if (ifa->ifa_addr == NULL)
                continue;

            int family = ifa->ifa_addr->sa_family;
            if (family == AF_INET || family == AF_INET6) {
                int sts = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST,NULL, 0, NI_NUMERICHOST);
                if (sts != 0) 
                    error(gai_strerror(sts));
                printf("%s:\t%s\n",ifa->ifa_name, host);
            }
        }

    listen(s, 10);
    int connectionSocket;
    while ((connectionSocket = accept(s, (struct sockaddr*)&dest, &socketSize)) > 0)
    {
        ConnectionData* data = malloc(sizeof(connectionData));
        data->socket = connectionSocket;
        unsigned char* addrBuf = malloc(sizeof(char)*INET6_ADDRSTRLEN+1);
        inet_ntop(dest.sin_family, dest.sin_addr, addrBuf, INET6_ADDRSTRLEN);
        pthread_t thread;
        if (pthread_create(&thread, NULL, threadAccept, &data))
		{
			free(data);
            close(s);
            error("Error creating thread");
		}
    }

    close(s);
    return 0;
}

void* threadAccept(void* args) 
{
    char buffer[MAXBUFFER+1];
    int connectionSocket = 0;

    if (debug > 0)
        printf("Recieved message from %s\n", );
    int len;
    while ((len = recv(connectionSocket, buffer, MAXBUFFER, 0)) > 0) 
    {
        if (len < 0)
        {
            close(s);
            error(strerror(errno));
        }
        buffer[len] = '\0';
        //send(connectionSocket, &len, sizeof(len), 0);
        printf("%s",buffer);
    }
    printf("\n");
    free(args);
    close(connectionSocket);
}
