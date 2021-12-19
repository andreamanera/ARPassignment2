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

#define BYEL "\e[1;33m"
#define RESET "\e[0m"
#define RED "\e[0;31m"
#define SIZE 2000000

int main(int argc, char* argv[]){

    int fd_time0;
    int fd_time1;

    clock_t seconds0;
    clock_t seconds1;

    int p[2];
  
    if (pipe(p) < 0){

        exit(1);
    }
    
    printf(BYEL "Enter the amount of kB you want to transfer (maximum quantity is 100000 kB)" RESET "\n");

    int kBsize;

    scanf("%d", & kBsize);

    while (kBsize > 100000){

        printf(RED "ENTER AN AMOUNT OF KB LESS THAN 10000" RESET "\n");

        scanf("%d", & kBsize);
    }

    int num = kBsize / 0.004;

    int id = fork();

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0){

        int A[SIZE];

        for(int i = 0; i < SIZE; i++){

            A[i] = 1 + rand()%100;

        }

        fd_time0 = open(argv[1], O_WRONLY);

        seconds0 = clock();

        double time_taken0 =(double) seconds0 / CLOCKS_PER_SEC;

        printf("Time 0 : %f\n", time_taken0);

        write(fd_time0, &time_taken0, sizeof(time_taken0));

        for(int i = 0; i < num; i++){

            write(p[1], &A[i%SIZE], sizeof(A[i%SIZE]));
        }
    }

    else{

        fd_time1 = open(argv[2], O_WRONLY);

        int B[SIZE];
                    
        for(int i = 0; i < num; i++){

            read(p[0], &B[i%SIZE], sizeof(B[i%SIZE]));
        }

        seconds1 = clock();

        double time_taken1 =(double) seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        write(fd_time1, &time_taken1, sizeof(time_taken1));
    }

    close(fd_time0);
    close(fd_time1);

    close(p[0]);
    close(p[1]);

    return 0;
}
