#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
 
sem_t sem;

void bee(char* data, int num, int n) {
    num++;
    printf("Bee number %d woke up\n", num);
    while (data[3] < 3 && data[4] > 0) {
        sleep(3);
        sem_wait(&sem);
        if (data[2] > 1 && (data[0] * 10 + data[1] + n - data[2] < 30)) {
            printf("Bee number %d goes for the honey\n", num);
            data[2] -= 1;
            sem_post(&sem);
            int y = rand() % 30 + 15;
            sleep(y);
            sem_wait(&sem);
            printf("Bee number %d returns with the honey\n", num);
            data[2] += 1;
            int honey = data[0] * 10 + data[1];
            honey++;
            data[0] = honey / 10;
            data[1] = honey % 10;
            printf("Now %d honey in the hive\n", honey);
        }
        sem_post(&sem);
        sleep(rand() % 20 + 10); 
    }
    printf("The bee number %d is going to have a rest now\n", num);
}

void Vinni(char* data) {
    while (data[3] < 3 && data[4] > 0) {
        sleep(5);
        sem_wait(&sem);
        int honey = data[0] * 10 + data[1];
        if (honey < 15) {
            printf("Not enough honey, Vinni is still sleeping\n");
        } else {
            printf("Vinni goes for the honey\n");
            if (data[2] < 3) {
                printf("Only %d bees in the hive, Vinni takes the honey\n", data[2]);
                data[0] = 0;
                data[1] = 0;
            } else {
                printf("%d bees in the hive, they bite Vinni and he runs away\n", data[2]);
                data[3] += 1;
            }
        }
        sem_post(&sem);
        sleep(20);
    }
    sem_post(&sem);
    printf("Vinni decided to leave the hive alone\n");
}

int main(int argc, char **argv) {
    char *name = "semaphore";
    sem_open(name, O_CREAT, 0777, 1);
    sem_post(&sem);

    char *shared_name = "shared_mem";
    char *data; 
    char *count;

    int n, fd;
    n = atoi(argv[1]);

    if ((fd = shm_open(shared_name, O_RDWR | O_CREAT, 0777)) == -1) {
        perror("Can't create shared memory for fd\n");
        return 0;
    }

    if (ftruncate(fd, 5) == -1) {
        perror("Can't set the size for fd\n");
        return 0;
    }

    data = mmap(NULL, 5, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("Error mmaping data\n");
        return 0;
    }

    data[0] = 0;
    data[1] = 0;
    data[2] = n;
    data[3] = 0;
    data[4] = 1;

    for (int i = 0; i < n; ++i) {
        int id1 = fork();
        if (id1 == 0) {
            bee(data, i, n);
            exit(EXIT_SUCCESS);
        }
    } 
    int id2 = fork();
    if (id2 == 0) {
        Vinni(data);
        exit(EXIT_SUCCESS);
    }

    char p[4];
    scanf("%s", p);
    data[4] = 0;

    sem_destroy(&sem);

    shm_unlink(shared_name);

    printf("Wait a little bit, the program will end soon\n");

    return 0;
}  
