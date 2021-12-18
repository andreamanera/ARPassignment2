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
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>

#define SIZE 2000000

int main(int argc, char* argv[]){


    int fd_time0;
    int fd_time1;
    
    clock_t seconds0;
    clock_t seconds1;

    printf("Insert size of the array (in kB), max size is 100000 kB\n");

    int kBsize;

    scanf("%d", & kBsize);

    while (kBsize > 100000){

        printf("Insert a number smaller than 100");

        scanf("%d", & kBsize);

    }

    int num = kBsize / 0.004;

    sem_t not_full;
    sem_t not_empty;
    pthread_mutex_t mutex;

    /* Initialize not_full semaphore to a count of BUFFER_SIZE */

    sem_init(&not_full, 0, SIZE);

    /* Initialize not_empty semaphore to a count of 0 */

    sem_init(&not_empty, 0, 0);

    

    const char * shm_name = "/AOS";

    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);

    int buf[SIZE];

    int id = fork();

    if(id != 0){

        int i;

        caddr_t ptr;

        if (shm_fd == 1) {
            
            printf("Shared memory segment failed\n");
            exit(1);
        }

        ftruncate(shm_fd, SIZE*sizeof(int));

        ptr = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);

        if (ptr == MAP_FAILED) {
            
            printf("Map failed\n");
            return 1;
        }

        /* Write into the memory segment */

        // implement logic of sempahores and ring buffer

        fd_time0 = open(argv[1], O_WRONLY);

        seconds0 = clock();

        double time_taken0 =(double) seconds0 / CLOCKS_PER_SEC;

        printf("Time 0 : %f\n", time_taken0);

        write(fd_time0, &time_taken0, sizeof(time_taken0));

        int in = 0; //index at which producer will put the next data

        int count = 0;

        while(count < num){

            // produce an item

            sem_wait(&not_full); // wait/sleep when there are no empty slots
            pthread_mutex_lock(&mutex);

            int item = 1 + rand()%100;

            buf[in] = item;

            in = (in+1) % SIZE;

            count++;
            
            pthread_mutex_unlock(&mutex);
            sem_post(&not_empty); // Signal/wake to consumer that buffer has some   data and they can consume now
            
        }

        munmap(ptr, SIZE);
    }

    else{

        int i;

        caddr_t ptr;

        shm_fd = shm_open(shm_name, O_RDONLY, 0666);

        if (shm_fd == 1) {

            printf("Shared memory segment failed\n");
            exit(1);
        }

        ptr = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);

        if (ptr == MAP_FAILED) {

            printf("Map failed\n");
            return 1;
        }


        int Cons[SIZE];
        int item;

        fd_time1 = open(argv[2], O_WRONLY);

        int out = 0; // index from which the consumer will consume next data

        int count = 0;

        while(count < num){

            sem_wait(&not_empty); // wait/sleep when there are no empty slots
            pthread_mutex_lock(&mutex);

            printf("aaa");

            fflush(stdout);

            int item;

            item = buf[out];

            out = (out + 1) % SIZE;

            count++;
            
            pthread_mutex_unlock(&mutex);
            sem_post(&not_full); // Signal/wake to consumer that buffer has some   data and they can consume now
        }


        if (shm_unlink(shm_name) == 1) {

            exit(1);
        }

        seconds1 = clock();

        double time_taken1 =(double) seconds1 / CLOCKS_PER_SEC;

        printf("Time 1 : %f\n", time_taken1);

        write(fd_time1, &time_taken1, sizeof(time_taken1));

    }

    close(fd_time0);
    close(fd_time1);

    sem_destroy(&not_empty);
    sem_destroy(&not_full);

    pthread_mutex_destroy(&mutex);
    
    return 0;
}



