#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define SHM_PATH "sharedmemory"

int main() {
    key_t key = ftok(SHM_PATH, 65);

    int shmid;
    if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }

    int *data;
    if ((data = (int *) shmat(shmid, (void *) 0, 0)) == (int *) -1) {
        perror("shamt");
        exit(1);
    }

    printf("Data in shared memory: %d\n", *data);
}