#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>

typedef struct msgbuffer_t {
    long mtype;
    char mtext[100];
} msgbuffer;

int fd[2];

int get_input() {
    int tmp;
    // printf("Enter input: ");
    scanf("%d", &tmp);    
    char input[10];
    sprintf(input, "%d", tmp);
    size_t length = strlen(input);
    write(fd[1], input, length);
    return tmp;
}

int read_from_pipe() {
    char buffer[10];
    ssize_t count = read(fd[0], buffer, sizeof(buffer));
    if (count <= 0) {
        perror("read");
        exit(1);
    }
    buffer[count] = '\0';
    int ret = atoi(buffer);
    // printf("Read from pipe = %d\n", ret);
    return ret;
}

#define MQ_PATH "messagequeue"

int create_msgqueue() {
    key_t key = ftok(MQ_PATH, 65);
    int msgid;
    if ((msgid = msgget(key, 0666 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    return msgid;
}

void send_to_msgqueue(int msgid, int value) {
    msgbuffer message;
    message.mtype = 1;
    sprintf(message.mtext, "%d", value);
    if (msgsnd(msgid, &message, sizeof(message), 0) == -1) {
        perror("msgsnd");
        exit(1);
    }
    // printf("Message sent\n");
}

int receive_msg(int msgid) {
    msgbuffer message;
    if (msgrcv(msgid, &message, sizeof(message), 1, 0) == -1) {
        perror("msgrcv");
        exit(1);
    }
    int msgq_value = atoi(message.mtext);
    return msgq_value;
}

int main(int argc, char** argv) {
    if (pipe(fd) < 0) {
        perror("pipe");
        exit(1);
    }

    int value;
    int msgid = create_msgqueue();
    
    int child_pid = -1, parent_pid = -1;
    switch (fork()) {
    case -1:
        perror("fork");
        exit(1);
    case 0:
        child_pid = getpid();
        // printf("Child process id = %d\n", child_pid);
        break;
    default:
        parent_pid = getpid();
        // printf("Parent process id = %d\n", parent_pid);
        break;
    }
    
    while (1) {
        int pid = getpid();
        if (pid == parent_pid) {
            int val = get_input();
            if (val == 0) break;
        } else if (pid == child_pid) {
            value = read_from_pipe();
            send_to_msgqueue(msgid, value);
            int result = receive_msg(msgid);
            printf("%d\n", result);
        } else {
            printf("Something wrong happened.\n");
            exit(1);
        }
    }
    
    return 0;
}