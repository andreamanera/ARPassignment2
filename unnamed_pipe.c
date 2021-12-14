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

    int fd_time0;
    int fd_time1;
    time_t seconds0;
    time_t seconds1;

    int p[2];
  
    if (pipe(p) < 0){
        exit(1);
    }

    printf("Insert number of elements of the array\n");

    int num;

    scanf("%d", & num);

    if (num > 25000000){

        printf("Insert a number smaller than 25000000");

        scanf("%d", & num);

    }

    int id = fork();

    if (id != 0){

        printf("Producer!\n");

        int A[num];
        for(int i = 0; i < num; i++){

            A[i] = 1 + rand()%100;

        }
        fd_time0 = open(argv[1], O_WRONLY);

        time(&seconds0);

        printf("Time 0 : %ld\n", seconds0);

        write(fd_time0, &seconds0, sizeof(seconds0));

        for(int i = 0; i < num; i++){

            write(p[1], &A[i], sizeof(A[i]));
        }
    }

    else{

        printf("Consumer!\n");

        fd_time1 = open(argv[2], O_WRONLY);

        int B[num];
                    
        for(int i = 0; i < num; i++){

            read(p[0], &B[i], sizeof(B[i]));

        }

        time(&seconds1);

        printf("Time 1 : %ld\n", seconds1);

        write(fd_time1, &seconds1, sizeof(seconds1));

    }
    close(fd_time0);
    close(fd_time1);

    close(p[0]);
    close(p[1]);

    return 0;
}