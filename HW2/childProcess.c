#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>

extern char **environ;

static volatile sig_atomic_t keep_running = 1;


static void sig_handler(int _)
{
    (void)_;

    keep_running = 0;
}

int main(int argc, char *argv[]){
    double x[10]; double sumOfX; double varX; double medianX; double covarianceXY;
    double y[10]; double sumOfY; double varY; double medianY; double covarianceXZ;
    double z[10]; double sumOfZ; double varZ; double medianZ; double covarianceYZ;
    double covarianceMatrix[3][3];
    char covarianceMatrixChar[100];
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    int fd;
    fd = open(argv[0], O_WRONLY|O_APPEND|O_CREAT,0777);
    if(fd == -1){
        perror("Failed to open input file!");
        return 1;
    }
    //printf("Test inside of Child: %s \n",argv[0]);
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_type = F_WRLCK;
    

    struct sigaction act;
    struct sigaction oldAct;
    memset(&act,0,sizeof(act));
    act.sa_handler = &sig_handler;
    sigaction(SIGINT,&act,&oldAct);


char **ep;
ep = environ;

for(int i=0, xIndex=0,yIndex=0,zIndex=0; i<30; i++){
    if(i%3==0){
        x[xIndex] = (int)environ[i][0];
        xIndex++;
    }
    if(i%3==1){
        y[yIndex] = (int)environ[i][0];
        yIndex++;
    }
    if(i%3==2){
        z[zIndex] = (int)environ[i][0];
        zIndex++;
    }

}


if(keep_running == 0){
    sigaction(SIGINT,&oldAct,NULL);
    raise(SIGINT);
    close(fd);
    exit(EXIT_SUCCESS);
}

/*
for(int i=0; i<10;i++){
    printf("x indexes: %lf\n",x[i]);
}
for(int i=0; i<10;i++){
    printf("y indexes: %lf\n",y[i]);
}
for(int i=0; i<10;i++){
    printf("z indexes: %lf\n",z[i]);
}*/

for(int i=0; i<10;i++){
    sumOfX+=x[i];
    sumOfY+=y[i];
    sumOfZ+=z[i];
}

medianX = sumOfX/10;
medianY = sumOfY/10;
medianZ = sumOfZ/10;

if(keep_running == 0){
    sigaction(SIGINT,&oldAct,NULL);
    raise(SIGINT);
    close(fd);
    exit(EXIT_SUCCESS);
}

//printf("Median X:%lf Y:%lf Z:%lf\n",medianX,medianY,medianZ);


for(int i=0;i<10;i++){
    varX+=(x[i]-medianX)*(x[i]-medianX)/10;
    varY+=(y[i]-medianY)*(y[i]-medianY)/10;
    varZ+=(z[i]-medianZ)*(z[i]-medianZ)/10;
}

//printf("Variant X:%lf Y:%lf Z:%lf\n",varX,varY,varZ);

covarianceMatrix[0][0]=varX;
covarianceMatrix[1][1]=varY;
covarianceMatrix[2][2]=varZ;

for(int i=0;i<10;i++){
    covarianceXY+=(x[i]-medianX)*(y[i]-medianY)/10;
    covarianceXZ+=(x[i]-medianX)*(z[i]-medianZ)/10;
    covarianceYZ+=(y[i]-medianY)*(z[i]-medianZ)/10;
}
int mainArray[10][3];
mainArray[0][0]=x[0];
mainArray[1][0]=x[1];
mainArray[2][0]=x[2];
mainArray[3][0]=x[3];
mainArray[4][0]=x[4];
mainArray[5][0]=x[5];
mainArray[6][0]=x[6];
mainArray[7][0]=x[7];
mainArray[8][0]=x[8];
mainArray[9][0]=x[9];

mainArray[0][1]=y[0];
mainArray[1][1]=y[1];
mainArray[2][1]=y[2];
mainArray[3][1]=y[3];
mainArray[4][1]=y[4];
mainArray[5][1]=y[5];
mainArray[6][1]=y[6];
mainArray[7][1]=y[7];
mainArray[8][1]=y[8];
mainArray[9][1]=y[9];

mainArray[0][2]=z[0];
mainArray[1][2]=z[1];
mainArray[2][2]=z[2];
mainArray[3][2]=z[3];
mainArray[4][2]=z[4];
mainArray[5][2]=z[5];
mainArray[6][2]=z[6];
mainArray[7][2]=z[7];
mainArray[8][2]=z[8];
mainArray[9][2]=z[9];


covarianceMatrix[0][1]=covarianceXY;
covarianceMatrix[0][2]=covarianceXZ;
covarianceMatrix[1][0]=covarianceXY;
covarianceMatrix[1][2]=covarianceYZ;
covarianceMatrix[2][0]=covarianceXZ;
covarianceMatrix[2][1]=covarianceYZ;

/*
for(int i=0;i<3;i++){
    for(int j=0;j<3;j++){
        printf("%lf\t",covarianceMatrix[i][j]);
    }
    printf("\n");
}*/



sprintf(covarianceMatrixChar," %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf %.3lf ",covarianceMatrix[0][0],covarianceMatrix[0][1],covarianceMatrix[0][2],covarianceMatrix[1][0],covarianceMatrix[1][1],covarianceMatrix[1][2],covarianceMatrix[2][0],covarianceMatrix[2][1],covarianceMatrix[2][2]);
//printf("Sumation of X: %lf, Y: %lf Z: %lf",sumOfX,sumOfY,sumOfZ);
//printf("Child Process PID:%d \n",(int)getpid());
printf("Created R_%s with (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d), (%d, %d, %d)\n",*(ep+30),mainArray[0][0],mainArray[0][1],mainArray[0][2],mainArray[1][0],mainArray[1][1],mainArray[1][2],mainArray[2][0],mainArray[2][1],mainArray[2][2],mainArray[3][0],mainArray[3][1],mainArray[3][2],mainArray[4][0],mainArray[4][1],mainArray[4][2],mainArray[5][0],mainArray[5][1],mainArray[5][2],mainArray[6][0],mainArray[6][1],mainArray[6][2],mainArray[7][0],mainArray[7][1],mainArray[7][2],mainArray[8][0],mainArray[8][1],mainArray[8][2],mainArray[9][0],mainArray[9][1],mainArray[9][2]);
//printf("Output file path: %s\n" ,argv[4]);

char tempstr[50];
sprintf(tempstr,"%d\n",(int)getpid());
        fcntl(fd,F_SETLKW,&lock);
        write(fd,environ[30],strlen(environ[30]));
        write(fd,covarianceMatrixChar,strlen(covarianceMatrixChar));
        //printf("I'm writing file Rn PID: %d\n",(int)getpid());
    
    


if(keep_running == 0){
    sigaction(SIGINT,&oldAct,NULL);
    raise(SIGINT);
    close(fd);
    exit(EXIT_SUCCESS);
}


    lock.l_type = F_UNLCK;
    fcntl(fd,F_SETLKW,&lock);
    close(fd);
    exit(EXIT_SUCCESS);

}