#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;

    keep_running = 0;
}

#define BLKSIZE 1024
void arrayClear(char *inputArr, int size);
int findSize(int *willFindSize);
int main(int argc, char *argv[])
{

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL);

    // printf("\nMain Process PID:%d",(int)getpid());
    char *buffer = malloc(sizeof(char) * BLKSIZE * BLKSIZE * BLKSIZE);
    int fromfd;
    int outfd;
    char **envVec;
    char **argvOfChild;

    argvOfChild = malloc(sizeof(char *) * 100);
    for (int i = 0; i < 100; i++)
    {
        argvOfChild[i] = malloc(sizeof(char) * 100);
    }

    envVec = malloc(sizeof(char *) * 32);
    for (int i = 0; i < 32; i++)
    {
        envVec[i] = malloc(sizeof(char) * 32);
    }
    pid_t childPid; /*PID of waited for child*/
    pid_t wpid;
    int pids[100000];
    int childCount = 0;

    char *inputFilePath = NULL;
    char *outputFilePath = NULL;
    char tempStr[10];
    int opt;

    if (argc < 5 || argc > 5)
    {
        printf("Usage error! Usage have to be:./processP -i inputFilePath -o outputFilePath\n");
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "i:o:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            inputFilePath = optarg;
            break;
        case 'o':
            outputFilePath = optarg;
            break;
        case '?':
            if (optopt == 'i')
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

    strcpy(argvOfChild[0], outputFilePath);

    // printf("%s \n",argv[2]);
    fromfd = open(inputFilePath, O_RDONLY);
    if (fromfd == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }
    outfd = open(outputFilePath, O_RDONLY | O_TRUNC | O_CREAT ,0777);
    close(outfd);

    read(fromfd, buffer, 1024 * 1024 * 1024);
    //printf("Size of char array is: %ld byte\n",strlen(buffer));
    if(strlen(buffer)<60){
        printf("File must be contain AT LEAST 60 Character!\n");
        exit(EXIT_FAILURE);
    }

    printf("Process P reading %s file \n", inputFilePath);

    char *childProcess = "./childProcess";

    int k = 0;
    for (int i = 0, j = 0; i < strlen(buffer) + 1 && keep_running != 0; i++)
    {
        if (j == 30)
        {
            sprintf(tempStr, "%d", k);
            strcpy(envVec[30], tempStr);
            free(envVec[31]);
            envVec[31] = NULL;
            childPid = fork();
            pids[k] = childPid;
            childCount++;
            if (childPid == 0)
            {
                execve(childProcess, argvOfChild, envVec);
                perror("Failed to execution of Child Process");
            }
            else
            {
                k++;
                j = 0;
            }
        }
        tempStr[0] = buffer[i];
        tempStr[1] = '\0';
        strcpy(envVec[j], tempStr);
        j++;
    }
    if (keep_running == 0)
    {
        unlink(outputFilePath);
        for (int i = 0; i < childCount; i++)
        {
            kill(pids[i], SIGINT);
        }
        for (int i = 0; i < 32; i++)
        {
            free(envVec[i]);
        }
        free(envVec);

        for (int i = 0; i < 100; i++)
        {
            free(argvOfChild[i]);
        }
        free(argvOfChild);
        free(buffer);
        close(fromfd);
        exit(EXIT_FAILURE);
    }

    while ((wpid = wait(NULL)) > 0);
    printf("Reached EOF, collecting outputs from %s\n", outputFilePath);
    close(fromfd);
    fromfd = open(outputFilePath, O_RDONLY);
    if (fromfd == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }
    arrayClear(buffer, strlen(buffer));
    read(fromfd, buffer, 1024 * 1024 * 1024);
    // printf("Size of char array is: %ld byte\n",strlen(buffer));

    // int newLineCounter;
    /*
    for(int i=0;i<strlen(buffer)+1;i++){
        if(buffer[i] == '\n'){
            newLineCounter++;
        }
    }*/
    // printf("New line counter is: %d\n",newLineCounter );

    double allMatrices[childCount][3][3];
    int spaceCounter = 0;
    char wordDetector[50];
    int ithMatrix = (int)buffer[0] - 48;
    int xIndex = 0, yIndex = 0;
    int i = 0;
    int l = 0;

    while (i < strlen(buffer) + 1 && keep_running==1)
    {
        if (buffer[i] == ' ')
        {
            spaceCounter++;
            l = 0;
            // printf("%s\n",wordDetector);
            if (spaceCounter == 1)
            {
                sscanf(wordDetector, "%d", &ithMatrix);
                // printf("ith Matrix: %d\n",ithMatrix);
                arrayClear(wordDetector, 50);
            }
            else
            {
                sscanf(wordDetector, "%lf", &allMatrices[ithMatrix][xIndex][yIndex]);
                arrayClear(wordDetector, 50);
                yIndex++;
            }
            if (spaceCounter == 4 || spaceCounter == 7)
            {
                arrayClear(wordDetector, 50);
                xIndex++;
                yIndex = 0;
            }

            if (spaceCounter == 10)
            {
                arrayClear(wordDetector, 50);
                xIndex = 0;
                yIndex = 0;
                spaceCounter = 0;
            }
        }
        wordDetector[l] = buffer[i];
        l++;
        i++;
    }
       if (keep_running == 0)
    {
        unlink(outputFilePath);
        for (int i = 0; i < 32; i++)
        {
            free(envVec[i]);
        }
        free(envVec);

        for (int i = 0; i < 100; i++)
        {
            free(argvOfChild[i]);
        }
        free(argvOfChild);
        free(buffer);
        close(fromfd);
        exit(EXIT_FAILURE);
    }

    /*
    for(int i=0;i<childCount;i++){
            for(int j=0;j<3;j++){
                for(int k=0;k<3;k++){
                    printf("%lf ",allMatrices[i][j][k]);
                }
                printf("\n");
            }
            printf("\n");
        }*/

    double frobeniusNorms[childCount];
    double frobeniusNorm = 0;

    for (int i = 0; i < childCount && keep_running==1; i++)
    {
        // printf("R_%d ",i);
        for (int j = 0; j < 3; j++)
        {
            for (int k = 0; k < 3; k++)
            {
                // printf("%.3lf ",allMatrices[i][j][k]);
                frobeniusNorm += (allMatrices[i][j][k] * allMatrices[i][j][k]);
                // printf("Frob inner loop step by step: %lf \n",frobeniusNorm);
            }
        }
        frobeniusNorms[i] = sqrt(frobeniusNorm);
         //printf("Frobenius norm of %d th: is %lf \n", i,sqrt(frobeniusNorm));
        frobeniusNorm = 0;
    }
    

    double min_distance = INFINITY;
    int minDistanceFirstArrayIndex;
    int minDistaceSecondArrayIndex;
    double distance;
    for (int i = 0; i < childCount - 1 && keep_running==1; i++)
    {
        for (int j = i + 1; j < childCount; j++)
        {
            distance = fabs(frobeniusNorms[i] - frobeniusNorms[j]);
            if (distance < min_distance)
            {
                min_distance = distance;
                minDistanceFirstArrayIndex = i;
                minDistaceSecondArrayIndex = j;
            }
        }
    }
    if (keep_running == 0)
    {
        unlink(outputFilePath);
        for (int i = 0; i < 32; i++)
        {
            free(envVec[i]);
        }
        free(envVec);

        for (int i = 0; i < 100; i++)
        {
            free(argvOfChild[i]);
        }
        free(argvOfChild);
        free(buffer);
        close(fromfd);
        exit(EXIT_SUCCESS);
    }

    printf("The closest 2 matrices are R_%d and R_%d, and their distance is %lf\n", minDistanceFirstArrayIndex, minDistaceSecondArrayIndex, min_distance);

    for (int i = 0; i < 32; i++)
    {
        free(envVec[i]);
    }
    free(envVec);

    for (int i = 0; i < 100; i++)
    {
        free(argvOfChild[i]);
    }
    free(argvOfChild);
    free(buffer);
    close(fromfd);
    return 0;
}

int findSize(int *willFindSize)
{

    int sizeCounter = 0;

    for (int i = 0; willFindSize[i] != '\0'; i++)
    {
        sizeCounter++;
    }

    return sizeCounter;
}

void arrayClear(char *inputArr, int size)
{

    for (int i = 0; i < size; i++)
    {
        inputArr[i] = '\0';
    }
}