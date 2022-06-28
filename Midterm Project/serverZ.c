#include <errno.h>
#include <sys/ipc.h>
#include <semaphore.h>
#include <ctype.h>
#include <sys/shm.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include "fifo_seqnum.h"
#include <time.h>
#include "become_daemon.h"

static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
    (void)_;

    keep_running = 0;
    unlink("doubleSafer2");
}
int becomeDaemon(int flags);
void arrayClear(char *inputArr, int size);
int squareMatrixController(int elements[99], int IndexOfX);
float determinant(int a[99][99], int k);
void writeToFile(char *inputString, char *writeFilePath);
void checkFileIsOpen();
int main(int argc, char *argv[])
{   
    //skeleton_daemon();
    //checkFileIsOpen();
    becomeDaemon(1);
    
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);

    struct sigaction act;
    memset(&act, 0, sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL);

    char *pathForServerZFifo = "/tmp/serverZFifo_sv";
    int serverFd, dummyFd, clientFd;
    char clientFifo[CLIENT_FIFO_NAME_LEN];
    //char *pathToServerFifo = NULL;
    char *pathToLogFile = NULL;
    int opt;
    struct request *req;
    struct response resp;
    int poolsize2, t; // poolsize,
    int shmid = shmget(1234, sizeof(struct request *), IPC_CREAT | 0666);
    req = shmat(shmid, NULL, 0);

    /* place semaphore in shared memory */
    sem_t *sema = mmap(NULL, sizeof(*sema), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sema == MAP_FAILED)
    {
        perror("mmap");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(sema, 1, 0) < 0)
    {
        perror("sem_init");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    sem_t *sema2 = mmap(NULL, sizeof(*sema2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sema2 == MAP_FAILED)
    {
        perror("mmap");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(sema2, 1, 3) < 0)
    {
        perror("sem_init");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    sem_t *sema3 = mmap(NULL, sizeof(*sema2), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (sema2 == MAP_FAILED)
    {
        perror("mmap");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(sema3, 1, 0) < 0)
    {
        perror("sem_init");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    sem_t *semaSucces = mmap(NULL, sizeof(*semaSucces), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semaSucces == MAP_FAILED)
    {
        perror("mmap");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(semaSucces, 1, 0) < 0)
    {
        perror("sem_init");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    sem_t *semaFail = mmap(NULL, sizeof(*semaFail), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (semaFail == MAP_FAILED)
    {
        perror("mmap");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }

    /* create/initialize semaphore */
    if (sem_init(semaFail, 1, 0) < 0)
    {
        perror("sem_init");
        unlink("doubleSafer2");
        exit(EXIT_FAILURE);
    }


    /* Create well-known FIFO, and open it for reading */

    while ((opt = getopt(argc, argv, "s:o:p:r:t:")) != -1)
    {
        switch (opt)
        {
        case 's':
            //pathToServerFifo = optarg;
            break;
        case 'o':
            pathToLogFile = optarg;
            break;
        case 'p':
            //poolsize = atoi(optarg);
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
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'o')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'p')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            if (optopt == 'r')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            if (optopt == 't')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            if (isprint(optopt))
            {
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                unlink("doubleSafer2");
                exit(EXIT_FAILURE);
            }
            else
            {
                fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
                exit(EXIT_FAILURE);
            }
        }
    }
    int pids[poolsize2];
    printf("Server Z (%s, p=%d, t=%d) started\n", pathToLogFile, poolsize2, t);
    if (mkfifo(pathForServerZFifo, S_IRUSR | S_IWUSR | S_IWGRP) == -1 && errno != EEXIST)
    {
        perror("Error occuring while fifo");
        unlink("doubleSafer2");
        exit(1);
    }
    serverFd = open(pathForServerZFifo, O_RDWR, 0777);
    if (serverFd == -1)
    {
        perror("ServerFD Openning Error: ");
        unlink("doubleSafer2");
        exit(1);
    }

    /* Open an extra write descriptor, so that we never see EOF */

    for (int i = 0; i < poolsize2; i++)
    {
        pid_t childPid = fork();
        pids[i] = childPid;
        switch (childPid)
        {
        case -1:
            perror("Forking error! ");
            unlink("doubleSafer2");
            exit(1);
        case 0:
            while (keep_running == 1)
            {
                //time_t tim = time(NULL);
                //struct tm tm = *localtime(&tim);

                sigset_t set;
                sigfillset(&set);
                sigdelset(&set, SIGINT);
                sigprocmask(SIG_BLOCK, &set, NULL);

                struct sigaction act;
                memset(&act, 0, sizeof(act));
                act.sa_handler = &sig_handler;
                sigaction(SIGINT, &act, NULL);

                if (sem_wait(sema) < 0)
                {
                    perror("Semaphore value decrement error! ");
                    unlink("doubleSafer2");
                    exit(EXIT_SUCCESS);
                }
                int semaValue;
                sem_getvalue(sema, &semaValue);
                int sema2Value;
                sem_getvalue(sema2, &sema2Value);
                int sema3Value;
                sem_getvalue(sema3, &sema3Value);
                //printf("sema value: %d\n",semaValue);
                //int id = shmget(1234, 256 * sizeof(struct request *), IPC_CREAT | 0644);
                //req = shmat(id, NULL, 0);
                //printf("req: %d\n",req->pid);
                snprintf(clientFifo, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE,
                         (long)req->pid);
                char tempString[500];

                // printf("[%d-%02d-%02d %02d:%02d:%02d] Z:Worker PID#%d is handling client PID#%d, matrix size nxn, pool busy %d/%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), req->pid, 3 - sema2Value - 1, poolsize2);
                sprintf(tempString, "Z:Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n", getpid(), req->pid, req->matrixDimension, req->matrixDimension, sema3Value, poolsize2);
                writeToFile(tempString, pathToLogFile);
                int matrix[99][99];
                for (int i = 0, matrixXIndex = 0, matrixYIndex = 0; i < req->matrixDimension * req->matrixDimension; i++)
                {

                    matrix[matrixXIndex][matrixYIndex] = req->matrix[i];
                    matrixYIndex++;
                    if (matrixYIndex == req->matrixDimension)
                    {
                        matrixYIndex = 0;
                        matrixXIndex++;
                    }
                }
                /*
                for(int i=0;i<req->matrixDimension;i++){
                    for(int j = 0;j<req->matrixDimension; j++){
                        printf("%d ", matrix[i][j]);
                    }
                    printf("\n");
                }*/
                sleep(t);
                int det = determinant(matrix, req->matrixDimension);
                //tim = time(NULL);
                //tm = *localtime(&tim);

                if (det == 0)
                {
                    arrayClear(tempString, strlen(tempString));
                    sprintf(tempString, "Z:Worker PID#%d responding to client PID%d: the matrix IS NOT invertible.\n", getpid(), req->pid);
                    writeToFile(tempString, pathToLogFile);
                    sem_post(semaFail);
                    //printf("[%d-%02d-%02d %02d:%02d:%02d] Z:Worker PID#%d responding to client PID%d: the matrix IS NOT invertible.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), req->pid);
                    resp.invertible = 0;
                }
                else
                {
                    sprintf(tempString, "Z:Worker PID#%d responding to client PID%d: the matrix IS invertible.\n", getpid(), req->pid);
                    writeToFile(tempString, pathToLogFile);
                    sem_post(semaSucces);
                    //printf("[%d-%02d-%02d %02d:%02d:%02d] Z:Worker PID#%d responding to client PID%d: the matrix IS invertible.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, getpid(), req->pid);
                    resp.invertible = 1;
                }

                resp.childPID = getpid();
                clientFd = open(clientFifo, O_RDWR, 0777);
                if (clientFd == -1)
                { /* Open failed, give up on client */
                    perror("Client Fifo Openning Error: \n");
                     unlink("doubleSafer2");
                    break;
                }
                if (write(clientFd, &resp, sizeof(struct response)) != sizeof(struct response))
                {
                    perror("Error writing to FIFO %s\n");
                     unlink("doubleSafer2");
                    exit(1);
                }
                if (sem_post(sema2) < 0)
                {
                    perror("Semaphore value decrement error! ");
                     unlink("doubleSafer2");
                }
                if (sem_wait(sema3) < 0)
                {
                    perror("Semaphore value decrement error! ");
                     unlink("doubleSafer2");
                }
                
                unlink(clientFifo);
            }
            if (keep_running == 0)
            {
                 unlink("doubleSafer2");
                raise(SIGINT);
                exit(EXIT_FAILURE);
            }
        default:
            break;
        }
    }

    dummyFd = open(pathForServerZFifo, O_RDWR, 0777);
    if (dummyFd == -1)
    {
         unlink("doubleSafer2");
        perror("Dummyfd openning error: ");
        exit(1);
    }
    while (keep_running == 1)
    {
        sem_wait(sema2);
        if (read(serverFd, req, sizeof(struct request)) != sizeof(struct request))
        {
            perror("Reading from client error!: ");
             unlink("doubleSafer2");
            break; /* Either partial read or error */
        }
        // printf("Request has received from: %d in ServerZ\n", req->pid);
        if (sem_post(sema3) < 0)
        {
            perror("Error occuring while post Semaphore operation.");
             unlink("doubleSafer2");
            break;
        }

        int semaValue;
        sem_getvalue(sema, &semaValue);
        if (sem_post(sema) < 0)
        {
            perror("Error occuring while post Semaphore operation.");
             unlink("doubleSafer2");
            break;
        }
    }

    if (keep_running == 0)
    {
        printf("Signal Catched in Z!\n");
        int succesVal, failVal;
        sem_getvalue(semaSucces, &succesVal);
        sem_getvalue(semaFail, &failVal);
        char tempLastString[150];
        sprintf(tempLastString, "Z:SIGINT Received, exiting server Z. Total requests handled: %d, %d invertible, %d not.\n", succesVal + failVal, succesVal, failVal);
        printf(tempLastString, "Z:SIGINT Received, exiting server Z. Total requests handled: %d, %d invertible, %d not.\n", succesVal + failVal, succesVal, failVal);
        writeToFile(tempLastString, pathToLogFile);     
        for (int i = 0; i < poolsize2; i++)
        {
            kill(pids[i], SIGINT);
        }
        unlink("doubleSafer2");
        unlink(clientFifo);
        raise(SIGINT);
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
         unlink("doubleSafer2");
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
    char *DoubleSafer = "doubleSafer2";
    if(open(DoubleSafer, O_RDWR | O_CREAT | O_EXCL, 0666) == -1) {
        if (errno == EEXIST){
            printf("Double Inst. Safer!\n");
            exit(EXIT_FAILURE);
        }
    }
}