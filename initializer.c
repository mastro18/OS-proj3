#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <semaphore.h>
#include "shared_mem.h"

void spawn_receptionist(int order_time) {
    pid_t pid = fork();
    if (pid == 0) {

        char order_time_str[10];
        snprintf(order_time_str, sizeof(order_time_str), "%d", order_time);

        execl("./receptionist", "./receptionist", "-d", order_time_str, "-s", SHM_KEY, (char *)NULL);

        perror("execl failed for receptionist");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed for receptionist");
        exit(EXIT_FAILURE);
    }
}

void spawn_visitor(int rest_time, int random_sleep_time) {
    pid_t pid = fork();
    if (pid == 0) {
        
        char rest_time_str[10];
        char random_sleep_time_str[10];
        snprintf(rest_time_str, sizeof(rest_time_str), "%d", rest_time);
        snprintf(random_sleep_time_str, sizeof(random_sleep_time_str), "%d", random_sleep_time);

        execl("./visitor", "./visitor", "-d", rest_time_str, "-s", SHM_KEY, random_sleep_time_str, (char *)NULL);

        perror("execl failed for visitor");
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("fork failed for visitor");
        exit(EXIT_FAILURE);
    }
}

int main() {
    int num_visitors = 212;     //Number of visitors that will come to bar.(can be changed)
    int order_time = 4;         //Order and rest time can change if you like.
    int rest_time = 9;

    srand(time(NULL));

    key_t shm_key = atoi(SHM_KEY);
    int shm_id = shmget(shm_key, sizeof(shared_data), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget1 failed");
        exit(EXIT_FAILURE);
    }

    shared_data *data = (shared_data *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }
    
    data->shared_stats.avg_wait_time = 0.0;
    data->shared_stats.total_wait_time = 0.0;
    data->shared_stats.avg_stay_time = 0.0;
    data->shared_stats.total_stay_time = 0.0;
    data->shared_stats.wine_consumed = 0;
    data->shared_stats.water_consumed = 0;
    data->shared_stats.cheese_consumed = 0;
    data->shared_stats.salads_consumed = 0;
    data->shared_stats.visitors_served = 0;

    data->fcfs.front = 0;
    data->fcfs.rear = 0;
    data->fcfs.length = 0;
    for (int i = 0; i < MAX_VISITORS; i++) {
        data->fcfs.WaitingBuffer[i] = 0;
        if (sem_init(&data->fcfs.positionSemaphore[i], 1, 0) != 0) {
            perror("sem_init failed");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < 3; i++) {
        data->tables[i].isOccupied = false;
        for (int j = 0; j < 4; j++) {
            data->tables[i].chairs[j] = 0;
        }
        data->tables[i].chairsOccupied = 0;
    }
    
    for(int j = 0; j < 3; j++){
        for(int i = 0; i < 4; i++){
            if (sem_init(&data->tables[j].waitOnChair[i], 1, 0) != 0) {
                perror("sem_init failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (sem_init(&data->fcfs.bufferSemaphore, 1, MAX_VISITORS) != 0) {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }

    if (sem_init(&data->mutex, 1, 1) != 0) {
        perror("sem_init failed");
        exit(EXIT_FAILURE);
    }

    data->is_closed = false;

    //Spawn receptionist.
    spawn_receptionist(order_time); 

    //Spawn visitors with random arrival time.
    for (int i = 0; i < num_visitors; i++) {

        int random_sleep_time = rand() % 3 + 1;    
        //sleep(random_sleep_time);
        spawn_visitor(rest_time,random_sleep_time);
    }

    for (int i = 0; i < num_visitors + 2; i++) {
        wait(NULL);
    }

    printf("All processes have completed.\n");

    if (shmdt(data) < 0) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) < 0) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}