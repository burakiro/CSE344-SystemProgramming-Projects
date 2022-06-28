#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>



int main(int argc, char *argv[]){
    for(;;){

    printf("Child Process PID:%d \n",(int)getpid());
    sleep(10);
    //kill(getppid(),SIGUSER1);
    }
    
}