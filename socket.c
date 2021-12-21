// Including all needed libraries

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

// Defining some aesthetic stuff and the maximum SIZE that an array can have, filling it with integers
// that have a dimension of 2 or 4 byte. SIZE is 2000000 since the maximum array dimension is 8 MB, so 2000000
// integers which each occupies at least 2 byte (For the computation we have calculated that each integer occupies at least 4 bytes).

#define MAG "\e[0;35m"
#define RESET "\e[0m"
#define RED "\e[0;31m"
#define SIZE 2000000

void error(char *msg){

    perror(msg);
    exit(1);
}


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

int main(int argc, char* argv[]){

    // Initializing the variables needed for the named pipes that will transfer the time values
    // about the start and finish of the transfer directly to the master process.

    int fd_time0;
    int fd_time1;

    // The C library function clock_t clock(void) returns the number of clock ticks
    // elapsed since the program was launched.

    clock_t seconds0;
    clock_t seconds1;

    printf(MAG "Enter the amount of kB you want to transfer (maximum quantity is 100000 kB)" RESET "\n");

    int kBsize;

    scanf("%d", & kBsize);

    // The maximum amount of kB was requested by the professor.

    while (kBsize> 100000){

        printf(RED "ENTER AN AMOUNT OF KB LESS THAN 10000" RESET "\n");

        scanf("%d", & kBsize);
    }

    // We have done a simple conversion. num represents the number of integers that must be transferred.

    int num = kBsize / 0.004;

    // Thanks to the fork() we have created the consumer and the producer without using 2 separate scripts

    int id = CHECK(fork());

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0){

        int sockfd, newsockfd, portno, clilen;

        struct sockaddr_in serv_addr, cli_addr;

        if (argc < 2) {

            fprintf(stderr,"ERROR, no port provided\n");
            exit(1);
        }

        sockfd = CHECK(socket(AF_INET, SOCK_STREAM, 0));

        if (sockfd < 0){

            error("ERROR opening socket");
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));

        portno = 8080;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);

        int reuse = 1;

        #ifdef SO_REUSEPORT
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0){

                perror("setsockopt(SO_REUSEPORT) failed");
            }
        #endif

        if (CHECK(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){

            error("ERROR on binding");
        }

        CHECK(listen(sockfd,5));

        clilen = sizeof(cli_addr);

        newsockfd = CHECK(accept(sockfd, (struct sockaddr *) &cli_addr, &clilen));

        if (newsockfd < 0){

            error("ERROR on accept");
        }

        int buffer[SIZE];

        // Filling the array with some random numbers

        for (int i = 0; i < SIZE; i++){

            buffer[i] = 1 + rand() % 100;
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

        for(int i = 0; i < num; i++){

            CHECK(write(newsockfd, &buffer[i%SIZE], sizeof(buffer[i%SIZE])));
        }
    }

    else{

        int sockfd, portno, n;

        struct sockaddr_in serv_addr;
        struct hostent *server;

        int rec[SIZE];

        if (argc < 3) {

            fprintf(stderr,"usage %s hostname port\n", argv[0]);
            exit(0);
        }

        portno = 8080;

        sockfd = CHECK(socket(AF_INET, SOCK_STREAM, 0));

        if (sockfd < 0){

            error("ERROR opening socket");
        }

        server = gethostbyname("127.0.0.1");

        if (server == NULL) {

            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }
        
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

        serv_addr.sin_port = htons(portno);

        if (CHECK(connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))) < 0){

            error("ERROR connecting");
        }

        CHECK(fd_time1 = open(argv[2], O_WRONLY));

        for(int i = 0; i < num; i++){
    
            CHECK(read(sockfd, &rec[i%SIZE], sizeof(rec[i%SIZE])));
        }

        // Stores time in seconds

        seconds1 = clock();

        double time_taken1 = (double)seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        CHECK(write(fd_time1, &time_taken1, sizeof(time_taken1)));
    }

    CHECK(close(fd_time0));
    CHECK(close(fd_time1));

    //non so se ci siano dei comandi per chiudere socket

    return 0;
}
