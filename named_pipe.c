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

#define BLU "\e[0;34m"
#define RESET "\e[0m"
#define RED "\e[0;31m"
#define SIZE 2000000

int main(int argc, char* argv[]){

    int fd_np;

    int fd_time0;
    int fd_time1;

    clock_t seconds0;
    clock_t seconds1;

    printf(BLU "Enter the amount of kB you want to transfer (maximum quantity is 100000 kB)" RESET "\n");

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

        //maximum size of array that can occupy memory without segmentation fault

        int A[SIZE];

        for(int i = 0; i < SIZE; i++){

            A[i] = 1 + rand()%100;
        }

        fd_np = open(argv[1], O_WRONLY);

        fd_time0 = open(argv[2], O_WRONLY);

        // Stores time seconds

        seconds0 = clock();

        double time_taken0 =(double) seconds0 / CLOCKS_PER_SEC;

        write(fd_time0, &time_taken0, sizeof(time_taken0));
        
        printf("Time 0 : %f\n", time_taken0);

        for(int i = 0; i < num ; i++){

            write(fd_np, &A[i%SIZE], sizeof(A[i%SIZE]));
        }
    }

    else{
            
        fd_np = open(argv[1], O_RDONLY);

        fd_time1 = open(argv[3], O_WRONLY);

        int B[SIZE];
                    
        for(int i = 0; i < num; i++){

            read(fd_np, &B[i%SIZE], sizeof(B[i%SIZE]));
        }

        // Stores time seconds

        seconds1 = clock();

        double time_taken1 = (double) seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        write(fd_time1, &time_taken1, sizeof(time_taken1));
    }
    
    close(fd_np);
    close(fd_time0);
    close(fd_time1);

    return 0;
}
