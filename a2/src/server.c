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
#define outputFile 0

void* threadAccept(void* args);
void *UIThread(void *args);

//prints a message and closes shop
void error(const char* msg)
{
    printf("%s\n", msg);
    exit(1);
}

//s is the server socket, rage handler is setup to handle cntrl-c
int s;
int flag = 0;
List *list;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
// int max_sd;

void destroy(void *data)
{

}

void rageHandler(int signum)
{
  list = listClear(list, destroy);
    close(s);
}

typedef struct thread
{
  pthread_t tid;
  int percent;

  char *filename;
}Thread;

typedef struct threadInfo{
  long connectionSocket;
  Thread *thread;
}ThreadInfo;

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

    fd_set set;

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

    list = init();

    int rc;

    pthread_t ui_thread;
    pthread_create(&ui_thread, NULL, UIThread, (void*) NULL);

    long connectionSocket;
    struct timeval timeout;

    while(1)
    {

      FD_ZERO(&set);
      FD_SET(s, &set);

      timeout.tv_sec = 0;
      timeout.tv_usec = 2 * 1000000;

      // printf("Waiting on select()...\n");
      rc = select(s + 1, &set, NULL, NULL, &timeout);

      if (rc < 0)
      {
         perror("select() failed");
         break;
      }

      if (rc == 0)
      {
         // printf("flag checked\n");
         if(flag == 1)
         {
           printf("%s\n", "hard terminate");
           break;
         }
         else if(flag == 2)
         {
           printf("%s\n", "soft terminate");
           // wait for threads to finish

           // while()
           // {
           //
           // }
           pthread_mutex_lock(&mutex);

           //view list

           Node *start = list->list;

           while(start)
           {
             pthread_join(((Thread*)(start->data))->tid, NULL);
             start = start->next;
           }

           pthread_mutex_unlock(&mutex);
         }
         continue;
      }

      printf("%i\n", rc);

      if ((connectionSocket = accept(s, (struct sockaddr*)&dest, &socketSize)) > 0)
      {
          if (debug > 0)
              printf("Recieved message from %s\n", inet_ntoa(dest.sin_addr));


          Thread *thread = malloc(sizeof(Thread));
          thread->percent = 0;
          thread->filename = NULL;

          ThreadInfo *threadInfo = malloc(sizeof(ThreadInfo));
          threadInfo->connectionSocket = connectionSocket;
          threadInfo->thread = thread;

          if (pthread_create(&thread->tid, NULL, threadAccept, (void*)threadInfo))
      		{
              close(s);
              error("Error creating thread");
      		}

          pthread_mutex_lock(&mutex);
          listAdd(list, (void*) thread);
          pthread_mutex_unlock(&mutex);
      }

    }

    list = listClear(list, destroy);

    printf("\nclosing shop\n");
    close(s);
    return 0;
}

void *UIThread(void *args)
{

  char buffer[256];

  while(1)
  {
    // fgets(buffer, 256, stdin);
    fscanf(stdin, "%s", buffer);

    if(strcmp(buffer, "show") == 0)
    {
      //display active transfers

      pthread_mutex_lock(&mutex);

      //view list

      Node *start = list->list;

      while(start)
      {
        Thread *t = (Thread*)(start->data);
        if(t->filename)
          printf("%s:", t->filename);
        printf("%i%%\n", t->percent);
        start = start->next;
      }

      pthread_mutex_unlock(&mutex);
    }
    else if(strcmp(buffer, "hexit") == 0)
    {
      //terminate
      flag = 1;
      return NULL;
    }
    else if(strcmp(buffer, "sexit") == 0)
    {
      flag = 2;
      return NULL;
    }

  }

}

void* threadAccept(void* args)
{
  ThreadInfo *threadInfo = (ThreadInfo*)args;

    char buffer[MAXBUFFER+1];
    long connectionSocket = threadInfo->connectionSocket;

    FileInfo fInfo;
    int len = recv(connectionSocket, &fInfo, sizeof(FileInfo), 0);
    int ending = 0;
    char fname[128];
    sprintf(fname, "%s%s", "output/", fInfo.fileName);
    //Append number to end of file if needed
    if (access(fname, F_OK) != -1)
        while (access(fname, F_OK) != -1)
            sprintf(fname, "%s%s-%d", "output/", fInfo.fileName, ++ending);

    pthread_mutex_lock(&mutex);
    threadInfo->thread->filename = malloc(sizeof(char) * (1+strlen(fInfo.fileName)));
    strcpy(threadInfo->thread->filename, fInfo.fileName);
    threadInfo->thread->filename[strlen(fInfo.fileName)] = '\0';
    pthread_mutex_unlock(&mutex);

    FILE* output;
    if (outputFile)
    {
        output = fopen(fname, "w");
        if (output == NULL)
        {
            close(connectionSocket);
            error("Could not create file");
        }
    }

    int currentChunk = 0;

    while ((len = recv(connectionSocket, buffer, MAXBUFFER, 0)) > 0)
    {
        if (len < 0)
        {
            close(s);
            error(strerror(errno));
        }
        currentChunk++;

        pthread_mutex_lock(&mutex);
        threadInfo->thread->percent = 100 * ( ( ((double)currentChunk) * ((double)fInfo.chunkSize) )/((double)fInfo.fileSize) );
        // threadInfo->thread->percent = fInfo.fileSize;
        pthread_mutex_unlock(&mutex);

        buffer[len] = '\0';
        if (outputFile)
            fprintf(output, "%s", buffer);
    }
    if (outputFile)
        fclose(output);

    printf("done\n");
    close(connectionSocket);
}
