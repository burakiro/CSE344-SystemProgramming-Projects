#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include "fifo_seqnum.h"
#include <sys/mman.h>
#include "become_daemon.h"
#include <time.h>

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;

    keep_running = 0;
    unlink("doubleSafer");
}

/* fifo_seqnum_server.c

   An example of a server using a FIFO to handle client requests.
   The "service" provided is the allocation of unique sequential
   numbers. Each client submits a request consisting of its PID, and
   the length of the sequence it is to be allocated by the server.
   The PID is used by both the server and the client to construct
   the name of the FIFO used by the client for receiving responses.

   The server reads each client request, and uses the client's FIFO
   to send back the starting value of the sequence allocated to that
   client. The server then increments its counter of used numbers
   by the length specified in the client request.

   See fifo_seqnum.h for the format of request and response messages.

   The client is in fifo_seqnum_client.c.
*/
void arrayClear(char *inputArr, int size);
int squareMatrixController(int elements[99], int IndexOfX);
float determinant(int a[99][99], int k);
void writeToFile(char *inputString, char *writeFilePath);
int becomeDaemon(int flags);
void checkFileIsOpen();
int main(int argc, char *argv[])
{   
    //skeleton_daemon();
    checkFileIsOpen();
    
    becomeDaemon(1);
    int serverFd, serverFdForZ, dummyFd, clientFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    struct request req;
    struct response resp;
    char *pathToServerFifo = NULL;
    char *pathToLogFile = NULL;
    int opt;
    int poolsize,poolsize2, t; 
    int fd[2];

    char *pathForServerZFifo = "/tmp/serverZFifo_sv";
    //char *argvForZ[5];

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL);

    /* place semaphore in shared memory */
    sem_t *sema = mmap(NULL, sizeof(*sema), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sema == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(sema, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sem_t *semaSucces = mmap(NULL, sizeof(*semaSucces), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semaSucces == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(semaSucces, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sem_t *semaFail = mmap(NULL, sizeof(*semaFail), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semaFail == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(semaFail, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sem_t *semaForward = mmap(NULL, sizeof(*semaForward), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semaForward == MAP_FAILED)
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(semaForward, 1, 0) < 0)
    {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    if (pipe(fd) < 0)
    {
        perror("pipe");
        exit(1);
    }

    /* Create well-known FIFO, and open it for reading */

    while ((opt = getopt(argc, argv, "s:o:p:r:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            pathToServerFifo = optarg;
            break;
        case 'o':
            pathToLogFile = optarg;
            break;
        case 'p':
            poolsize = atoi(optarg);
            break;
        case 'r':
            poolsize2 = atoi(optarg);
            break;
        case 't':
            t = atoi(optarg);
            break;
        case '?':
            if (optopt == 's')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'o')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'p')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'r')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            if (optopt == 't')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                unlink("doubleSafer");
                exit(EXIT_FAILURE);
            }
        }
    }
    int pids[poolsize];
    if(poolsize<2){
        char tempStrFirst[50]= "p value can not be smaller than 2!\n";
        writeToFile(tempStrFirst,pathToLogFile);
        unlink("doubleSafer");
        exit(EXIT_SUCCESS); 
    }
    if(poolsize2<2){
        char tempStrFirst[50]= "r value can not be smaller than 2!\n";
        writeToFile(tempStrFirst,pathToLogFile);
        unlink("doubleSafer");
        exit(EXIT_SUCCESS); 
    }
    printf("Server Y (%s, p=%d, t=%d) started\n", pathToLogFile, poolsize, t);
    char *serverZ = "./serverZ";
    pid_t pidZ = fork();
    if (pidZ == 0)
    {
        execv(serverZ, argv);
    }

    for (int i = 0; i < poolsize; i++)
    {
        pid_t childPid = fork();
        pids[i] = childPid;
        switch (childPid)
        {
        case -1:
            perror("Forking error! ");
            exit(1);
        case 0:
            close(fd[1]);
            while (keep_running == 1)
            {
                sigset_t set;
                sigfillset(&set);
                sigdelset(&set, SIGINT);
                sigprocmask(SIG_BLOCK, &set, NULL);

                struct sigaction act;
                memset(&act, 0, sizeof(act));
                act.sa_handler = &sig_handler;
                sigaction(SIGINT, &act, NULL);
                read(fd[0], &req, sizeof(struct request));
                snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
                         (long)req.pid);
                clientFd = open(clientFifo, O_RDWR, 0777);
                if (clientFd == -1)
                { /* Open failed, give up on client */
                    perror("Client Fifo Openning Error: ");
                    unlink("doubleSafer");
                    break;
                }
                int semVal;
                sem_getvalue(sema, &semVal);
                //printf("Children Semaphore: %d\n", semVal);
                char tempString[500];

                //printf("[%d-%02d-%02d %02d:%02d:%02d] Worker PID#%d is handling client PID#%d , %dx%d matrix size, pool busy %d/%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), req.pid, req.matrixDimension, req.matrixDimension, semVal, poolsize);
                sprintf(tempString, "Worker PID#%d is handling client PID#%d , %dx%d matrix size, pool busy %d/%d\n", getpid(), req.pid, req.matrixDimension, req.matrixDimension, semVal, poolsize);
                //printf("%s",tempString);
                writeToFile(tempString, pathToLogFile);
                int matrix[99][99];
                for (int i = 0, matrixXIndex = 0, matrixYIndex = 0; i < req.matrixDimension * req.matrixDimension; i++)
                {

                    matrix[matrixXIndex][matrixYIndex] = req.matrix[i];
                    matrixYIndex++;
                    if (matrixYIndex == req.matrixDimension)
                    {
                        matrixYIndex = 0;
                        matrixXIndex++;
                    }
                }

                /*
                for(int i=0;i<req.matrixDimension;i++){
                    for(int j = 0;j<req.matrixDimension; j++){
                        printf("%d ", matrix[i][j]);
                    }
                    printf("\n");
                }
*/
                sleep(t);
                int det = determinant(matrix, req.matrixDimension);
                resp.invertible = det;
                if (det == 0)
                {
                    arrayClear(tempString, 500);
                    sprintf(tempString, "Worker PID#%d responding to client PID%d: the matrix IS NOT invertible.\n", getpid(), req.pid);
                    writeToFile(tempString, pathToLogFile);
                    sem_post(semaFail);
                    //printf("Worker PID#%d responding to client PID%d: the matrix IS NOT invertible.\n", getpid(), req.pid);
                }
                else
                {
                    arrayClear(tempString, 500);
                    sprintf(tempString, "Worker PID#%d responding to client PID%d: the matrix IS invertible.\n", getpid(), req.pid);
                    writeToFile(tempString, pathToLogFile);
                    sem_post(semaSucces);
                    //printf("Worker PID#%d responding to client PID%d: the matrix IS invertible.\n", getpid(), req.pid);
                }

                resp.childPID = getpid();

                if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
                {
                    perror("Error writing to FIFO %s\n");
                    unlink("doubleSafer");
                    break;
                }

                if (close(clientFd) == -1)
                {
                    perror("Closing clientFd Error: \n");
                    unlink("doubleSafer");
                    break;
                }
                if (sem_wait(sema) < 0)
                {
                    perror("Semaphore value decrement error! ");
                    unlink("doubleSafer");
                    break;
                }
                unlink(clientFifo);
                
            }
            if (keep_running == 0)
            {
                raise(SIGINT);
                exit(EXIT_FAILURE);
            }
        default:
            break;
        }
    }

    close(fd[0]);
    umask(0); /* So we get the permissions we want */
    if (mkfifo(pathToServerFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("Error occuring while fifo");
        unlink("doubleSafer");
        exit(1);
    }

    serverFd = open(pathToServerFifo, O_RDWR, 0777);
    if (serverFd == -1)
    {
        perror("ServerFD Openning Error: ");
        unlink("doubleSafer");
        exit(1);
    }

    /* Open an extra write descriptor, so that we never see EOF */

    dummyFd = open(pathToServerFifo, O_RDWR, 0777);
    if (dummyFd == -1)
    {
        perror("Dummyfd openning error: ");
        unlink("doubleSafer");
        exit(1);
    }

    while (keep_running == 1)
    { /* Read requests and send responses */
        if (read(serverFd, &req, sizeof(struct request)) != sizeof(struct request))
        {
            perror("Reading from client error!: ");
            break; /* Either partial read or error */
        }

        int semaValue;
        sem_getvalue(sema, &semaValue);
        if (semaValue >= poolsize)
        {
            sem_post(semaForward);
            //printf("Request now forwarding to Z\n");
            serverFdForZ = open(pathForServerZFifo, O_RDWR, 0777);
            if (serverFdForZ == -1)
            {
               perror("Error occuring while openin server path.\n");
               unlink("doubleSafer");
                break;
            }

            if (write(serverFdForZ, &req, sizeof(struct request)) != sizeof(struct request))
            {
                printf("Can't write to server\n");
                unlink("doubleSafer");
                break;
            }
        }
        else
        {
            //printf("Request has received from: %d\n", req.pid);
            /* Open client FIFO (previously created by client) */
            if (sem_post(sema) < 0)
            {
                perror("Error occuring while post Semaphore operation.");
                unlink("doubleSafer");
                break;
            }

            sem_getvalue(sema, &semaValue);
            //printf("Request semaphore: %d", semaValue);
            write(fd[1], &req, sizeof(struct request));
            //printf("buraya geldim %d\n", seqNum);
        }
    }

    if (keep_running == 0)
    {
        kill(pidZ, SIGINT);
        int succesVal, failVal, forwardVal;
        sem_getvalue(semaSucces, &succesVal);
        sem_getvalue(semaFail, &failVal);
        sem_getvalue(semaForward, &forwardVal);
        char tempLastString[150];
        sprintf(tempLastString, "SIGINT Received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded.\n", succesVal + failVal + forwardVal, succesVal, failVal, forwardVal);
        printf(tempLastString, "SIGINT Received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded.\n", succesVal + failVal + forwardVal, succesVal, failVal, forwardVal);
        writeToFile(tempLastString, pathToLogFile);      
        for (int i = 0; i < poolsize; i++)
        {
            kill(pids[i], SIGINT);
        }
        raise(SIGINT);
        unlink("doubleSafer");
        unlink(clientFifo);
        exit(EXIT_FAILURE);
    }
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

/*For calculating Determinant of the Matrix */
float determinant(int a[99][99], int k)
{
    int s = 1;
    float det = 0;
    int b[99][99];
    int i, j, m, n, c;
    if (k == 1)
    {
        return (a[0][0]);
    }
    else
    {
        det = 0;
        for (c = 0; c < k; c++)
        {
            m = 0;
            n = 0;
            for (i = 0; i < k; i++)
            {
                for (j = 0; j < k; j++)
                {
                    b[i][j] = 0;
                    if (i != 0 && j != c)
                    {
                        b[m][n] = a[i][j];
                        if (n < (k - 2))
                            n++;
                        else
                        {
                            n = 0;
                            m++;
                        }
                    }
                }
            }
            det = det + s * (a[0][c] * determinant(b, k - 1));
            s = -1 * s;
        }
    }
    return (det);
}

void writeToFile(char *inputString, char *writeFilePath)
{
    time_t tim = time(NULL);
    struct tm tm = *localtime(&tim);
    int fd;
    char tempString[100];
    fd = open(writeFilePath, O_WRONLY | O_APPEND | O_CREAT, 0777);
    if (fd == -1)
    {
        perror("Failed to open input file! For Write");
        exit(1);
    }
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;

    fcntl(fd, F_SETLKW, &lock);
    //printf("[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    sprintf(tempString, "[%d-%02d-%02d %02d:%02d:%02d]", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    //printf("%s\n", tempString);

    write(fd, tempString, strlen(tempString));
    arrayClear(tempString, 100);

    write(fd, inputString, strlen(inputString));
    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLKW, &lock);
    close(fd);
}

int                                     /* Returns 0 on success, -1 on error */
becomeDaemon(int flags)
{
    int maxfd, fd;

    switch (fork()) {                   /* Become background process */
    case -1: return -1;
    case 0:  break;                     /* Child falls through... */
    default: _exit(EXIT_SUCCESS);       /* while parent terminates */
    }

    if (setsid() == -1)                 /* Become leader of new session */
        return -1;

    switch (fork()) {                   /* Ensure we are not session leader */
    case -1: return -1;
    case 0:  break;
    default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0);                       /* Clear file mode creation mask */

    if (!(flags & BD_NO_CHDIR))
        chdir("/");                     /* Change to root directory */

    if (!(flags & BD_NO_CLOSE_FILES)) { /* Close all open files */
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)                /* Limit is indeterminate... */
            maxfd = BD_MAX_CLOSE;       /* so take a guess */

        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);            /* Reopen standard fd's to /dev/null */

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }

    return 0;
}



void checkFileIsOpen(){
    char *DoubleSafer = "doubleSafer";
    if(open(DoubleSafer, O_RDWR | O_CREAT | O_EXCL, 0666) == -1) {
        if (errno == EEXIST){
            printf("Double Inst. Safer!\n");
            exit(EXIT_FAILURE);
        }
    }
}