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

#define UGRN "\e[4;32m"
#define BHRED "\e[1;91m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define RESET "\e[0m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define BHWHT "\e[1;97m"

int spawn(const char * program, char ** arg_list) {

	// When a program calls fork, a duplicate process, called the child process, is created.
	// The parent process continues executing the program from the point that fork was called. The child process, too, executes the same program from the same place.
	// The fork function provides different return values to the parent and child processes
	// one process "goes in" to the fork call, and two processes "come out," with different return values.
	// The return value in the parent process is the process ID of the child.
	// The return value in the child process is zero.
	
	pid_t child_pid = fork();

  	if (child_pid != 0){

    	return child_pid;
    }

  	else {

		// The exec functions replace the program running in a process with another program. 
		// When a program calls an exec function, that process immediately ceases executing that program and begins executing a new program from the beginning, 
		// assuming that the exec call doesn't encounter an error. 

		execvp(program, arg_list);
		fprintf (stderr, "An error occurred in execvp\n");
		abort ();
    }
}

// Main in which we developed the principal code of master

int main(){

    // Declaring all necessary PIDs

    pid_t pid_unpipe;
	pid_t pid_npipe; 
	pid_t pid_socket; 
	pid_t pid_circb;
	pid_t pid_master;

	int fd_time0;
	int fd_time1;

	double seconds0;
    double seconds1;
	double tot;

    // Getting the pid's master to "fprintf" it in the log file

	pid_master=getpid();


	mkfifo("/tmp/np", 0666);
	mkfifo("/tmp/time0", 0666);
	mkfifo("/tmp/time1", 0666);

	char *arg_list_np[] = { "./np", "/tmp/np", "/tmp/time0",  "/tmp/time1", NULL };
	char *arg_list_up[] = { "./up", "/tmp/time0",  "/tmp/time1", NULL };
	char *arg_list_sck[] = { "./sck", "/tmp/time0",  "/tmp/time1", NULL };
	char *arg_list_cb[] = { "./cb", "/tmp/time0",  "/tmp/time1", NULL };

	printf("\n");
	printf(BHWHT "These 4 concurrents programs aim to measure the speed efficiency when transfer data between a producer and a consumer using different models. The project was made by Lorenzo Benedetti and Andrea G.P. Manera." RESET "\n");
	printf("\n");

	char c;
	 
    while (c != 'e'){

		printf(UGRN "Select which IPC you want to use in order to transfer data:" RESET "\n");
		printf(BHYEL "Press U for Unnamed Pipe transfer" RESET "\n");
		printf(BHBLU "Press N for Named Pipe transfer" RESET "\n");
		printf(BHMAG "Press S for Socket transfer" RESET "\n");
		printf(BHCYN "Press C for Circular Buffer transfer" RESET "\n");
		printf(BHRED "Press E if you want to EXIT" RESET "\n");

		system ("/bin/stty raw");

		c = getchar();
		printf("\n");
		
		system ("/bin/stty cooked");

        switch(c){

            case 117:
                pid_unpipe = spawn ("./up", arg_list_up);
                fd_time0 = open("/tmp/time0", O_RDONLY);
				read(fd_time0, &seconds0, sizeof(seconds0));
				fd_time1 = open("/tmp/time1", O_RDONLY);
				read(fd_time1, &seconds1, sizeof(seconds1));
				tot = seconds1 - seconds0;
				printf(UYEL "Time taken for the transfer of the kB amount chosen : %f s" RESET "\n", tot);
				printf("\n");
                fflush(stdout);
            break;

            case 110:
                pid_npipe = spawn ("./np", arg_list_np);
				fd_time0 = open("/tmp/time0", O_RDONLY);
				read(fd_time0, &seconds0, sizeof(seconds0));
				fd_time1 = open("/tmp/time1", O_RDONLY);
				read(fd_time1, &seconds1, sizeof(seconds1));
				tot = seconds1 - seconds0;
				printf(UBLU "Time taken for the transfer of the kB amount chosen : %f s" RESET "\n", tot);
				printf("\n");
				fflush(stdout);
            break;

            case 115:
                pid_socket = spawn ("./sck", arg_list_sck);
				fd_time0 = open("/tmp/time0", O_RDONLY);
				read(fd_time0, &seconds0, sizeof(seconds0));
				fd_time1 = open("/tmp/time1", O_RDONLY);
				read(fd_time1, &seconds1, sizeof(seconds1));
				tot = seconds1 - seconds0;
				printf(UMAG "Time taken for the transfer of the kB amount chosen : %f s" RESET "\n", tot);
				printf("\n");
				fflush(stdout);
            break;

            case 99:
                pid_circb = spawn ("./cb", arg_list_cb);
				fd_time0 = open("/tmp/time0", O_RDONLY);
				read(fd_time0, &seconds0, sizeof(seconds0));
				fd_time1 = open("/tmp/time1", O_RDONLY);
				read(fd_time1, &seconds1, sizeof(seconds1));
				tot = seconds1 - seconds0;
				printf(UCYN "Time taken for the transfer of the kB amount chosen : %f s" RESET "\n", tot);
				printf("\n");
				fflush(stdout);
			break;

			case 101:

			break;

			default:
				printf(BHRED "Wrong command, try again!" RESET "\n");
				printf("\n");
		}
	}

	return 0;
}
