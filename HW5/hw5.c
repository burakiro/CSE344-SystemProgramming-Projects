#include <math.h>
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
#include <time.h>
#define BUFFERSIZE 3000000
#define PI M_PI

int fromfd1, fromfd2;
int m;
FILE *fp;
int matrixA[2048][2048];
int matrixB[2048][2048];
int matrixC[2048][2048];
int squareMatrixSize;
int arrived = 0;
double resultRE[2048][2048];
double resultIM[2048][2048];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;

static void sig_handler(int _)
{
    (void)_;
    close(fromfd1);
    close(fromfd2);
    exit(EXIT_SUCCESS);
}

void *arrayCalculator(void *vargp)
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL); //signal handler.

    clock_t begin = clock();
    long threadId = (long)vargp; //Thread ID.
    int helper = squareMatrixSize / m;
    int startIndex = threadId * helper;

    if (threadId == m - 1)
    {
        int temp = helper * (m - 1);
        helper = squareMatrixSize - temp;
    }
    int pieceOfBMatrix[helper][squareMatrixSize];

    for (int i = 0; i < helper; i++)
    {
        for (int j = 0; j < squareMatrixSize; j++)
        {
            pieceOfBMatrix[i][j] = matrixB[j][startIndex + i];
        }
    }

    for (int i = 0; i < helper; i++)
    {
        for (int j = 0; j < squareMatrixSize; j++)
        {
            int cVals = 0;
            for (int z = 0; z < squareMatrixSize; z++)
            {
                cVals = cVals + pieceOfBMatrix[i][z] * matrixA[j][z];
            }
            matrixC[j][startIndex + i] = cVals;
        }
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    time_t tim = time(NULL); // For timestamp.
    struct tm tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Thread %li has reached the rendezvous point in %.4lf seconds.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, threadId + 1, time_spent);
    pthread_mutex_lock(&mutex);
    ++arrived;
    if (arrived < m) // if this thread is not lastcwait(c,m)// then wait for otherselsebroadcast(c)// i’m last, awaken the other N-1unlock(m)
    {
        pthread_cond_wait(&c, &mutex);
    }
    else
    {
        pthread_cond_broadcast(&c);
    }
    pthread_mutex_unlock(&mutex);
    tim = time(NULL); // For timestamp.
    tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Thread %li is advancing to the second part.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, threadId + 1);
    for (int z = 0; z < helper; z++) // int z = 0, j = startIndex + z; z < helper; z++
    {
        int j = startIndex + z;
        for (int i = 0; i < squareMatrixSize; i++) // int i = 0; i < squareMatrixSize; i++
        {
            double ak = 0;
            double bk = 0;
            for (int ii = 0; ii < squareMatrixSize; ii++)
            {
                for (int jj = 0; jj < squareMatrixSize; jj++)
                {

                    double x = -2.0 * PI * i * ii / (double)squareMatrixSize;
                    double y = -2.0 * PI * j * jj / (double)squareMatrixSize;
                    ak += matrixC[ii][jj] * cos(x + y);
                    bk += matrixC[ii][jj] * 1.0 * sin(x + y);
                }
            }
            resultRE[i][j] = ak;
            resultIM[i][j] = bk;
        }
    }
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    tim = time(NULL); // For timestamp.
    tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Thread %li has finished the second part in %.4lf seconds.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, threadId + 1, time_spent);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&c);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL); //signal handler.
    clock_t begin = clock();
    char *filePath1 = NULL;
    char *filePath2 = NULL;
    char *outputFile = NULL;
    int opt, n;
    char *buffer1 = malloc(BUFFERSIZE);
    char *buffer2 = malloc(BUFFERSIZE);

    while ((opt = getopt(argc, argv, "i:j:o:n:m:")) != -1) //command line argumands flag putter.
    {
        switch (opt)
        {
        case 'i':
            filePath1 = optarg;
            break;
        case 'j':
            filePath2 = optarg;
            break;
        case 'o':
            outputFile = optarg;
            break;
        case 'n':
            n = atoi(optarg);
            break;
        case 'm':
            m = atoi(optarg);
            break;
        case '?':
            if (optopt == 'i')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'j')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'o')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'n')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'm')
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

    if (n <= 2)
    {
        printf("Enter a valid n value! (n>2)\n");
        exit(EXIT_FAILURE);
    }

    if (m > pow(2, n))
    {
        printf("Your m value is too high, please enter a valid one!\n");
        exit(EXIT_FAILURE);
    }

    if (m < 2)
    {
        printf("m has to be >=2k and k has to be k>=1!\n");
        exit(EXIT_FAILURE);
    }
    fromfd1 = open(filePath1, O_RDONLY);
    if (fromfd1 == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }
    fromfd2 = open(filePath2, O_RDONLY);
    if (fromfd2 == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }

    read(fromfd1, buffer1, BUFFERSIZE); //File read.
    read(fromfd2, buffer2, BUFFERSIZE); //File read.

    int xIndex = 0, yIndex = 0;
    squareMatrixSize = pow(2, n);
    
    if (strlen(buffer1) < squareMatrixSize * squareMatrixSize)
    {
        printf("Input1 has insufficient content!\n");
        exit(EXIT_FAILURE);
    }

    if (strlen(buffer2) < squareMatrixSize * squareMatrixSize)
    {
        printf("Input2 has insufficient content!\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < pow(squareMatrixSize, 2); i++)
    {
        matrixA[xIndex][yIndex] = (int)buffer1[i];
        if (i % squareMatrixSize == squareMatrixSize - 1)
        {
            xIndex++;
            yIndex = 0;
            continue;
        }
        yIndex++;
    }

    xIndex = 0, yIndex = 0;
    for (int i = 0; i < pow(squareMatrixSize, 2); i++)
    {
        matrixB[xIndex][yIndex] = (int)buffer2[i];
        if (i % squareMatrixSize == squareMatrixSize - 1)
        {
            xIndex++;
            yIndex = 0;
            continue;
        }
        yIndex++;
    }

    pthread_t idArrayCalculators[m];

    if (m > squareMatrixSize)
    {
        m = squareMatrixSize;
    }
    time_t tim = time(NULL); // For timestamp.
    struct tm tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Two matrices of size %dx%d have been read. The number of threads is %d \n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, squareMatrixSize, squareMatrixSize, m);

    for (int i = 0; i < m; i++)
    {
        pthread_create(&idArrayCalculators[i], NULL, arrayCalculator, (void *)(intptr_t)i); //consumer threads create.
    }

    for (int i = 0; i < m; i++)
    {
        pthread_join(idArrayCalculators[i], NULL); //wait for consumers end.
    }


    remove(outputFile); // remove output file if exist.
    fp = fopen(outputFile, "w");

    for (int i = 0; i < squareMatrixSize; i++)
    {
        for (int j = 0; j < squareMatrixSize; j++)
        {
            fprintf(fp, "%.3lf + (%.3lf)i\t", resultRE[i][j], resultIM[i][j]);
        }
        fprintf(fp, "\n");
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    tim = time(NULL); // For timestamp.
    tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] The process has written the output file. The total time spend is %.3lf seconds.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, time_spent);

    fclose(fp);
    free(buffer1);
    free(buffer2);
    close(fromfd1);
    close(fromfd2);

    return 0;
}
