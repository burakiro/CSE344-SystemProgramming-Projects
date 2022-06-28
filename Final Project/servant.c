#define _GNU_SOURCE
#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <netdb.h>
#include <sys/ipc.h>
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
#include <dirent.h>
#include "utils.h"
#define SA struct sockaddr
int opt, portValue;
int handledRequest = 0;
char *directoryPath = NULL;
char *ipValue = NULL;
char *cityRange = NULL;
int findPID(char[]);
void enQueue(commonMessage);
commonMessage deQueue();
void displayLinkedList();
void display();
void insert_end(char[]);
int socketConnectorForSendServer();
int socketConnectorForReceive(int);
int compareDates(char[], char[]);
commonMessage items[QUEUESIZE];
int front = -1, rear = -1;
int sizeOfQueue = 0;
struct node *start = NULL;
commonMessage servantDataSet;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;

    keep_running = 0;
}
void *servantThread(void *vargp)
{
    pthread_mutex_lock(&mutex);
    long confdinst = (long)vargp;
    /*
    if (sizeOfQueue == 0)
    {
        pthread_cond_wait(&c, &mutex);
    }*/
    commonMessage messageWillProcess = deQueue();

    //printf("Thread Created Successfully! %d\n", confdinst);
    //printf("servantDataSet Mesaage in Thread: %s\n", messageWillProcess.requestMessage);
    const char s[2] = " ";
    char *wordDivider;
    char city[40];
    char firstDate[20];
    char secondDate[20];
    char type[20];
    int wordCounter = 0;
    char tempRequestMessage[100];
    int response = 0;
    strcpy(tempRequestMessage, messageWillProcess.requestMessage);
    wordDivider = strtok(tempRequestMessage, s);
    while (wordDivider != NULL)
    {
        wordDivider = strtok(NULL, s);
        wordCounter++;
        if (wordCounter == 1)
        {
            strcpy(type, wordDivider);
        }
        if (wordCounter == 2)
        {
            strcpy(firstDate, wordDivider);
        }
        if (wordCounter == 3)
        {
            strcpy(secondDate, wordDivider);
        }
        if (wordCounter == 4 && wordDivider != NULL)
        {
            strcpy(city, wordDivider); //printf("City name: %s\n",wordDivider);
        }
    }

    if (wordCounter == 5)
    {
        //printf("Type %s, Date1: %s , Date2: %s , City: %s\n", type, firstDate, secondDate, city);
        struct dirent **fileList;
        int returnValueOfScanDir;
        FILE *fp;
        int i = 0;
        char newDirectory[500];
        strcpy(newDirectory, directoryPath);
        strcat(newDirectory, "/");
        strcat(newDirectory, city);
        //printf("New directory: %s\n", newDirectory);
        returnValueOfScanDir = scandir(newDirectory, &fileList, NULL, alphasort);
        if (returnValueOfScanDir < 0)
            perror("scandir");
        else
        {
            while (i < returnValueOfScanDir)
            {
                if ((strcmp(fileList[i]->d_name, ".")) == 0 || (strcmp(fileList[i]->d_name, "..")) == 0)
                {
                    i++;
                    //free(fileList[i]);
                    continue;
                }
                char tempFileName[300];
                strcpy(tempFileName, fileList[i]->d_name);
                if (((compareDates(firstDate, tempFileName) == 1) && (compareDates(secondDate, tempFileName) == -1)) || (compareDates(firstDate, tempFileName) == 0) || compareDates(secondDate, tempFileName) == 0)
                {
                    //printf("TEstingso\n");
                    char enterDirectory[300];
                    strcpy(enterDirectory, newDirectory);
                    strcat(enterDirectory, "/");
                    strcat(enterDirectory, fileList[i]->d_name);
                    //printf("Enter Directory: %s",enterDirectory);
                    fp = fopen(enterDirectory, "r");
                    if (fp == NULL)
                        exit(EXIT_FAILURE);
                    int bufferLength = 255;
                    char buffer[bufferLength]; /* not ISO 90 compatible */
                    while (fgets(buffer, bufferLength, fp))
                    {
                        //printf("%s\n", buffer);
                        char *token;
                        token = strtok(buffer, s);
                        token = strtok(NULL, s);
                        //printf("Token:%s\n",token);
                        //printf("Type:%s\n",type);
                        if (strcmp(token, type) == 0)
                        {
                            response++;
                        }
                    }
                    fclose(fp);
                }

                //printf("fffsss%s\n", fileList[i]->d_name);
                ++i;
                //free(fileList[i]);
            }
            //free(fileList);
        }

        //printf("New Directory: %s || Response: %d || Type: %s\n", newDirectory, response, type);
        handledRequest++;
        write(confdinst, &response, sizeof(response));
    }

    if (wordCounter == 4)
    {
        struct dirent **fileList;
        int returnValueOfScanDir;

        int i = 0;
        struct node *ptr;
        if (start == NULL)
        {
            //printf("nList is empty:n");
            pthread_exit(NULL);
        }
        else
        {
            ptr = start;
            //printf("nThe List elements are:n");
            while (ptr != NULL)
            {

                char newDirectory[500];
                strcpy(newDirectory, directoryPath);
                strcat(newDirectory, "/");
                strcat(newDirectory, ptr->info);
                returnValueOfScanDir = scandir(newDirectory, &fileList, NULL, alphasort);
                //printf("New Directory: %s\n", newDirectory);
                if (returnValueOfScanDir < 0)
                    perror("scandir");
                else
                {
                    while (i < returnValueOfScanDir)
                    {
                        if ((strcmp(fileList[i]->d_name, ".")) == 0 || (strcmp(fileList[i]->d_name, "..")) == 0)
                        {
                            i++;
                            //free(fileList[i]);
                            continue;
                        }
                        char tempFileName[300];
                        strcpy(tempFileName, fileList[i]->d_name);
                        if (((compareDates(firstDate, tempFileName) == 1) && (compareDates(secondDate, tempFileName) == -1)) || (compareDates(firstDate, tempFileName) == 0) || compareDates(secondDate, tempFileName) == 0)
                        {
                            //printf("TEstingso\n");
                            FILE *fp;
                            char enterDirectory[300];
                            strcpy(enterDirectory, newDirectory);
                            strcat(enterDirectory, "/");
                            strcat(enterDirectory, fileList[i]->d_name);
                            //printf("Enter Directory: %s\n",enterDirectory);
                            fp = fopen(enterDirectory, "r");
                            if (fp == NULL)
                                exit(EXIT_FAILURE);
                            int bufferLength = 255;
                            char buffer[bufferLength]; /* not ISO 90 compatible */
                            while (fgets(buffer, bufferLength, fp))
                            {
                                //printf("%s\n", buffer);
                                char *token;
                                token = strtok(buffer, s);
                                token = strtok(NULL, s);
                                //printf("Token:%s\n",token);
                                //printf("Type:%s\n",type);
                                if (strcmp(token, type) == 0)
                                {
                                    response++;
                                }
                            }
                            fclose(fp);
                        }

                        //printf("fffsss%s\n", fileList[i]->d_name);
                        ++i;
                        //free(fileList[i]);
                    }
                    //free(fileList);
                }
                i = 0;
                ptr = ptr->next;
            }
        }

        //printf("New Directory: %s || Response: %d || Type: %s\n", directoryPath, response, type);
        //printf("Type %s, Date1: %s , Date2: %s\n", type, firstDate, secondDate);
        handledRequest++;
        write(confdinst, &response, sizeof(response));
    }
    /*
    int resp=31;
    write(confdinst,&resp,sizeof(resp));*/
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL);
    struct sockaddr_in cli;
    socklen_t len;
    while ((opt = getopt(argc, argv, "d:c:r:p:")) != -1) //command line argumands flag putter.
    {
        switch (opt)
        {
        case 'd':
            directoryPath = optarg;
            break;
        case 'c':
            cityRange = optarg;
            break;
        case 'r':
            ipValue = optarg;
            break;
        case 'p':
            portValue = atoi(optarg);
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
    char *cityRangeDelimiter = strtok(cityRange, "-");
    int lowerBoundOfRange, upperBoundOfRange;
    lowerBoundOfRange = atoi(cityRangeDelimiter);
    //printf("LowerBound: %d\n", lowerBoundOfRange);
    cityRangeDelimiter = strtok(NULL, "-");
    upperBoundOfRange = atoi(cityRangeDelimiter);
    //printf("UpperBound: %d\n", upperBoundOfRange);

    int servantPID = findPID(argv[0]);
    //printf("PID: %d\n",servantPID);

    struct dirent **namelist;
    int n;
    int indexOfCity = 1;

    n = scandir(directoryPath, &namelist, 0, alphasort);
    if (n < 0)
        perror("scandir");
    else
    {
        int i = 0;
        while (n > i)
        {
            if ((strcmp(namelist[i]->d_name, ".")) == 0 || (strcmp(namelist[i]->d_name, "..")) == 0)
            {
                i++;
                free(namelist[i]);
                continue;
            }
            if (indexOfCity >= lowerBoundOfRange && indexOfCity <= upperBoundOfRange)
            {
                //printf("%d\n",indexOfCity);
                //printf("Selamlar\n");
                insert_end(namelist[i]->d_name);
            }
            //printf("i = %d --- %s\n", i, namelist[i]->d_name);
            i++;
            free(namelist[i]);
            indexOfCity++;
        }
        free(namelist);
        displayLinkedList();
        int sockfd = socketConnectorForSendServer();

        servantDataSet.typeOfMessage = 1;
        servantDataSet.portOfServant = lowerBoundOfRange + portValue;
        servantDataSet.servantPID = servantPID;
        servantDataSet.differenceOfBounds = upperBoundOfRange - lowerBoundOfRange;
        printf("Servant %d: loaded dataset, cities %s-%s\n", servantPID, servantDataSet.dataSetDirectoryNames[0], servantDataSet.dataSetDirectoryNames[servantDataSet.differenceOfBounds]);
        fflush(stdout);
        write(sockfd, &servantDataSet, sizeof(servantDataSet));

        sockfd = socketConnectorForReceive(servantDataSet.portOfServant);
        len = sizeof(cli);
        char test1[15];
        char test2[15];
        char ourDate[15];
        strcpy(test1, "01-01-2000");
        strcpy(ourDate, "27-12-1000");
        strcpy(test2, "27-12-2078");
        /*if (((compareDates(test1, ourDate) == 1) && (compareDates(test2, ourDate) == -1)) || (compareDates(test1, ourDate) == 0) || compareDates(test2, ourDate) == 0){
            printf("Icerdeyim\n");
            exit(1);
        }*/
        //int returnValueOfComp = compareDates(test2, test1);
        //printf("Return Val Of func: %d\n", returnValueOfComp);

        while (keep_running == 1)
        {
            commonMessage reqFromClient;
            int connfd = accept(sockfd, (SA *)&cli, &len);
            read(connfd, &reqFromClient, sizeof(reqFromClient));
            //printf("Servant Port Num: %d Message received to Servant from Server: %s\n", servantDataSet.portOfServant, reqFromClient.requestMessage);
            pthread_t idForServantThread;
            //pthread_mutex_lock(&mutex);
            enQueue(reqFromClient);
            //pthread_cond_signal(&c);
            //pthread_mutex_unlock(&mutex);
            pthread_create(&idForServantThread, NULL, servantThread, (void *)(intptr_t)connfd);
        }
        close(sockfd);
        printf("Servant %d: termination message received, handled %d requests in total.\n", servantPID, handledRequest);
        fflush(stdout);
        
        exit(1);
    }
}

void displayLinkedList()
{
    struct node *ptr;
    if (start == NULL)
    {
        printf("nList is empty:n");
        return;
    }
    else
    {
        ptr = start;
        //printf("nThe List elements are:n");
        int i = 0;
        while (ptr != NULL)
        {
            //printf("%s\n", ptr->info);
            strcpy(servantDataSet.dataSetDirectoryNames[i], ptr->info);
            ptr = ptr->next;
            i++;
        }
    }
}

void insert_end(char willAddToList[])
{
    struct node *temp, *ptr;
    temp = (struct node *)malloc(sizeof(struct node));
    if (temp == NULL)
    {
        printf("nOut of Memory Space:n");
        return;
    }
    //printf("nEnter the data value for the node:t" );
    //scanf("%d",&temp->info );

    strcpy(temp->info, willAddToList);
    temp->next = NULL;
    if (start == NULL)
    {
        start = temp;
    }
    else
    {
        ptr = start;
        while (ptr->next != NULL)
        {
            ptr = ptr->next;
        }
        ptr->next = temp;
    }
}
int socketConnectorForSendServer()
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
    servaddr.sin_addr.s_addr = inet_addr(ipValue);
    servaddr.sin_port = htons(portValue);

    // connect the client socket to server socket
    if (connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connection with the server failed...\n");
        exit(0);
    }
    //printf("connected to the server..\n");

    return sockfd;
}

