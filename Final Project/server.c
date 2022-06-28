// ./server -p 33000 -t 11
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netdb.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>
#include "utils.h"

sem_t *lock0;
int sockfd, connfd, option = 1;
socklen_t len;
struct sockaddr_in servaddr, cli;
int opt, portValue, numberOfThreads;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
int handledRequest = 0;
void enQueue(commonMessage);
commonMessage deQueue();
void display();
void socketConnectorForReceive();
int socketConnectorForSend(int);
commonMessage items[QUEUESIZE];
commonMessage servantInfoArray[MAX_SERVANT_LIMIT];
int front = -1, rear = -1;
int sizeOfQueue = 0;
int servantsCounter = 0;
int indexOfServant = 0;
//Signal Handler.
static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
	(void)_;

	keep_running = 0;
}
void *serverThreads(void *vargp)
{
	//printf("I created %d",(int)vargp);
	const char s[2] = " ";
	char *token;
	char tempRequestMessage[100];
	while (1)
	{
		/*
		if (sem_wait(lock0) < 0)
		{
			perror("Semaphore value decrement error! ");
			exit(EXIT_SUCCESS);
		}*/

		//need mutex lock right there

		pthread_mutex_lock(&mutex);
		if (sizeOfQueue < 1)
		{
			pthread_cond_wait(&c, &mutex);
		}
		commonMessage receivedCommonMessage = deQueue();
		pthread_mutex_unlock(&mutex);
		if (receivedCommonMessage.typeOfMessage == 0)
		{
			pthread_mutex_lock(&mutex);
			//printf("Client Request:\n");

			/* get the first token */
			int wordCounter = 0;

			strcpy(tempRequestMessage, receivedCommonMessage.requestMessage);
			token = strtok(tempRequestMessage, s);
			char cityName[50];
			/* walk through other tokens */
			while (token != NULL)
			{
				//printf(" %s\n", token);
				token = strtok(NULL, s);
				wordCounter++;
				if (wordCounter == 4 && token != NULL)
				{
					strcpy(cityName, token);
					//printf("City name: %s\n",token);
				}
			}
			pthread_mutex_unlock(&mutex);

			if (wordCounter == 4)
			{
				int finalResult = 0;

				printf("Request arrived: \"%s\"\n", receivedCommonMessage.requestMessage);
				fflush(stdout);
				printf("Contacting ALL servants\n");
				fflush(stdout);

				for (int i = 0; i < indexOfServant; i++)
				{
					pthread_mutex_lock(&mutex);
					int socketForSend = socketConnectorForSend(servantInfoArray[i].portOfServant);
					write(socketForSend, &receivedCommonMessage, sizeof(receivedCommonMessage));
					int a;
					read(socketForSend, &a, sizeof(a));
					finalResult = finalResult + a;
					pthread_mutex_unlock(&mutex);
				}
				pthread_mutex_lock(&mutex);

				printf("Response received: %d, forwarded to client\n", finalResult);
				fflush(stdout);

				handledRequest++;
				write(receivedCommonMessage.responseConnectionFd, &finalResult, sizeof(finalResult));
				pthread_mutex_unlock(&mutex);
			}

			if (wordCounter == 5)
			{

				printf("Request arrived: \"%s\"\n", receivedCommonMessage.requestMessage);
				fflush(stdout);

				for (int i = 0; i < indexOfServant; i++)
				{
					for (int j = 0; j < 100; j++)
					{
						if (strcmp(servantInfoArray[i].dataSetDirectoryNames[j], cityName) == 0)
						{
							pthread_mutex_lock(&mutex);

							printf("Contacting servant %d\n", servantInfoArray[i].servantPID);
							fflush(stdout);
							int socketForSend = socketConnectorForSend(servantInfoArray[i].portOfServant);
							write(socketForSend, &receivedCommonMessage, sizeof(receivedCommonMessage));
							int a;
							read(socketForSend, &a, sizeof(a));
							printf("Response received: %d, forwarded to client\n", a);
							fflush(stdout);
							handledRequest++;
							write(receivedCommonMessage.responseConnectionFd, &a, sizeof(a));
							pthread_mutex_unlock(&mutex);
						}
					}
				}
			}

			//printf("Req: %s - Word Count: %d\n", receivedCommonMessage.requestMessage, wordCounter);
			//printf("%d Port Of Servant:\n", servantInfoArray[0].portOfServant);

			//printf("receivedCommonMessagein connection fd'si: %d", receivedCommonMessage.responseConnectionFd);
		}

		if (receivedCommonMessage.typeOfMessage == 1)
		{
			//printf("Servant's directories: \n");
			for (int i = 0; i < 9; i++)
			{
				if (receivedCommonMessage.dataSetDirectoryNames[i] != NULL)
				{
					//printf("%s\n", receivedCommonMessage.dataSetDirectoryNames[i]);
				}
			}

			printf("Servant %d present at port %d handling cities %s-%s\n", receivedCommonMessage.servantPID, receivedCommonMessage.portOfServant, receivedCommonMessage.dataSetDirectoryNames[0], receivedCommonMessage.dataSetDirectoryNames[receivedCommonMessage.differenceOfBounds]);
			fflush(stdout);
			//printf("Port Num: %d\n", receivedCommonMessage.portOfServant);
			pthread_mutex_lock(&mutex);
			servantInfoArray[indexOfServant] = receivedCommonMessage;
			//printf("Servant")
			indexOfServant++;
			pthread_mutex_unlock(&mutex);
		}
	}
}

