#include <stdio.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <signal.h>
#include "fifo_seqnum.h"
#include <time.h>
#include <sys/time.h>

#define BLKSIZE 1024

/* fifo_seqnum_client.c

   A simple client that uses a well-known FIFO to request a (trivial)
   "sequence number service". This client creates its own FIFO (using a
   convention agreed upon by client and server) which is used to receive a reply
   from the server. The client then sends a request to the server consisting of
   its PID and the length of the sequence it wishes to be allocated. The client
   then reads the server's response and displays it on stdout.

   See fifo_seqnum.h for the format of request and response messages.

   The server is in fifo_seqnum_server.c.
*/

static char clientFifo[CLIENT_FIFO_NAME_LEN];
static void /* Invoked on exit to delete client FIFO */
removeFifo(void)
{
    unlink(clientFifo);
}

/* Convert a numeric command-line argument string to an integer. See the
   comments for getNum() for a description of the arguments to this function. */
int squareMatrixController(int elements[99], int IndexOfX);
void arrayClear(char *inputArr, int size);
void chekFileIsOpen();
int main(int argc, char *argv[])
{
    int fromfd;
    char *buffer = malloc(3000);
    int opt;
    char *pathToServerFifo = NULL;
    char *pathToDataFile = NULL;
    while ((opt = getopt(argc, argv, "s:o:")) != -1)
    {
        switch (opt)
        {
        case 's':
            pathToServerFifo = optarg;
            break;
        case 'o':
            pathToDataFile = optarg;
            break;
        case '?':
            if (optopt == 's')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'o')
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

    int serverFd, clientFd;
    struct request req;
    struct response resp;

    fromfd = open(pathToDataFile, O_RDONLY);
    if (fromfd == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }
    read(fromfd, buffer, 3000);
    req.pid = getpid();

    char tempStr[20];
    int matrixRowElementsSizes[99];
    //int matrix[99][99];
    int xIndex = 0, yIndex = 0;
    int reqMatrixIndex = 0;
    for (int i = 0, j = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] >= 48 && buffer[i] <= 57)
        {
            tempStr[j] = buffer[i];
            j++;
        }
        if (buffer[i] == ',')
        {
            int val = atoi(tempStr);
            //matrix[xIndex][yIndex] = val;
            req.matrix[reqMatrixIndex] = val;
            yIndex++;
            reqMatrixIndex++;
            arrayClear(tempStr, 20);
            j = 0;
        }
        if (buffer[i] == '\n')
        {
            int val = atoi(tempStr);
            //matrix[xIndex][yIndex] = val;
            matrixRowElementsSizes[xIndex] = yIndex + 1;
            req.matrix[reqMatrixIndex] = val;
            reqMatrixIndex++;
            yIndex = 0;
            xIndex++;
            arrayClear(tempStr, 20);
            j = 0;
        }
    }

    //printf("xIndex= %d\n",xIndex);
    //printf("Determinant: %f\n",det);
    int isSquareMatrix = squareMatrixController(matrixRowElementsSizes, xIndex);
    if (isSquareMatrix == 0)
    {
        printf("Matrix is not square or your input is wrong.\n");
        exit(EXIT_SUCCESS);
    }
    req.matrixDimension = matrixRowElementsSizes[0];

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct timeval begin_time;
    struct timeval end_time;
    gettimeofday(&begin_time, NULL);
    //clock_t begin = clock();
    printf("[%d-%02d-%02d %02d:%02d:%02d] Client PID#%d (%s) is submitting a %dx%d matrix\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), pathToDataFile, matrixRowElementsSizes[0], matrixRowElementsSizes[0]);

    //printf("%sClient PID#%d (%s) is submitting a %dx%d matrix\n", asctime(ptm), getpid(), pathToDataFile, matrixRowElementsSizes[0], matrixRowElementsSizes[0]);

    /* Create our FIFO (before sending request, to avoid a race) */

    umask(0); /* So we get the permissions we want */
    snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, (long)getpid());
    //open(clientFifo, O_CREAT | O_RDWR);
    if (mkfifo(clientFifo, 0777) == -1 && errno != EEXIST)
    {
        perror("Error!: ");
        exit(EXIT_SUCCESS);
    }

    if (atexit(removeFifo) != 0)
    {
        perror("Error!: ");
        exit(EXIT_SUCCESS);
    }

    serverFd = open(pathToServerFifo, O_RDWR, 0777);
    if (serverFd == -1)
    {
        perror("Error occured when Server opening!\n");
        exit(EXIT_SUCCESS);
    }

    if (write(serverFd, &req, sizeof(struct request)) !=
        sizeof(struct request))
    {
        printf("Can't write to server\n");
        exit(EXIT_SUCCESS);
    }

    /* Open our FIFO, read and display response */
    clientFd = open(clientFifo, O_RDWR, 0777);
    if (clientFd == -1)
    {
        perror("Error occuring when opening client fifo!\n");
        exit(EXIT_SUCCESS);
    }

    if (read(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
    {
        printf("Can't read response from server");
        exit(EXIT_SUCCESS);
    }
    gettimeofday(&end_time, NULL);
    //clock_t end = clock();
    //double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    long int timeDiff = end_time.tv_sec - begin_time.tv_sec;


    t = time(NULL);
    tm = *localtime(&t);

    /* here, do your time-consuming job */

    if ((resp.invertible == 1))
    {
        printf("Invertibility: %d\n", resp.invertible);
        printf("[%d-%02d-%02d %02d:%02d:%02d] Client PID#%d: the matrix is invertible, total time %ld seconds, goodbye.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), timeDiff);
    }
    if ((resp.invertible == 0))
    {
        printf("[%d-%02d-%02d %02d:%02d:%02d] Client PID#%d: the matrix is not invertible, total time %ld seconds, goodbye.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), timeDiff);
    }

    exit(EXIT_SUCCESS);
}
void arrayClear(char *inputArr, int size)
{

    for (int i = 0; i < size; i++)
    {
        inputArr[i] = '\0';
    }
}

int squareMatrixController(int elements[99], int IndexOfX)

{

    for (int i = 0; i < IndexOfX; i++)
    {
        if (elements[i] == IndexOfX)
        {
            continue;
        }
        else
        {
            return 0;
        }
    }
    return 1;
}
