#include <errno.h>
#include <signal.h>
#include <strings.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define CYN "\e[0;36m"
#define RESET "\e[0m"
#define RED "\e[0;31m"
#define ERROR_CHAR (-1)
#define TRUE 1
#define FALSE 0
#define _SVID_SOURCE
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

int main(int argc, char* argv[]){


    int fd_time0;
    int fd_time1;
    int kBsize;
    int i;
    int shmid;
    
    clock_t seconds0;
    clock_t seconds1;

    printf(CYN "Enter the amount of kB you want to transfer (maximum quantity is 100000 kB)" RESET "\n");

    scanf("%d", & kBsize);

    while (kBsize > 100000){

        printf(RED "ENTER AN AMOUNT OF KB LESS THAN 10000" RESET "\n");

        scanf("%d", & kBsize);
    }

    int num = kBsize / 0.004;

    typedef struct{

        char file_path[SIZE];
        int in;
        int out;
        sem_t full;
        sem_t empty;
        sem_t mutex;

    } Buffer;

    Buffer *ptr;

    /* shmid is the id of the shared memory address for our buffer */

    shmid = CHECK(shmget(IPC_PRIVATE, sizeof(ptr->file_path), IPC_CREAT | 0666));

    /* get a pointer to our buffer in shared memory */

    ptr = (Buffer*) shmat(shmid, NULL, 0); //no CHECK perchè da problemi

    /* initialise the buffer */

    ptr->in = 0;
    ptr->out = 0;

    /* initialise our semaphores (2nd param 1 means shared betweeen processes */

    CHECK(sem_init(&ptr->empty, 1, SIZE));
    CHECK(sem_init(&ptr->full, 1, 0));

    // fork

    int id = CHECK(fork());

    if (id == -1){

        printf("Error forking...\n");
        exit(1);
    }

    if (id != 0){

        /* this is the producer process */

        CHECK(fd_time0 = open(argv[1], O_WRONLY));

        seconds0 = clock();

        double time_taken0 =(double) seconds0 / CLOCKS_PER_SEC;

        printf("Time 0 : %f\n", time_taken0);

        CHECK(write(fd_time0, &time_taken0, sizeof(time_taken0)));

        int a = 0;

        while(a < num){

            sem_wait(&ptr->empty);

            ptr->file_path[ptr->in] = 1 + rand()%100;; 
            ptr->in = (ptr->in + 1)%SIZE;

            a++;

            sem_post(&ptr->full);
        }
    }

    else{
        
        /* this is the consumer process */
        
        int b;

        while (b < num){

            sem_wait(&ptr->full);

            int h = ptr->file_path[ptr->out];
            ptr->out = (ptr->out + 1)%SIZE;

            b++;

            sem_post(&ptr->empty);
        }

        CHECK(fd_time1 = open(argv[2], O_WRONLY));

        seconds1 = clock();

        double time_taken1 =(double) seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        CHECK(write(fd_time1, &time_taken1, sizeof(time_taken1)));
          
    }

    CHECK(sem_destroy(&ptr->empty));
    CHECK(sem_destroy(&ptr->full));
    
    /* detach the shared memory and deallocate the memory segment */
    
    shmdt(&ptr); // errore CHECK
    CHECK(shmctl(shmid, IPC_RMID, 0));

    close(fd_time1); // il CHECK da un errore, non capisco
    CHECK(close(fd_time0));
    
    /* finally, close the signature file */

    return 0;
}