int socketConnectorForReceive(int portForReceive)
{
    int sockfd, option = 1;
    struct sockaddr_in servaddr;
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
    if (sockfd == -1)
    {
        perror("socket creation failed...\n");
        exit(0);
    }
    //printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(portForReceive);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA *)&servaddr, sizeof(servaddr))) != 0)
    {
        perror("socket bind failed...\n");
        exit(0);
    }

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1000)) != 0)
    {
        perror("Listen failed...\n");
        exit(0);
    }

    return sockfd;
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
        printf("\nQueue is Empty!!");
    }


        //printf("\nDeleted : %s", items[front]);
        sizeOfQueue--;
        return items[front++];
   
}

// Function to print the queue
void display()
{
    if (rear == -1)
        printf("\nQueue is Empty!!!");
    else
    {
        int i;
        //printf("\nQueue elements are:\n");
        for (i = front; i <= rear; i++)
            printf(" ");
    }
    printf("\n");
}

int compareDates(char firstDate[], char secondDate[])
{
    char firstDateCopy[50];
    char secondDateCopy[50];
    strcpy(firstDateCopy, firstDate);
    strcpy(secondDateCopy, secondDate);
    const char s[2] = "-";
    char *dateDivider;
    int wordCounter = 0;
    char dayForFirstDate[5], dayForSecondDate[5];
    char monthForFirstDate[5], monthForSecondDate[5];
    char yearForFirstDate[7], yearForSecondDate[7];
    int dayForFirstDateInt, dayForSecondDateInt;
    int monthForFirstDateInt, monthForSecondDateInt;
    int yearForFirstDateInt, yearForSecondDateInt;

    dateDivider = strtok(firstDateCopy, s);
    while (dateDivider != NULL)
    {
        wordCounter++;
        if (wordCounter == 1)
        {
            strcpy(dayForFirstDate, dateDivider);
        }
        if (wordCounter == 2)
        {
            strcpy(monthForFirstDate, dateDivider);
        }
        if (wordCounter == 3)
        {
            strcpy(yearForFirstDate, dateDivider);
        }
        dateDivider = strtok(NULL, s);
    }
    dayForFirstDateInt = atoi(dayForFirstDate);
    monthForFirstDateInt = atoi(monthForFirstDate);
    yearForFirstDateInt = atoi(yearForFirstDate);
    //printf("Day:%d Month:%d Year:%d\n", dayForFirstDateInt, monthForFirstDateInt, yearForFirstDateInt);

    wordCounter = 0;
    dateDivider = strtok(secondDateCopy, s);
    while (dateDivider != NULL)
    {
        wordCounter++;
        if (wordCounter == 1)
        {
            strcpy(dayForSecondDate, dateDivider);
        }
        if (wordCounter == 2)
        {
            strcpy(monthForSecondDate, dateDivider);
        }
        if (wordCounter == 3)
        {
            strcpy(yearForSecondDate, dateDivider);
        }
        dateDivider = strtok(NULL, s);
    }
    dayForSecondDateInt = atoi(dayForSecondDate);
    monthForSecondDateInt = atoi(monthForSecondDate);
    yearForSecondDateInt = atoi(yearForSecondDate);
    //printf("Day:%d Month:%d Year:%d\n", dayForSecondDateInt, monthForSecondDateInt, yearForSecondDateInt);

    //Eğer second date first date'den daha ileri bir tarihteyse 1 geri bir tarihteyse -1 döndüreceğiz.
    // eşitse 0 dönecek.

    if (yearForFirstDateInt < yearForSecondDateInt)
    {
        return 1;
    }
    if (yearForFirstDateInt > yearForSecondDateInt)
    {
        return -1;
    }
    if (yearForFirstDateInt == yearForSecondDateInt)
    {
        if (monthForFirstDateInt < monthForSecondDateInt)
        {
            return 1;
        }
        if (monthForFirstDateInt > monthForSecondDateInt)
        {
            return -1;
        }
        if (monthForFirstDateInt == monthForSecondDateInt)
        {
            if (dayForFirstDateInt < dayForSecondDateInt)
            {
                return 1;
            }
            if (dayForFirstDateInt < dayForSecondDateInt)
            {
                return -1;
            }
            if (dayForFirstDateInt == dayForSecondDateInt)
            {
                return 0;
            }
        }
    }

    return 1;
}

