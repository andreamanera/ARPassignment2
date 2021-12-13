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

    // Getting the pid's master to "fprintf" it in the log file

	pid_master=getpid();

    // We had put this structure code just to avoid pressing 
	// "Enter" every time we want to increase/decrease x and z of our joist
	// The tcgetattr() function will get the parameters associated with 
    // the terminal referred to by the first argument and store them in 
    // the termios structure referenced by the second argument.

	static struct termios oldt;

	void restore_terminal_settings(void){

		tcsetattr(0, TCSANOW, &oldt);  // Apply saved settings. 
	}

	void disable_waiting_for_enter(void){

		struct termios newt;

		// Make terminal read 1 char at a time.

		tcgetattr(0, &oldt);  
		newt = oldt;  
		newt.c_lflag &= ~(ICANON | ECHO); 
		tcsetattr(0, TCSANOW, &newt);  
		atexit(restore_terminal_settings);  
	}

    disable_waiting_for_enter();

	mkfifo("/tmp/np", 0666);
	char *arg_list_np] = { "./np", "/tmp/np", NULL };

    printf("chose which IPC you want to use\n");
    printf("press U for unnamed pipe\n");
    printf("press N for named pipe\n");
    printf("press S for socket\n");
    printf("press C for circular buffer\n");

    char c = getchar();

    while (c != 'e'){

        switch(c){

            case 117:
                pid_unpipe = spawn ("./up", arg_list_up);
                
                c = getchar();
            break;

            case 110:
                pid_npipe = spawn ("./np", arg_list_np);
                c = getchar();
            break;

            case 115:
                pid_socket = spawn ("./sck", arg_list_sck);
                c = getchar();
            break;

            case 99:
                pid_circb = spawn ("./cb", arg_list_cb);
                c = getchar();
            break;
        }



}
