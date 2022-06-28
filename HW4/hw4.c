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
#define BUFFERSIZE 300000

//Signal Handler.
static volatile sig_atomic_t keep_running = 1;

static void sig_handler(int _)
{
  (void)_;

  keep_running = 0;
}
char *inputFilePath = NULL;
int opt, N, C;
int semid; //System V Semaphore ID.
union semun
{
  int val;              /* used for SETVAL only */
  struct semid_ds *buf; /* used for IPC_STAT and IPC_SET */
  ushort *array;        /* used for GETALL and SETALL */
};
union semun semopts;
// The function to be executed by supplier thread.
void *supplier(void *vargp)
{

  int fromfd;            //File descriptor for supplier.
  struct sembuf sops[1]; //semaphore operation array for supplier.
  char *buffer = malloc(BUFFERSIZE);
  if (keep_running == 0)
  {
    semctl(semid, 0, IPC_RMID);
    free(buffer);
    exit(1);
  }
  fromfd = open(inputFilePath, O_RDONLY);
  if (fromfd == -1)
  {
    perror("Failed to open input file!");
    exit(EXIT_FAILURE);
  }

  read(fromfd, buffer, BUFFERSIZE); //File read.
  for (int i = 0; i < strlen(buffer); i++)
  {
    if (buffer[i] == '1')
    {
      int retval1, retval2;
      retval1 = semctl(semid, 0, GETVAL, semopts); //1's counts.
      retval2 = semctl(semid, 1, GETVAL, semopts); //2's counts.
      time_t tim = time(NULL);                     // For timestamp.
      struct tm tm = *localtime(&tim);
      printf("[%d-%02d-%02d %02d:%02d:%02d] Supplier: read from input a '1'. Current amounts: %d x '1' , %d x '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, retval1, retval2);
      sops[0].sem_num = 0; /* Operate on semaphore 0 */
      sops[0].sem_op = 1;  /* Increment value by one */
      sops[0].sem_flg = 0;
      if (semop(semid, sops, 1) == -1) // Do the operation!
      {
        perror("semop1");
        exit(EXIT_FAILURE);
      }
      retval1 = semctl(semid, 0, GETVAL, semopts);
      retval2 = semctl(semid, 1, GETVAL, semopts);
      tim = time(NULL);
      tm = *localtime(&tim);
      printf("[%d-%02d-%02d %02d:%02d:%02d] Supplier: delivered a '1'. Post-delivery amounts: %d x '1' , %d x '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, retval1, retval2);
    }
    if (buffer[i] == '2')
    {
      time_t tim = time(NULL); // For timestamp.
      struct tm tm = *localtime(&tim);
      int retval1, retval2;
      retval1 = semctl(semid, 0, GETVAL, semopts);
      retval2 = semctl(semid, 1, GETVAL, semopts);
      printf("[%d-%02d-%02d %02d:%02d:%02d] Supplier: read from input a '2'. Current amounts: %d x '1' , %d x '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, retval1, retval2);
      sops[0].sem_num = 1; /* Operate on semaphore 1 */
      sops[0].sem_op = 1;  /* Increment value by one */
      sops[0].sem_flg = 0;
      if (semop(semid, sops, 1) == -1) //Do the operation!
      {
        perror("semop2");
        exit(EXIT_FAILURE);
      }
      retval1 = semctl(semid, 0, GETVAL, semopts);
      retval2 = semctl(semid, 1, GETVAL, semopts);
      if (keep_running == 0)
      {
        close(fromfd);
        free(buffer);
        semctl(semid, 0, IPC_RMID);
        exit(1);
      }
      tim = time(NULL);
      tm = *localtime(&tim);
      printf("[%d-%02d-%02d %02d:%02d:%02d] Supplier: delivered a '2'. Post-delivery amounts: %d x '1' , %d x '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, retval1, retval2);
    }
  }
  time_t tim = time(NULL); // For timestamp.
  struct tm tm = *localtime(&tim);
  printf("[%d-%02d-%02d %02d:%02d:%02d] The Supplier has left.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
  close(fromfd); //Gracefully Exit.
  free(buffer);
  pthread_exit(NULL);
}

void *consumer(void *vargp)
{

  struct sembuf sops[2];
  long consumerID = (long)vargp; //Consumer ID.
  if (keep_running == 0)
  {
    semctl(semid, 0, IPC_RMID);
    exit(1);
  }
  int j = 0; // J-value.
  while (j < N)
  {
    int retval1 = semctl(semid, 0, GETVAL, semopts);
    int retval2 = semctl(semid, 1, GETVAL, semopts);
    time_t tim = time(NULL);
    struct tm tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Consumer-%li at iteration %d (waiting). Current amounts: %d '1', %d '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, consumerID, j, retval1, retval2);

    sops[0].sem_num = 0; /* Operate on semaphore 0 */
    sops[0].sem_op = -1; /* Increment value by one */
    sops[0].sem_flg = 0;

    sops[1].sem_num = 1; /* Operate on semaphore 0 */
    sops[1].sem_op = -1; /* Increment value by one */
    sops[1].sem_flg = 0;
    if (keep_running == 0)
    {
      semctl(semid, 0, IPC_RMID);
      exit(1);
    }
    if (semop(semid, sops, 2) == -1) //Atomically consume both material.
    {
      perror("semop2");
      exit(EXIT_FAILURE);
    }

    if (keep_running == 0)
    {
      semctl(semid, 0, IPC_RMID);
      exit(1);
    }
    retval1 = semctl(semid, 0, GETVAL, semopts);
    retval2 = semctl(semid, 1, GETVAL, semopts);
    if (keep_running == 0)
    {
      semctl(semid, 0, IPC_RMID);
      exit(1);
    }
    tim = time(NULL);
    tm = *localtime(&tim);
    printf("[%d-%02d-%02d %02d:%02d:%02d] Consumer-%li at iteration %d (consumed). Post-consumption amounts: %d '1', %d '2'.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, consumerID, j, retval1, retval2);
    j++;
  }
  time_t tim = time(NULL);
  struct tm tm = *localtime(&tim);
  printf("[%d-%02d-%02d %02d:%02d:%02d] Consumer-%li has left.\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, consumerID);
  pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
  setvbuf(stdout, NULL, _IONBF, 0); //buffer disable for stdouts.
  /*
  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGINT);
  sigprocmask(SIG_BLOCK, &set, NULL); //block other signals.
*/

  struct sigaction act;
  memset(&act, 0, sizeof(act));
  act.sa_handler = &sig_handler;
  sigaction(SIGINT, &act, NULL); //signal handler.

  if (keep_running == 0)
  {
    exit(1);
  }

  while ((opt = getopt(argc, argv, "C:N:F:")) != -1) //command line argumands flag putter.
  {
    switch (opt)
    {
    case 'C':
      C = atoi(optarg);
      if (C <= 4)
      {
        printf("Invalid 'C' value!\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'N':
      N = atoi(optarg);
      if (N <= 1)
      {
        printf("Invalid 'N' value!\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'F':
      inputFilePath = optarg;
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

  if (keep_running == 0)
  {
    exit(1);
  }
  pthread_t idSupplier, idConsumers[C]; //threads
  key_t key;
  key = ftok(inputFilePath, N); //key definer using by ftok.
  if (keep_running == 0)
  {
    exit(1);
  }
  if (key == -1)
  {
    perror("ftok");
    exit(EXIT_FAILURE);
  }
  semid = semget(key, 2, IPC_CREAT | 0600); //semaphore set initialize.
  if (semid == -1)
  {
    perror("semget");
    exit(EXIT_FAILURE);
  }

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, 1);
  pthread_create(&idSupplier, &attr, supplier, NULL); //Detached supplier created.
  pthread_attr_destroy(&attr);

  if (keep_running == 0) //signal handler block.
  {
    semctl(semid, 0, IPC_RMID);
    pthread_cancel(idSupplier);
    exit(1);
  }

  for (int i = 0; i < C; i++)
  {
    pthread_create(&idConsumers[i], NULL, consumer, (void *)(intptr_t)i); //consumer threads create.
  }

  if (keep_running == 0)
  {
    semctl(semid, 0, IPC_RMID);
    exit(1);
  }

  for (int i = 0; i < C; i++)
  {
    pthread_join(idConsumers[i], NULL); //wait for consumers end.
  }
  semctl(semid, 0, IPC_RMID);

  pthread_exit(0);
}