int findPID(char servantName[])
{
    DIR *procdir;
    FILE *fp;
    struct dirent *entry;
    char path[270]; // d_name + /proc + /stat
    int pid;

    // Open /proc directory.
    procdir = opendir("/proc");
    if (!procdir)
    {
        perror("opendir failed");
        return 1;
    }

    // Iterate through all files and directories of /proc.
    while ((entry = readdir(procdir)))
    {
        // Skip anything that is not a PID directory.
        //if (!is_pid_dir(entry))
        //    continue;

        // Try to open /proc/<PID>/stat.
        snprintf(path, sizeof(path), "/proc/%s/stat", entry->d_name);
        fp = fopen(path, "r");

        if (!fp)
        {
            //perror(path);
            continue;
        }

        // Get PID, process name and number of faults.
        fscanf(fp, "%d %s ",
               &pid, path);
        char *nameComparator = servantName + 2;
        char finalComparatorName[100];
        snprintf(finalComparatorName, sizeof(finalComparatorName), "(%s)", nameComparator);
        //printf("Name: %s\n",finalComparatorName);
        //printf("Path: %s",path);
        if (strcmp(path, finalComparatorName) == 0)
        {
            //printf("PID:%d",pid);
            fclose(fp);
            closedir(procdir);
            return pid;
        }
    }

    return 1;
}