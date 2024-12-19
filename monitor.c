#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shared_mem.h"

void display_state(shared_data *data) {

    //print waiting buffer
    printf("Waiting buffer:\n");
    for (int i = 0; i < MAX_VISITORS; i++) {
        if (data->fcfs.WaitingBuffer[i] == 0) {
            printf("  Position %d: Empty\n", i);
        } else {
            printf("  Position %d: Visitor %d\n", i, data->fcfs.WaitingBuffer[i]);
        }
    }

    printf("Current state of the bar:\n");
    for (int i = 0; i < 3; i++) {
        //print table and its availability
        printf("Table %d: ", i);
        if (data->tables[i].isOccupied) {
            printf("Occupied\n");
        } else {
            printf("Available\n");
        }
        for (int j = 0; j < 4; j++) {
            if (data->tables[i].chairs[j] == 0) {
                printf("  Chair %d: Available\n", j);
            } else {
                printf("  Chair %d: Occupied by visitor %d\n", j, data->tables[i].chairs[j]);
            }
        }
    }

    printf("Product consumption:\n");
    printf("  Water consumed: %d\n", data->shared_stats.water_consumed);
    printf("  Wine consumed: %d\n", data->shared_stats.wine_consumed);
    printf("  Cheese consumed: %d\n", data->shared_stats.cheese_consumed);
    printf("  Salads consumed: %d\n", data->shared_stats.salads_consumed);
    printf("  Total visitors served: %d\n", data->shared_stats.visitors_served);
}

int main() {
    key_t shm_key = atoi(SHM_KEY);
    int shm_id = shmget(shm_key, sizeof(shared_data), 0666);
    if (shm_id < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    // Attach the shared memory segment
    shared_data *data = (shared_data *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    // Display the current state of the bar
    display_state(data);

    // Detach from shared memory
    if (shmdt(data) < 0) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}