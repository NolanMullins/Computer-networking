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
#define printFile 0

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

    List* threads = init();
    long connectionSocket;
    while ((connectionSocket = accept(s, (struct sockaddr*)&dest, &socketSize)) > 0)
    {
        if (debug > 0)
            printf("Recieved message from %s\n", inet_ntoa(dest.sin_addr));
        pthread_t thread;
        if (pthread_create(&thread, NULL, threadAccept, (void*)connectionSocket))
		{
            close(s);
            error("Error creating thread");
		}
    }
    printf("\nclosing shop\n");
    close(s);
    return 0;
}

void* threadAccept(void* args) 
{
    char buffer[MAXBUFFER+1];
    long connectionSocket = (long)args;

    //TODO recv file info from client
    char* givenName = "tmp";

    int ending = 0;
    char fname[128];
    sprintf(fname, "%s%s", "output/", givenName);
    //Append number to end of file if needed
    if (access(fname, F_OK) != -1)
        while (access(fname, F_OK) != -1)
            sprintf(fname, "%s%s-%d", "output/", givenName, ++ending);

    FILE* output = fopen(fname, "w");

    int len;
    while ((len = recv(connectionSocket, buffer, MAXBUFFER, 0)) > 0) 
    {
        if (len < 0)
        {
            close(s);
            error(strerror(errno));
        }
        buffer[len] = '\0';
        fprintf(output, "%s", buffer);
    }
    fclose(output);
    close(connectionSocket);
}
