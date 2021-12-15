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
#include <termios.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SIZE 2000000

void error(char *msg){

    perror(msg);
    exit(1);
}

int main(int argc, char* argv[]){

    int fd_time0;
    int fd_time1;

    int sockfd;
    int newsockfd;
    int portno;
    int clilen;

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr; 

    struct hostent *server;

    time_t seconds0;
    time_t seconds1;

    printf("Insert number of elements of the array\n");

    int num;

    scanf("%d", & num);

    if (num > 25000000){

        printf("Insert a number smaller than 25000000");

        scanf("%d", & num);
    }

    int id = fork();

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0){

        printf("Server!\n");

        int A[SIZE];

        for(int i = 0; i < SIZE; i++){

            A[i] = 1 + rand()%100;

        }

        fd_time0 = open(argv[1], O_WRONLY);

        time(&seconds0);

        printf("Time 0 : %ld\n", seconds0);

        write(fd_time0, &seconds0, sizeof(seconds0));

        if(argc < 2){

            fprintf(stderr, "Error, no port provided\n");
            exit(1);
        }

        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if(sockfd < 0){

            error("Error opening socket");
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
            
            error("setsockopt(SO_REUSEADDR) failed");
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));

        portno = 8080;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        int reuse = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0){

            perror("setsockopt(SO_REUSEADDR) failed");
        }

        #ifdef SO_REUSEPORT
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0){

                perror("setsockopt(SO_REUSEPORT) failed");
            }
        #endif

        if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){

            error("Error on binding\n");
        }

        listen(sockfd, 5);

        clilen = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0){

            error("Error on accept");
        }

        for(int i = 0; i < num; i++){

            write(newsockfd, &A[i%SIZE], sizeof(A[i%SIZE]));
        }
    }

    else{

        printf("Client!\n");

        fd_time1 = open(argv[2], O_WRONLY);

        int B[SIZE];

        if (argc < 3) {

            exit(0);
        }

        portno = 8080;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

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

        bzero(B, SIZE);

        for(int i = 0; i < num; i++){

            read(sockfd, &B[i%SIZE], sizeof(B[i%SIZE]));
        }

        time(&seconds1);

        printf("Time 1 : %ld\n", seconds1);

        write(fd_time1, &seconds1, sizeof(seconds1));
    }

    close(fd_time0);
    close(fd_time1);

    close(newsockfd);

    return 0;
}
    






        




