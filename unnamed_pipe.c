// Including all needed libraries

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

// Defining some aesthetic stuff and the maximum SIZE that an array can have, filling it with integers
// that have a dimension of 2 or 4 byte. SIZE is 2000000 since the maximum array dimension is 8 MB, so 2000000
// integers which each occupies at least 2 byte (For the computation we have calculated that every integer occupies at least 4 bytes).

#define BYEL "\e[1;33m"
#define RESET "\e[0m"
#define RED "\e[0;31m"
#define SIZE 2000000

// The CHECK(X) function is usefull to write on shell whenever a system call return an error.
// The function will print the name of the file and the line at which it found the error.
// It will end the check exiting with code 1.

#define CHECK(X) (                                                 \
    {                                                              \
        int __val = (X);                                           \
        (__val == -1 ? (                                           \
                           {                                       \
                               fprintf(stderr, "ERROR ("__FILE__   \
                                               ":%d) -- %s\n",     \
                                       __LINE__, strerror(errno)); \
                               exit(EXIT_FAILURE);                 \
                               -1;                                 \
                           })                                      \
                     : __val);                                     \
    })

// Main in which we have developed the code

int main(int argc, char *argv[])
{

    // Initializing the variables needed for the named pipes that will transfer the time values
    // about the start and finish of the transfer directly to the master process.

    int fd_time0;
    int fd_time1;

    // The C library function clock_t clock(void) returns the number of clock ticks
    // elapsed since the program was launched.

    clock_t seconds0;
    clock_t seconds1;

    // Initialize the array for file descriptors of the named pipe. When you code
    // the pipe() function to create a pipe, you pass a pointer to a two-element integer
    //array where pipe() puts the file descriptors it creates. One descriptor is for the input end of the pipe,
    // and the other is for the output end of the pipe. One process writes data to the input end of the pipe and another process reads from the output.

    int p[2];

    if (pipe(p) < 0)
    {

        exit(1);
    }

    printf(BYEL "Enter the amount of kB you want to transfer (maximum quantity is 100000 kB)" RESET "\n");

    int kBsize;

    scanf("%d", &kBsize);

    // The maximum amount of kB was requested by the professor.

    while (kBsize > 100000)
    {

        printf(RED "ENTER AN AMOUNT OF KB LESS THAN 10000" RESET "\n");

        scanf("%d", &kBsize);
    }

    // We have done a simple conversion. num represents the number of integers that must be transferred.

    int num = kBsize / 0.004;

    // Thanks to the fork() we have created the consumer and the producer without using 2 separate scripts

    int id = CHECK(fork());

    if (id == -1)
    {

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0)
    {

        // The father process represent the producer P. We have created an empty array of integers
        // of maximum size of 2000000

        int A[SIZE];

        // Filling the array with some random numbers

        for (int i = 0; i < SIZE; i++)
        {

            A[i] = 1 + rand() % 100;
        }

        // Taking the time in which the producer start writing the data to the consumer process (child)
        // and sending directly to the master process that will compute the exact transfer time.
        // To convert the number of CPU clock cycles into seconds, we need to use the CLOCKS_PER_SEC constant, which is also defined in the time.h header.
        // We send the time through a named pipe.

        CHECK(fd_time0 = open(argv[1], O_WRONLY));

        // Stores time in seconds

        seconds0 = clock();

        double time_taken0 = (double)seconds0 / CLOCKS_PER_SEC;

        printf("Time 0 : %f\n", time_taken0);

        CHECK(write(fd_time0, &time_taken0, sizeof(time_taken0)));

        // Writing to the consumer through the unnamed pipe type. Since more than 2000000 integers
        // could be transferred, but the array can only handles 2000000 of them, we have devised a system
        // thanks to which we will send the amount of integers we want up to 25000000 (100 MB).

        for (int i = 0; i < num; i++)
        {

            // Once we get to 2000000 integers, the write will restart writing the 0-element of the array and so on
            // till the "num"

            CHECK(write(p[1], &A[i % SIZE], sizeof(A[i % SIZE])));
        }
    }

    else
    {

        // Child process --> consumer

        CHECK(fd_time1 = open(argv[2], O_WRONLY));

        // The child process will read the data transferred and we simply store them into
        // an array (not every data since the array at maximum has a size of 2000000)

        int B[SIZE];

        for (int i = 0; i < num; i++)
        {

            CHECK(read(p[0], &B[i % SIZE], sizeof(B[i % SIZE])));
        }

        // Taking the time in which the consumer finishes reading the data from the producer process (father)
        // and sending directly to the master process that will compute the exact transfer time.

        // Stores time in seconds

        seconds1 = clock();

        double time_taken1 = (double)seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        CHECK(write(fd_time1, &time_taken1, sizeof(time_taken1)));
    }

    // Closing pipes

    CHECK(close(fd_time0));
    CHECK(close(fd_time1));

    CHECK(close(p[0]));
    CHECK(close(p[1]));

    return 0;
}
