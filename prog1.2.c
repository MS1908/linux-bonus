#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct msgbuffer_t {
    long mtype;
    char mtext[100];
} msgbuffer;

#define MQ_PATH "messagequeue"
#define SHM_PATH "sharedmemory"

int msgid, shmid;
int msgq_value, shm_value;

void create_shmem() {
    key_t key = ftok(SHM_PATH, 65);
    if ((shmid = shmget(key, sizeof(int), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
}

void create_msgqueue() {
    key_t key = ftok(MQ_PATH, 65);
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
}

int receive_msg() {
    msgbuffer message;
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    msgq_value = atoi(message.mtext);
    return msgq_value;
}

void send_msg(int value) {
    msgbuffer message;
    message.mtype = 1;
    sprintf(message.mtext, "%d", value);
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}

void access_shmem_and_stuff() {
    pthread_mutex_t pm;
    pthread_mutexattr_t pmattr;
    pthread_mutexattr_init(&pmattr);

    if (pthread_mutexattr_setpshared(&pmattr, PTHREAD_PROCESS_SHARED) != 0) {
        perror("pthread_mutexattr_setpshared");
        exit(1);
    }

    pthread_mutex_init(&pm, &pmattr);

    if (pthread_mutex_lock(&pm) != 0) {
        perror("pthread_mutex_lock");
        exit(1);
    }

    int *data;
    if ((data = (int *) shmat(shmid, (void *) 0, 0)) == (int *) -1) {
        perror("shamt");
        exit(1);
    }
    printf("Data from shared memory = %d\n", *data);

    sleep(5);

    *data += msgq_value;
    printf("Calculation result = %d\n", *data);
    
    if (pthread_mutex_unlock(&pm) != 0) {
        perror("pthread_mutex_unlock");
        exit(1);
    }

    int value = *data;
    send_msg(value);
}

int main() {
    create_msgqueue();
    create_shmem();
    while (1) {        
        int value = receive_msg();
        if (value == 0) break;
        // printf("Value = %d\n", value);
        access_shmem_and_stuff();

    }
    msgctl(msgid, IPC_RMID, NULL);
}
