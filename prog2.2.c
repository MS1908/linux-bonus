#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

typedef struct msgbuffer_t {
    long mtype;
    int op;
    int id;
    int pid;
} msgbuffer;

#define MSG_PATH "messagequeue"

int msgid;
int pid[3];
int curid;

int nxt(int i) {
    return (i < 2 ? i + 1 : 0);
}

void receive_msg() {
    msgbuffer message;
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    if (pid[message.id] == -1) {
        pid[message.id] = message.pid;
    }
    printf("Received from process %d, order number %d, type %d\n", message.pid, message.id, message.op);
    if (message.op == 2) {
        int nxt_pid = pid[nxt(message.id)];
        curid = nxt_pid;
        kill(nxt_pid, SIGUSR1);
    } else if (message.id == curid) {
        kill(message.pid, SIGUSR1);
    }
}

int main(int argc, char** argv) {
    key_t key = ftok(MSG_PATH, 65);
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    memset(pid, -1, sizeof(pid));
    receive_msg();
}