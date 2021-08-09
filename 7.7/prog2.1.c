#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

int semid, semval;
int shmid, msgid;
struct sembuf buf;

#define SEM_PATH "semaphore"
#define MSG_PATH "messagequeue"
#define SHM_PATH "sharedmemory"

typedef struct msgbuffer_t {
    long mtype;
    int op;
    int id;
    int pid;
} msgbuffer;

typedef struct data_t {
    int x;
    int y;
} data;

void send_msg(int op, int id, int pid) {
    msgbuffer message;
    message.mtype = 1;
    message.op = op;
    message.id = id;
    message.pid = pid;
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
}

int rnd(int L, int R) {
    return rand() % (R - L + 1) + L;
}

void handler_signal_1(int sig) {
    if (sig == SIGUSR1) {
        printf("Process 1 handler\n");
        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        data *value;
        if ((value = (data *) shmat(shmid, (void *) 0, 0)) == (data *) -1) {
            perror("shmat");
            exit(1);
        }
        value -> x = 10;
        value -> y = 0;

        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        int pid = getpid();
        send_msg(2, 0, pid);
    }
}

void f1() {
    printf("Process 1 sending message.\n");
    signal(SIGUSR1, handler_signal_1);
    int sec = rnd(1, 5);
    sleep(sec);
    int pid = getpid();
    send_msg(1, 0, pid);
}

void handler_signal_2(int sig) {
    if (sig == SIGUSR1) {
        printf("Process 2 handler\n");
        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        data *value;
        if ((value = (data *) shmat(shmid, (void *) 0, 0)) == (data *) -1) {
            perror("shmat");
            exit(1);
        }
        double res = 100.0 / (value -> x);
        printf("100 / X = %0.4ld\n", res);
        value -> x = 0;
        value -> y = 10;

        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        int pid = getpid();
        send_msg(2, 1, pid);
    }
}

void f2() {
    printf("Process 2 sending message.\n");
    signal(SIGUSR1, handler_signal_2);
    int sec = rnd(1, 5);
    sleep(sec);
    int pid = getpid();
    send_msg(1, 1, pid);
}

void handler_signal_3(int sig) {
    if (sig == SIGUSR1) {
        printf("Process 3 handler\n");
        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        data *value;
        if ((value = (data *) shmat(shmid, (void *) 0, 0)) == (data *) -1) {
            perror("shmat");
            exit(1);
        }
        double res = 10000.0 / (value -> y);
        printf("10000 / Y = %0.4ld\n", res);
        value -> x = 0;
        value -> y = 0;

        if (semop(semid, &buf, 1) == -1) {
            perror("semop");
            exit(1);
        }

        int pid = getpid();
        send_msg(2, 2, pid);
    }
}

void f3() {
    printf("Process 3 sending message.\n");
    signal(SIGUSR1, handler_signal_3);
    int sec = rnd(1, 5);
    sleep(sec);
    int pid = getpid();
    send_msg(1, 2, pid);
}

int pid[3];

int main(int argc, char** argv) {
    srand(time(NULL));

    key_t key = ftok(SHM_PATH, 65);
    if ((shmid = shmget(key, sizeof(data), 0666 | IPC_CREAT)) == -1) {
        perror("shmget");
        exit(1);
    }
    key = ftok(MSG_PATH, 65);
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    key = ftok(SEM_PATH, 65);
    if ((semid = semget(key, 1, 0666 | IPC_CREAT)) == -1) {
        perror("semget");
        exit(1);
    }
    buf.sem_num = (unsigned short) 0;
    buf.sem_op = (short) -4;
    buf.sem_flg = (short) 0;

    for (int i = 0; i < 3; i++) {
        if (fork() == 0) {
            pid[i] = getpid();
        }
    }
    int iter = 0;
    while (iter < 10) {
        for (int i = 0; i < 3; i++) {
            if (i == 0) f1();
            if (i == 1) f2();
            if (i == 2) f3();
        }
        sleep(20);
        iter++;
    }
    return 0;
}