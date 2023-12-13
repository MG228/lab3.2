#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define DEPOSIT 0
#define WITHDRAW 1

void ParentProcess(int *sharedMemory);
void ChildProcess(int *sharedMemory);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }

    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = DEPOSIT; // Turn: 0 for deposit, 1 for withdraw

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    }

    ParentProcess(ShmPTR);

    wait(&status);
    printf("Parent has detected the completion of its child...\n");
    shmdt((void *)ShmPTR);
    printf("Parent has detached its shared memory...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Parent has removed its shared memory...\n");
    printf("Parent exits...\n");

    return 0;
}

void ParentProcess(int *sharedMemory) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for a random amount of time

        while (sharedMemory[1] != DEPOSIT)
            ; // Wait for the child's turn

        int account = sharedMemory[0];
        if (account <= 100) {
            int depositAmount = rand() % 101;
            if (depositAmount % 2 == 0) {
                sharedMemory[0] += depositAmount;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", depositAmount, sharedMemory[0]);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        sharedMemory[1] = WITHDRAW; // Set turn to withdraw
    }
}

void ChildProcess(int *sharedMemory) {
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6); // Sleep for a random amount of time

        while (sharedMemory[1] != WITHDRAW)
            ; // Wait for the parent's turn

        int account = sharedMemory[0];
        int withdrawAmount = rand() % 51;

        printf("Poor Student needs $%d\n", withdrawAmount);

        if (withdrawAmount <= account) {
            sharedMemory[0] -= withdrawAmount;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", withdrawAmount, sharedMemory[0]);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        sharedMemory[1] = DEPOSIT; // Set turn to deposit
    }
}