// Driver function
int main(int argc, char *argv[])
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_handler = &sig_handler;
	sigaction(SIGINT, &act, NULL);
	while ((opt = getopt(argc, argv, "p:t:")) != -1) //command line argumands flag putter.
	{
		switch (opt)
		{
		case 'p':
			portValue = atoi(optarg);
			break;
		case 't':
			numberOfThreads = atoi(optarg);
			break;
		case '?':
			if (optopt == 'p')
			{
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				exit(EXIT_FAILURE);
			}
			if (optopt == 't')
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
	//printf("pval:%d tval:%d\n",portValue,numberOfThreads);
	pthread_t idServerThreads[numberOfThreads];
	for (int i = 0; i < numberOfThreads; i++)
	{
		//printf("receivedCommonMessage!\n");
		pthread_create(&idServerThreads[i], NULL, serverThreads, (void *)(intptr_t)i);
	}

	socketConnectorForReceive();

	len = sizeof(cli);
	/*
	connfd = accept(sockfd, (SA *)&cli, &len);
	char buff[MAX];
	struct node *receivedCommonMessage;
	struct node *ptr;	
	receivedCommonMessage = (struct node *)malloc(sizeof(struct node));
	read(connfd, receivedCommonMessage, sizeof(receivedCommonMessage));
	ptr=receivedCommonMessage;
	printf("receivedCommonMessage info: %s\n",ptr->info);
	ptr=ptr->next;
	printf("receivedCommonMessage info: %s\n",ptr->info);
	*/

	// Accept the data packet from client and verification

	//else
	//printf("server accept the client...\n");

	while (keep_running == 1)
	{
		connfd = accept(sockfd, (SA *)&cli, &len);
		commonMessage reqFromClient;
		read(connfd, &reqFromClient, sizeof(reqFromClient));
		reqFromClient.responseConnectionFd = connfd;
		//printf("Type of Message: %d\n", reqFromClient.typeOfMessage);
		//printf("receivedCommonMessage Message of Server: %s\n",reqFromClient.requestMessage);
		/*
		for (int i = 0; i < strlen(reqFromClient.requestMessage); i++)
		{
			if (reqFromClient.requestMessage[i] == '\n')
			{
				for (int j = i; j < strlen(reqFromClient.requestMessage); j++)
				{
					reqFromClient.requestMessage[j] = '\0';
				}
			}
		}*/
		//printf("Clean Message: %s\n",buff);
		pthread_mutex_lock(&mutex2);
		enQueue(reqFromClient);
		pthread_cond_signal(&c);
		pthread_mutex_unlock(&mutex2);
	}
	close(sockfd);
	printf("SIGINT has been received. I handled a total of %d requests. Goodbye.\n", handledRequest);
	fflush(stdout);
	for (int i = 0; i < indexOfServant; i++)
	{
		kill(servantInfoArray[i].servantPID, SIGINT);
	}
	for (int i = 0; i < numberOfThreads; i++)
	{
		pthread_kill(idServerThreads[i],SIGINT);
	}	
	exit(1);
}

void enQueue(commonMessage message)
{
	if (rear == QUEUESIZE - 1)
		printf("\nQueue is Full!!");
	else
	{
		if (front == -1)
			front = 0;
		rear++;
		items[rear] = message;
		sizeOfQueue++;
		//strcpy(items[rear], value);
		//printf("\nInserted -> %s", value);
	}
}

commonMessage deQueue()
{
	if (front > rear)
		front = rear = -1;
	if (front == -1)
	{
	}

		//printf("\nDeleted : %s", items[front]);
		sizeOfQueue--;
		return items[front++];

	
}

// Function to print the queue
void display()
{
	if (rear == -1)
	{
	}
	else
	{
		int i;
		//printf("\nQueue elements are:\n");
		for (i = front; i <= rear; i++)
		{
		}
		//printf("%s  ", items[i]);
	}
	printf("\n");
}

void socketConnectorForReceive()
{
	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
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
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(portValue);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
	{
		perror("socket bind failed...\n");
		exit(0);
	}

	//printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 50)) != 0)
	{
		printf("Listen failed...\n");
		exit(0);
	}

	//printf("Server listening..\n");
}

int socketConnectorForSend(int portForSend)
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
	//printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(portForSend);

	// connect the client socket to server socket
	if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		perror("connection with the server failed...\n");
		exit(0);
	}
	//printf("connected to the server..\n");

	return sockfd;
}
