#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_mem.h"

//closing.c programm is like a bodygard that will not let visitors enter the waiting queue means
//that the restaurant is closed and no more visitors can enter. The visitors inside the bar,
//tables or the waiting queue will be serverd without any trouble.
int main() {
    key_t shm_key = atoi(SHM_KEY);
    int shm_id = shmget(shm_key, sizeof(shared_data), 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_data *data = (shared_data *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    data->is_closed = true;

    if (shmdt(data) < 0) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}