#include <stdio.h>    
#include <sys/types.h>
#include <sys/ipc.h>   
#include <sys/shm.h>
#include <stdlib.h>
#include <string.h> 
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
#include <time.h>
#define BUFFERSIZE 3000

int main(int argc, char *argv[])
{
    sem_unlink("/chef0DesertCount");
    sem_unlink("/chef1DesertCount");
    sem_unlink("/chef2DesertCount");
    sem_unlink("/chef3DesertCount");
    sem_unlink("/chef4DesertCount");
    sem_unlink("/chef5DesertCount");
    sem_unlink("/chef0Run");
    sem_unlink("/chef1Run");
    sem_unlink("/chef2Run");
    sem_unlink("/chef3Run");
    sem_unlink("/chef4Run");
    sem_unlink("/chef5Run");
    sem_unlink("/lock0");
    sem_unlink("/lock1");
    sem_unlink("/lock2");
    sem_unlink("/lock3");
    sem_unlink("/lock4");
    sem_unlink("/lock5");
    sem_unlink("/pusher");
    int shm_id;        /* ID of the shared memory segment.   */
    char *shm_addr;    /* address of shared memory segment.  */
    char *ingredients; 
    struct shmid_ds shm_desc;
    int fromfd;
    char *buffer = malloc(BUFFERSIZE);

    sem_t *lock0;
    sem_t *lock1;
    sem_t *lock2;
    sem_t *lock3;
    sem_t *lock4;
    sem_t *lock5;
    sem_t *wholeSalerTurn;
    sem_t *chef0Run;
    sem_t *chef1Run;
    sem_t *chef2Run;
    sem_t *chef3Run;
    sem_t *chef4Run;
    sem_t *chef5Run;
    sem_t *pusher;

    lock0 = sem_open("/lock0", O_CREAT, 0644, 0);
    lock1 = sem_open("/lock1", O_CREAT, 0644, 0);
    lock2 = sem_open("/lock2", O_CREAT, 0644, 0);
    lock3 = sem_open("/lock3", O_CREAT, 0644, 0);
    lock4 = sem_open("/lock4", O_CREAT, 0644, 0);
    lock5 = sem_open("/lock5", O_CREAT, 0644, 0);
    chef0Run = sem_open("/chef0Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    chef1Run = sem_open("/chef1Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    chef2Run = sem_open("/chef2Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    chef3Run = sem_open("/chef3Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    chef4Run = sem_open("/chef4Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    chef5Run = sem_open("/chef5Run", O_CREAT, S_IRUSR | S_IWUSR, 1);
    pusher = sem_open("/pusher", O_CREAT, S_IRUSR | S_IWUSR, 0);

    /* allocate a shared memory segment with size of 2048 bytes. */
    shm_id = shmget(8790, 2 * sizeof(char), IPC_CREAT | 0600);
    if (shm_id == -1)
    {
        perror("main: shmget: ");
        exit(1);
    }

    /* attach the shared memory segment to our process's address space. */
    shm_addr = shmat(shm_id, NULL, 0);
    if (!shm_addr)
    { /* operation failed. */
        perror("main: shmat: ");
        exit(1);
    }

    int opt;
    char *inputFilePath = NULL;
    char *semaphoreN = NULL;
    while ((opt = getopt(argc, argv, "i:n:")) != -1)
    {
        switch (opt)
        {
        case 'i':
            inputFilePath = optarg;
            break;
        case 'n':
            semaphoreN = optarg;
            break;
        case '?':
            if (optopt == 'i')
            {
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                exit(EXIT_FAILURE);
            }
            if (optopt == 'n')
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
    sem_unlink(semaphoreN);
    wholeSalerTurn = sem_open(semaphoreN, O_CREAT, 0644, 0);
    fromfd = open(inputFilePath, O_RDONLY);
    if (fromfd == -1)
    {
        perror("Failed to open input file!");
        exit(EXIT_FAILURE);
    }

    read(fromfd, buffer, BUFFERSIZE);
    ingredients = (char *)shm_addr;

    pid_t id = fork();
    if (id == -1)
        perror("fork");
    if (id == 0)
    {
        printf("Chef0 (pid %d) is waiting for Walnuts and Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        int chef0DesertCounter = 0;
        while (1)
        {

            if (sem_wait(lock0) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef0RunVal;
            sem_getvalue(chef0Run, &chef0RunVal);
            if (chef0RunVal == 0)
            {
                printf("Chef0 (pid %d) is exiting\n", getpid());
                sem_wait(chef1Run);
                sem_post(lock1);
                exit(chef0DesertCounter);
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'S'))
            {
                printf("Chef0 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef0 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'W'))
            {
                printf("Chef0 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef0 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            printf("Chef0 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef0 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef0DesertCounter++;
            printf("Chef0 (pid %d) is waiting for Walnuts and Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }
    pid_t id2 = fork();
    if (id2 == -1)
        perror("fork");
    if (id2 == 0)
    {
        printf("Chef1 (pid %d) is waiting for Flour and Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        int chef1DesertCounter = 0;
        while (1)
        {

            if (sem_wait(lock1) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef1RunVal;
            sem_getvalue(chef1Run, &chef1RunVal);
            if (chef1RunVal == 0)
            {
                printf("Chef1 (pid %d) is exiting\n", getpid());
                sem_wait(chef2Run);
                sem_post(lock2);
                exit(chef1DesertCounter);
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'W'))
            {
                printf("Chef1 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef1 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'F'))
            {
                printf("Chef1 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef1 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }

            printf("Chef1 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef1 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef1DesertCounter++;
            printf("Chef1 (pid %d) is waiting for Flour and Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }
    pid_t id3 = fork();
    if (id3 == -1)
        perror("fork");
    if (id3 == 0)
    {
        int chef2DesertCounter = 0;
        printf("Chef2 (pid %d) is waiting for Sugar and Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        while (1)
        {

            if (sem_wait(lock2) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef2RunVal;
            sem_getvalue(chef2Run, &chef2RunVal);
            if (chef2RunVal == 0)
            {
                printf("Chef2 (pid %d) is exiting\n", getpid());
                sem_wait(chef3Run);
                sem_post(lock3);
                exit(chef2DesertCounter);
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'F'))
            {
                printf("Chef2 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef2 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'S'))
            {
                printf("Chef2 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef2 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }

            printf("Chef2 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef2 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef2DesertCounter++;
            printf("Chef2 (pid %d) is waiting for Sugar and Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }
    pid_t id4 = fork();
    if (id4 == -1)
        perror("fork");
    if (id4 == 0)
    {
        printf("Chef3 (pid %d) is waiting for Milk and Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        int chef3DesertCounter = 0;
        while (1)
        {

            if (sem_wait(lock3) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef3RunVal;
            sem_getvalue(chef3Run, &chef3RunVal);
            if (chef3RunVal == 0)
            {
                printf("Chef3 (pid %d) is exiting\n", getpid());
                sem_wait(chef4Run);
                sem_post(lock4);
                exit(chef3DesertCounter);
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'F'))
            {
                printf("Chef3 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef3 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'M'))
            {
                printf("Chef3 (pid %d) has taken the Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef3 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            printf("Chef3 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef3 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef3DesertCounter++;
            printf("Chef3 (pid %d) is waiting for Milk and Flour. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }
    pid_t id5 = fork();
    if (id5 == -1)
        perror("fork");
    if (id5 == 0)
    {
        printf("Chef4 (pid %d) is waiting for Milk and Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        int chef4DesertCounter = 0;
        while (1)
        {

            if (sem_wait(lock4) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef4RunVal;
            sem_getvalue(chef4Run, &chef4RunVal);
            if (chef4RunVal == 0)
            {
                printf("Chef4 (pid %d) is exiting\n", getpid());
                sem_wait(chef5Run);
                sem_post(lock5);
                exit(chef4DesertCounter);
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'W'))
            {
                printf("Chef4 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef4 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'M'))
            {
                printf("Chef4 (pid %d) has taken the Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef4 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }

            printf("Chef4 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef4 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef4DesertCounter++;
            printf("Chef4 (pid %d) is waiting for Milk and Walnuts. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }
    pid_t id6 = fork();
    if (id6 == -1)
        perror("fork");
    if (id6 == 0)
    {
        printf("Chef5 (pid %d) is waiting for Sugar and Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
        int chef5DesertCounter = 0;
        while (1)
        {

            if (sem_wait(lock5) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }
            int chef5RunVal;
            sem_getvalue(chef5Run, &chef5RunVal);
            if (chef5RunVal == 0)
            {
                printf("Chef5 (pid %d) is exiting\n", getpid());
                sem_post(wholeSalerTurn);
                exit(chef5DesertCounter);
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'M'))
            {
                printf("Chef5 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef5 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'S'))
            {
                printf("Chef5 (pid %d) has taken the Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[0] = '\0';
                printf("Chef5 (pid %d) has taken the Sugar. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
                ingredients[1] = '\0';
            }
            printf("Chef5 (pid %d) is preparing the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            printf("Chef5 (pid %d) has delivered the dessert. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            chef5DesertCounter++;
            printf("Chef5 (pid %d) is waiting for Sugar and Milk. Ingredient Array Content: [%c %c]\n", getpid(), ingredients[0], ingredients[1]);
            sem_post(wholeSalerTurn);
        }
    }

    pid_t pusherID = fork();
    if (pusherID == 0)
    {
        while (1)
        {
            if (sem_wait(pusher) < 0)
            {
                perror("Semaphore value decrement error! ");
                exit(EXIT_SUCCESS);
            }

            if ((ingredients[0] == 'W' && ingredients[1] == 'S') || (ingredients[0] == 'S' && ingredients[1] == 'W'))
            {
                sem_post(lock0);
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'W') || (ingredients[0] == 'W' && ingredients[1] == 'F'))
            {
                sem_post(lock1);
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'F') || (ingredients[0] == 'F' && ingredients[1] == 'S'))
            {
                sem_post(lock2);
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'F') || (ingredients[0] == 'F' && ingredients[1] == 'M'))
            {
                sem_post(lock3);
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'W') || (ingredients[0] == 'W' && ingredients[1] == 'M'))
            {
                sem_post(lock4);
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'M') || (ingredients[0] == 'M' && ingredients[1] == 'S'))
            {
                sem_post(lock5);
            }
        }
    }

    for (int i = 0, j = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == '\n')
        {
            if (j != 2)
            {
                ingredients[0] = '\0';
                ingredients[1] = '\0';
                j = 0;
                continue;
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'S'))
            {
                printf("The wholesaler (pid %d) delivers Walnuts and Sugar\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'W'))
            {
                printf("The wholesaler (pid %d) delivers Sugar and Walnuts\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'W'))
            {
                printf("The wholesaler (pid %d) delivers Flour and Walnuts\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'F'))
            {
                printf("The wholesaler (pid %d) delivers Walnuts and Flour\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'F'))
            {
                printf("The wholesaler (pid %d) delivers Sugar and Flour\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'S'))
            {
                printf("The wholesaler (pid %d) delivers Flour and Sugar\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'F'))
            {
                printf("The wholesaler (pid %d) delivers Milk and Flour\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'F' && ingredients[1] == 'M'))
            {
                printf("The wholesaler (pid %d) delivers Flour and Milk\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'W'))
            {
                printf("The wholesaler (pid %d) delivers Milk and Walnuts\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'W' && ingredients[1] == 'M'))
            {
                printf("The wholesaler (pid %d) delivers Walnuts and Milk\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'S' && ingredients[1] == 'M'))
            {
                printf("The wholesaler (pid %d) delivers Sugar and Milk\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] == 'M' && ingredients[1] == 'S'))
            {
                printf("The wholesaler (pid %d) delivers Milk and Sugar\n", getpid());
                printf("The wholesaler (pid %d) is waiting for the dessert\n", getpid());
            }
            if ((ingredients[0] != 'S' && ingredients[0] != 'M' && ingredients[0] != 'F' && ingredients[0] != 'W') || (ingredients[1] != 'S' && ingredients[1] != 'M' && ingredients[1] != 'F' && ingredients[1] != 'W'))
            {
                ingredients[0] = '\0';
                ingredients[1] = '\0';
                j = 0;
                continue;
            }
            sem_post(pusher);
            j = 0;
            sem_wait(wholeSalerTurn);
            printf("The wholesaler (pid %d) has obtained the dessert and left\n", getpid());
            continue;
        }
        *(ingredients + j) = buffer[i];
        j++;
    }

    sem_wait(chef0Run);
    sem_post(lock0);
    kill(pusherID, SIGKILL);
    int stat;
    waitpid(id, &stat, 0);
    int return_value0 = WEXITSTATUS(stat);
    waitpid(id2, &stat, 0);
    int return_value2 = WEXITSTATUS(stat);
    waitpid(id3, &stat, 0);
    int return_value3 = WEXITSTATUS(stat);
    waitpid(id4, &stat, 0);
    int return_value4 = WEXITSTATUS(stat);
    waitpid(id5, &stat, 0);
    int return_value5 = WEXITSTATUS(stat);
    waitpid(id6, &stat, 0);
    int return_value6 = WEXITSTATUS(stat);
    int totalDesert = return_value0 + return_value2 + return_value3 + return_value4 + return_value5 + return_value6;
    sem_wait(wholeSalerTurn);
    printf("The wholesaler (pid %d) is done (total desserts: %d) \n", getpid(), totalDesert);

    /* detach the shared memory segment from our process's address space. */
    if (shmdt(shm_addr) == -1)
    {
        perror("main: shmdt: ");
    }

    /* de-allocate the shared memory segment. */
    if (shmctl(shm_id, IPC_RMID, &shm_desc) == -1)
    {
        perror("main: shmctl: ");
    }

    sem_close(lock0);
    sem_close(lock1);
    sem_close(lock2);
    sem_close(lock3);
    sem_close(lock4);
    sem_close(lock5);
    sem_close(chef0Run);
    sem_close(chef1Run);
    sem_close(chef2Run);
    sem_close(chef3Run);
    sem_close(chef4Run);
    sem_close(chef5Run);
    sem_close(pusher);
    sem_close(wholeSalerTurn);
    sem_unlink("/chef0Run");
    sem_unlink("/chef1Run");
    sem_unlink("/chef2Run");
    sem_unlink("/chef3Run");
    sem_unlink("/chef4Run");
    sem_unlink("/chef5Run");
    sem_unlink(semaphoreN);
    sem_unlink("/lock0");
    sem_unlink("/lock1");
    sem_unlink("/lock2");
    sem_unlink("/lock3");
    sem_unlink("/lock4");
    sem_unlink("/lock5");
    sem_unlink("/pusher");
    free(buffer);
    close(fromfd);
    exit(EXIT_SUCCESS);
}