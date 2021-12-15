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

        int A[num];

        for(int i = 0; i < num; i++){

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

        bzero((char *) &serv_addr, sizeof(serv_addr));

        portno = 51717;

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(portno);
        serv_addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){

            error("Error on binding");
        }

        listen(sockfd, 5);

        clilen = sizeof(cli_addr);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0){

            error("Error on accept");
        }

        for(int i = 0; i < num; i++){

            write(newsockfd, &A[i], sizeof(int));
        }
    }

    else{

        printf("Client!\n");

        fd_time1 = open(argv[2], O_WRONLY);

        int B[num];

        if (argc < 3) {

            exit(0);
        }

        portno = 51717;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0){

            error("ERROR opening socket");
        }
        
        server = gethostbyname("localhost");

        if (server == NULL) {

            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

        serv_addr.sin_port = htons(portno);

        if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){
        
            error("ERROR connecting");
        }

        bzero(B, num);

        for(int i = 0; i < num; i++){

            read(sockfd, &B[i], sizeof(int));
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
    






        



