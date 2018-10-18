#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <signal.h>
#include <pthread.h>

#include "list.h"
#include "libs.h"
#include "networkStuff.h"

#define debug 1

typedef struct 
{
    int socket;
    char* addr;
} ConnectionData;

void* threadAccept(void* args);

//prints a message and closes shop
void error(const char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

//s is the server socket, rage handler is setup to handle cntrl-c
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


    //Prints out the servers IP, useful for debugging
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
        printf("connectionSocket %d\n", connectionSocket);
        //pass connection data off to alt thread
        ConnectionData* data = malloc(sizeof(ConnectionData));
        data->socket = connectionSocket;
        unsigned char* addrBuf = malloc(sizeof(char)*INET6_ADDRSTRLEN+1);
        //inet_ntop(dest.sin_family, dest.sin_addr, addrBuf, INET6_ADDRSTRLEN);
        pthread_t thread;
        if (pthread_create(&thread, NULL, threadAccept, &data))
		{
			free(data);
            close(s);
            error("Error creating thread");
		}
    }

    printf("connectionSocket %d\n", connectionSocket);

    close(s);
    return 0;
}

void* threadAccept(void* args) 
{
    char buffer[MAXBUFFER+1];
    int connectionSocket = (*(ConnectionData**)args)->socket;

    if (debug > 0)
        printf("Recieved message from todo\n");
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
    free(*(ConnectionData**)args);
    close(connectionSocket);
    printf("Closed socket %d\n", connectionSocket);
}
