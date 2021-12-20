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

// Main in which we have developed the code

int main(int argc, char* argv[]){

    // Initializing the variables needed for the named pipes that will transfer the time values
    // about the start and finish of the transfer directly to the master process.

    int fd_time0;
    int fd_time1;

    // sockfd and newsockfd are file descriptors, i.e. array subscripts into the file descriptor table . 
    // These two variables store the values returned by the socket system call and the accept system call.

    int sockfd;
    int newsockfd;

    // portno stores the port number on which the server accepts connections.
    // clilen stores the size of the address of the client. This is needed for the accept system call

    int portno;
    int clilen;

    // A sockaddr_in is a structure containing an internet address. This structure is defined in <netinet/in.h> .
    // An in_addr structure, defined in the same header file, contains only one field, a unsigned long called
    // s_addr . The variable serv_addr will contain the address of the server, and cli_addr will contain the
    // address of the client which connects to the server.

    // The variable server is a pointer to a structure of type hostent . This structure is 
    // defined in the header file netdb.h. It defines a host computer on the Internet.

    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr; 
    struct hostent *server;

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

    int id = fork();

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0){

        // The father process represent the producer P. We have created an empty array of integers
        // of maximum size of 2000000

        int A[SIZE];

        // Filling the array with some random numbers

        for(int i = 0; i < SIZE; i++){

            A[i] = 1 + rand()%100;
        }

        // Taking the time in which the producer start writing the data to the consumer process (child)
        // and sending directly to the master process that will compute the exact transfer time.
        // To convert the number of CPU clock cycles into seconds, we need to use the CLOCKS_PER_SEC constant, which is also defined in the time.h header.
        // We send the time through a named pipe.

        fd_time0 = open(argv[1], O_WRONLY);

        // Stores time in seconds

        seconds0 = clock();

        double time_taken0 =(double) seconds0 / CLOCKS_PER_SEC;

        printf("Time 0 : %f\n", time_taken0);

        write(fd_time0, &time_taken0, sizeof(time_taken0));

        if(argc < 2){

            fprintf(stderr, "Error, no port provided\n");
            exit(1);
        }

        // The socket() system call creates a new socket. It takes three arguments. 
        // The first is the address domain of the socket. The symbol constant
        // AF_INET for the Internet domain for any two hosts on the Internet.
        // The second argument is the type of socket. The symbolic constants for the stream socket in which
        // characters are read in a continuous stream as if from a file or pipe is SOCK_STREAM
        // The third argument is the protocol. If this argument is zero (and it always should be except for unusual circumstances), the operating
        // system will choose the most appropriate protocol. It will choose TCP for stream sockets
        // and UDP for datagram sockets. The socket system call returns an entry into the file descriptor table (i.e. a small integer). 
        // This value is used for all subsequent references to this socket. If the socket call fails, it returns -1. In this case the program
        // displays and error message and exits. However, this system call is unlikely to fail.
        
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if(sockfd < 0){

            error("Error opening socket");
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0){
            
            error("setsockopt(SO_REUSEADDR) failed");
        }

        // The function bzero() sets all values in a buffer to zero. It takes two arguments, the first is a pointer to the
        // buffer and the second is the size of the buffer. Thus, this line initializes serv_addr to zeros.

        bzero((char *) &serv_addr, sizeof(serv_addr));

        // The port number on which the server will listen for connections

        portno = 8080;

        // The variable serv_addr is a structure of type struct sockaddr_in . This structure has four fields. The first
        // field is short sin_family , which contains a code for the address family. It should always be set to the
        // symbolic constant AF_INET .

        serv_addr.sin_family = AF_INET;

        // The second field of serv_addr is unsigned short sin_port , which contain the port number. However,
        // instead of simply copying the port number to this field, it is necessary to convert this to network byte order
        // using the function htons() which converts a port number in host byte order to a port number in network byte order.

        serv_addr.sin_port = htons(portno);

        // The third field of sockaddr_in is a structure of type struct in_addr which contains only a single field
        // unsigned long s_addr. This field contains the IP address of the host. For server code, this will always be
        // the IP address of the machine on which the server is running, and there is a symbolic constant INADDR_ANY
        // which gets this address.

        serv_addr.sin_addr.s_addr = INADDR_ANY;

        // SO_REUSEPORT:
        // Permits multiple AF_INET or AF_INET6 sockets to be bound to an identical socket address.  
        // This option must be set on each socket (including the first socket) prior to calling bind(2)
        // on the socket. To prevent port hijacking, all of the processes binding to the same address must have the same
        // effective UID. This option can be employed with both TCP and UDP sockets.

        int reuse = 1;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0){

            perror("setsockopt(SO_REUSEADDR) failed");
        }

        #ifdef SO_REUSEPORT
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (const char*)&reuse, sizeof(reuse)) < 0){

                perror("setsockopt(SO_REUSEPORT) failed");
            }
        #endif

        // The bind() system call binds a socket to an address, in this case the address of the current host and port
        // number on which the server will run. It takes three arguments, the socket file descriptor, the address to which
        // is bound, and the size of the address to which it is bound. The second argument is a pointer to a structure of
        // type sockaddr , but what is passed in is a structure of type sockaddr_in , and so this must be cast to the
        // correct type. This can fail for a number of reasons, the most obvious being that this socket is already in us on this machine.

        if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){

            error("Error on binding\n");
        }

        // The listen system call allows the process to listen on the socket for connections. The first argument is the
        // socket file descriptor, and the second is the size of the backlog queue, i.e., the number of connections that
        // can be waiting while the process is handling a particular connection. This should be set to 5, the maximum
        // size permitted by most systems. If the first argument is a valid socket, this call cannot fail, and so the code
        // doesn't check for errors.

        listen(sockfd, 5);

        clilen = sizeof(cli_addr);

        // The accept() system call causes the process to block until a client connects to the server. Thus, it wakes up
        // the process when a connection from a client has been successfully established. It returns a new file
        // descriptor, and all communication on this connection should be done using the new file descriptor. The
        // second argument is a reference pointer to the address of the client on the other end of the connection, and the
        // third argument is the size of this structure.

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        if (newsockfd < 0){

            error("Error on accept");
        }

        // Once a connection has been established, both ends can both read and write to the connection. Naturally,
        // everything written by the client will be read by the server, and everything written by the server will be read
        // by the client. This code simply writes a short message to the client. The last argument of write is the size of the message.

        // Writing to the consumer through the server-client type. Since more than 2000000 integers
        // could be transferred, but the array can only handles 2000000 of them, we have devised a system
        // thanks to which we will send the amount of integers we want up to 25000000 (100 MB).

        for(int i = 0; i < num; i++){

            write(newsockfd, &A[i%SIZE], sizeof(A[i%SIZE]));
        }
    }

    else{

        // Child process --> consumer

        fd_time1 = open(argv[2], O_WRONLY);

        // The child process will read the data transferred and we simply store them into
        // an array (not every data since the array at maximum has a size of 2000000)

        int B[SIZE];

        if (argc < 3) {

            exit(1);
        }

        portno = 8080;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        if (sockfd < 0){

            error("ERROR opening socket");
        }

        // Takes such a name as an argument and returns a pointer to a hostent containing information about that host.
        // The field char *h_addr contains the IP address. If this structure is NULL, the system could not locate a host
        // with this name.
        
        server = gethostbyname("127.0.0.1");

        if (server == NULL) {

            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }

        // This code sets the fields in serv_addr . Much of it is the same as in the server. However, because the field
        //server->h_addr is a character string, we use the function bcopy.

        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

        serv_addr.sin_port = htons(portno);

        if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){

            error("ERROR connecting");
        }

        // The connect function is called by the client to establish a connection to the server. It takes three arguments,
        // the socket file descriptor, the address of the host to which it wants to connect (including the port number),
        // and the size of this address. This function returns 0 on success and -1 if it fails.
        // Notice that the client needs to know the port number of the server, but it does not need to know its own port
        // number. This is typically assigned by the system when connect is called.

        bzero(B, SIZE);

        for(int i = 0; i < num; i++){

            read(sockfd, &B[i%SIZE], sizeof(B[i%SIZE]));
        }

        // Taking the time in which the consumer finishes reading the data from the producer process (father)
        // and sending directly to the master process that will compute the exact transfer time.

        // Stores time seconds

        seconds1 = clock();

        double time_taken1 =(double) seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        write(fd_time1, &time_taken1, sizeof(time_taken1));
    }

    // closing pipes 

    close(fd_time0);
    close(fd_time1);

    // close()

    close(newsockfd);

    return 0;
}
    






        




