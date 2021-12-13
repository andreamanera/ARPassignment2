#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <termios.h>

int main(int argc, char* argv[]){

    int fd_np;
    time_t seconds0;
    time_t seconds1;
    
    mkfifo("/tmp/np", 0666);

    printf("Insert number of elements of the array\n");

    int num;

    scanf("%d", & num);

    if (num > 25000000){

        printf("Insert a smaller number than 25000000");


    }

    int id = fork();

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id == 0){

        printf("Producer!");

        int A[num];

        for(int i = 0; i < num; i++){

            A[i] = 1 + rand()%100;

        }

        fd_np = open("/tmp/np", O_WRONLY);

        // Stores time seconds

        time(&seconds0);
        
        printf("Time 0 : %ld\n", seconds0);

        for(int i = 0; i < num; i++){

            write(fd_np, &A[i], sizeof(A[i]));
        }
    }

    printf("Consumer!");
        
    fd_np = open("/tmp/np", O_RDONLY);

    int B[num];
	
                
    for(int i = 0; i < num; i++){

    	read(fd_np, &B[i], sizeof(B[i]));

    }

    // Stores time seconds

    time(&seconds1);
    
    printf("Time 1 : %ld\n", seconds1);
    close(fd_np);

    time_t tot = seconds1 - seconds0;

    printf("Time of execution : %ld\n", tot);
    
    wait(NULL);

    return 0;
}