#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ifaddrs.h>
#include <signal.h>
#include <pthread.h>
#include <sys/sem.h>
#include <semaphore.h>

#include "list.h"
#include "libs.h"
#include "networkStuff.h"

#define debug 0
#define outputFile 1
#define serverSideSleep 0

void *threadAccept(void *args);
void *UIThread(void *args);

//prints a message and closes shop
void error(const char *msg)
{
	printf("%s\n", msg);
	exit(1);
}

//s is the server socket, rage handler is setup to handle cntrl-c
int s;
int flag = 0;
List *list;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int flag_id;
sem_t flag_wait;
// int max_sd;

typedef struct thread
{
	pthread_t tid;
	int percent;

	char *filename;
	char *ip;
} Thread;

typedef struct threadInfo
{
	long connectionSocket;
	Thread *thread;
} ThreadInfo;

void destroy(void *data)
{
	Thread *t = (Thread *)data;

	if (t->filename)
		free(t->filename);
	if (t->ip)
		free(t->ip);
	free(t);
}

int compare(void *a, void *b)
{
	Thread *t1 = (Thread *)a;
	Thread *t2 = (Thread *)b;

	if (pthread_equal(t1->tid, t2->tid))
	{
		return 0;
	}
	return 1;
}

void rageHandler(int signum)
{
	list = listClear(list, destroy);
	close(s);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		error("Port required");

	if (semctl(flag_id, 1, SETVAL, flag_wait) == 1)
		perror("Semaphore failed to initialize");

	//Port number
	int i = -1, port = 0;
	while (argv[1][++i] != '\0')
		port = port * 10 + argv[1][i] - '0';

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

	if (bind(s, (struct sockaddr *)&server, sizeof(struct sockaddr)) != 0)
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
		if (getifaddrs(&ifaddr) == -1)
		{
			perror("getifaddrs");
			exit(EXIT_FAILURE);
		}

	//Prints out the servers IP, useful for debugging
	int n;
	if (debug > 1)
		for (ifa = ifaddr, n = 0; ifa != NULL; ifa = ifa->ifa_next, n++)
		{
			printf("loop\n");
			if (ifa->ifa_addr == NULL)
				continue;

			int family = ifa->ifa_addr->sa_family;
			if (family == AF_INET || family == AF_INET6)
			{
				int sts = getnameinfo(ifa->ifa_addr, (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
				if (sts != 0)
					error(gai_strerror(sts));
				printf("%s:\t%s\n", ifa->ifa_name, host);
			}
		}

	listen(s, 10);

	list = init();

	int rc;

	pthread_t ui_thread;
	pthread_create(&ui_thread, NULL, UIThread, (void *)NULL);

	long connectionSocket;
	struct timeval timeout;

	while (1)
	{

		FD_ZERO(&set);
		FD_SET(s, &set);

		timeout.tv_sec = 0;
		timeout.tv_usec = 0.5 * 1000000;

		rc = select(s + 1, &set, NULL, NULL, &timeout);

		if (rc < 0)
		{
			perror("\nselect() failed");
			break;
		}

		if (rc == 0)
		{
			if (flag == 1)
			{
				printf("%s\n", "hard terminate");
				break;
			}
			else if (flag == 2)
			{
				int list_size = listSize(list);
				if (list_size)
				{
					char buffer[256];
					int terminate = 0;
					while (1)
					{
						printf("%s\n", "Connections are still in progress, 't' - terminate, 'r' - run");
						fscanf(stdin, "%s", buffer);

						if (strcmp(buffer, "t") == 0)
						{
							printf("Connections Terminated");
							terminate = 1;
							break;
						}
						else if (strcmp(buffer, "r") == 0)
						{
							printf("%s\n", "Finishing Connections");
							sem_post(&flag_wait);
							pthread_mutex_lock(&mutex);
							list_size = listSize(list);
							pthread_t threads[list_size];

							Node *start = list->list;

							for (int i = 0; start; i++)
							{
								Thread *t = (Thread *)start->data;
								threads[i] = t->tid;
								start = start->next;
							}

							pthread_mutex_unlock(&mutex);

							for (int i = 0; i < list_size; i++)
							{
								pthread_join(threads[i], NULL);
							}
							break;
						}
					}
				}
				pthread_cancel(ui_thread);
				break;
			}
			continue;
		}

		if ((connectionSocket = accept(s, (struct sockaddr *)&dest, &socketSize)) > 0)
		{
			char *address = inet_ntoa(dest.sin_addr);
			if (debug > 0)
				printf("Recieved message from %s\n", address);

			Thread *thread = malloc(sizeof(Thread));
			thread->percent = 0;
			thread->ip = NULL;
			thread->filename = NULL;

			thread->ip = malloc(sizeof(char) * (1 + strlen(address)));
			strcpy(thread->ip, address);

			ThreadInfo *threadInfo = malloc(sizeof(ThreadInfo));
			threadInfo->connectionSocket = connectionSocket;
			threadInfo->thread = thread;

			if (pthread_create(&thread->tid, NULL, threadAccept, (void *)threadInfo))
			{
				close(s);
				error("Error creating thread");
			}

			pthread_mutex_lock(&mutex);
			listAdd(list, (void *)thread);
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
	printf("%s\n", "Enter 'show' to see transfer progress or 'exit' to enter exit protocol");
	char buffer[256];

	while (1)
	{
		// fgets(buffer, 256, stdin);
		fscanf(stdin, "%s", buffer);

		if (strcmp(buffer, "show") == 0)
		{
			//display active transfers

			pthread_mutex_lock(&mutex);

			//view list

			Node *start = list->list;

			while (start)
			{
				Thread *t = (Thread *)(start->data);
				if (t->ip)
					printf("%s : ", t->ip);
				if (t->filename)
					printf("%s : ", t->filename);
				printf("%i%%\n", t->percent);
				start = start->next;
			}

			pthread_mutex_unlock(&mutex);
		}
		else if (strcmp(buffer, "exit") == 0)
		{
			//terminate
			printf("%s\n", "enter 's' - soft or 'h' - hard");

			fscanf(stdin, "%s", buffer);

			if (strcmp(buffer, "h") == 0)
			{
				flag = 1;
				return NULL;
			}
			else if (strcmp(buffer, "s") == 0)
			{
				flag = 2;
				sem_wait(&flag_wait);
			}
		}
	}
}

void *threadAccept(void *args)
{
	ThreadInfo *threadInfo = (ThreadInfo *)args;

	char buffer[MAXBUFFER + 1];
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
	threadInfo->thread->filename = malloc(sizeof(char) * (1 + strlen(fInfo.fileName)));
	strcpy(threadInfo->thread->filename, fInfo.fileName);
	pthread_mutex_unlock(&mutex);

	FILE *output;
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
	int percent = 0;
	int mark = 10;

	while ((len = recv(connectionSocket, buffer, fInfo.chunkSize, 0)) > 0)
	{
		if (percent >= mark)
		{
			if (serverSideSleep)
				sleep(1);
			mark += 10;
		}
		if (len < 0)
		{
			close(s);
			error(strerror(errno));
		}
		currentChunk++;

		pthread_mutex_lock(&mutex);
		threadInfo->thread->percent = percent = 100 * ((((double)currentChunk) * ((double)fInfo.chunkSize)) / ((unsigned long)fInfo.fileSize));
		// threadInfo->thread->percent = fInfo.fileSize;
		pthread_mutex_unlock(&mutex);

		buffer[len] = '\0';
		if (outputFile)
			fprintf(output, "%s", buffer);
	}
	if (outputFile)
		fclose(output);

	close(connectionSocket);
	pthread_mutex_lock(&mutex);
	destroy(listRemove(list, indexOf(list, compare, threadInfo->thread)));
	pthread_mutex_unlock(&mutex);
	return NULL;
}
