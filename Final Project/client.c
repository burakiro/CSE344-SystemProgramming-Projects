//./client -r requestFile -q 33444 -s 127.0.0.1
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>
#include <dirent.h>
#include "utils.h"
#define SA struct sockaddr
char *requestFile = NULL;
char *ipValue = NULL;
int opt, portValue;
commonMessage requestArray[QUEUESIZE];
int socketConnector();
int arrived = 0;
int countlines(char *filename);
int threadNum = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
void *clientThreads(void *vargp)
{
    pthread_mutex_lock(&mutex);
    long threadId = (long)vargp;
    printf("Client-Thread-%li: Thread-%li has been created\n", threadId, threadId);
    ++arrived;
    if (arrived < threadNum) // if this thread is not lastcwait(c,m)// then wait for otherselsebroadcast(c)// i’m last, awaken the other N-1unlock(m)
    {
        pthread_cond_wait(&c, &mutex);
    }
    else
    {
        pthread_cond_broadcast(&c);
    }
    pthread_mutex_unlock(&mutex);
    int sockfd = socketConnector();
    printf("Client-Thread-%li: \" I'm requesting %s \" \n", threadId, requestArray[threadId].requestMessage);
    requestArray[threadId].threadID = threadId;
    requestArray[threadId].typeOfMessage = 0;

    write(sockfd, &requestArray[threadId], sizeof(requestArray[threadId]));
    int processedMessageResp;
    read(sockfd, &processedMessageResp, sizeof(processedMessageResp));
    printf("Client-Thread-%li: The server's response to  \"%s\" is %d\n", threadId, requestArray[threadId].requestMessage, processedMessageResp);
    printf("Client-Thread-%li: Terminating\n",threadId);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{

    while ((opt = getopt(argc, argv, "r:q:s:")) != -1) //command line argumands flag putter.
    {
        switch (opt)
        {
        case 'r':
            requestFile = optarg;
            break;
        case 'q':
            portValue = atoi(optarg);
            break;
        case 's':
            ipValue = optarg;
            break;
        case '?':
            if (optopt == 'r')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'q')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 's')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(EXIT_FAILURE);
            }
        }
    }

    //printf("r (requestfile): %s q (port): %d s IP: %s \n",requestFile,portValue,ipValue);



    FILE *fp;
    char *line = NULL;
    line = malloc(300 * sizeof(line));
    size_t len = 0;
    ssize_t read;
    int lineSize = 0;

    fp = fopen(requestFile, "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (line[0] == '\n')
        {
            continue;
        }
        lineSize++;
    }
    struct request *clientRequest;
    clientRequest = malloc(lineSize * sizeof(clientRequest));

    printf("Client: I have loaded %d requests and I’m creating %d threads.\n", lineSize, lineSize);
    rewind(fp);
    pthread_t idClientThreads[lineSize];
    while ((read = getline(&line, &len, fp)) != -1)
    {
        //printf("Retrieved line of length %zu:\n", read);
        //strcpy(clientRequest->request, line);
        //printf("%s", clientRequest->request);
        //clientRequest->threadID = threadNum;
        //printf("%s\n", clientRequest->request);
        //printf("%d", clientRequest->threadID);
        //struct request *clientRequest;
        //clientRequest = malloc(sizeof(clientRequest));
        //(clientRequest + threadNum)->threadID = threadNum;
        if (line[0] == '\n')
        {
            continue;
        }

        line[strlen(line) - 1] = '\0';
        strcpy(requestArray[threadNum].requestMessage, line);

        requestArray[threadNum].threadID = threadNum;
        //printf("line: %s\n",line);
        //pthread_create(&idClientThreads[threadNum], NULL, clientThreads, NULL);
        //printf("ThreadNum: %d\n",threadNum);
        threadNum++;
    }

    for (int i = 0; i < lineSize; i++)
    {
        pthread_create(&idClientThreads[i], NULL, clientThreads, (void *)(intptr_t)i);
    }

    for (int i = 0; i < lineSize; i++)
    {
        pthread_join(idClientThreads[i], NULL); //wait for consumers end.
    }

    printf("Client: All threads have terminated, goodbye.\n");
    //close(sockfd);
    free(line);
    free(clientRequest);
    pthread_exit(NULL);
    // function for chat
    //func(sockfd);

    // close the socket
    //
}

int socketConnector()
{
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        //printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ipValue);
    servaddr.sin_port = htons(portValue);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connection with the server failed...\n");
        exit(0);
    }
    else
        //printf("connected to the server..\n");

        return sockfd;
}
