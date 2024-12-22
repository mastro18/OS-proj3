#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shared_mem.h"
#include <time.h>
#include "logger.h"
#include <unistd.h>
#include <sys/times.h>

int main(int argc, char *argv[]) {
    argc = argc;
    int eating_time = atoi(argv[2]);
    key_t shm_key = atoi(argv[4]);
    int shm_id = shmget(shm_key, sizeof(shared_data), IPC_CREAT | 0666);

    if (shm_id < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }
    
    shared_data *data = (shared_data *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    double t1, t2, t3, ticspersec;
    ticspersec = (double) sysconf(_SC_CLK_TCK);

    //If the waiting buffer is full, wait for a position to be free.
    sem_wait(&data->fcfs.bufferSemaphore);

    sem_wait(&data->mutex);

    //Check if the restaurant is closed. If it is, the visitor leaves.
    if (data->is_closed) {
        sem_post(&data->mutex);

        char log_msg5[100];
        sprintf(log_msg5, "Visitor %d is leaving because the restaurant is closed.", getpid());
        log_event(log_msg5);

        sem_post(&data->fcfs.bufferSemaphore);
        exit(EXIT_SUCCESS);
    }

    t1 = (double) times(NULL);
    
    //Free position found, so put the visitor in the waiting buffer.
    int position = data->fcfs.rear;
    data->fcfs.WaitingBuffer[position] = getpid();    
    data->fcfs.rear++;
    data->fcfs.length++;

    //If the end of the buffer is reached, go to the start.
    if (data->fcfs.rear == MAX_VISITORS) {
        data->fcfs.rear = 0;
    }

    sem_post(&data->mutex);
        
    char log_msg[100];
    sprintf(log_msg, "Visitor %d is waiting for a table in waiting buffer.", getpid());
    log_event(log_msg);

    //Wait for the receptionist to assign a table to the visitor.
    sem_wait(&data->fcfs.positionSemaphore[position]);

    //A table was found so the visitor can leave from the waiting buffer.
    sem_post(&data->fcfs.bufferSemaphore);
    
    t2 = (double) times(NULL);

    //Time spent in queue.
    double on_queue_time = (t2 - t1) / ticspersec; 

    char log_msg1[100];
    sprintf(log_msg1, "Visitor %d waited %lf seconds in the queue and now is waiting to order.", getpid(), on_queue_time);
    log_event(log_msg1);

    int j = 0;
    int i = 0;
    //Visitor found its chair and table.
    for (j = 0; j < 3; j++) {
        for (i = 0; i < 4; i++) {
            if (data->tables[j].chairs[i] == getpid()) {

                char log_msg[100];
                sprintf(log_msg, "Visitor %d Found its table %d, chair %d.", getpid(), j, i);
                log_event(log_msg);
                break;
            }
        }
        if (data->tables[j].chairs[i] == getpid()) {
            break;
        }
    }

    char log_msg11[100];
    sprintf(log_msg11, "Visitor %d is waiting to order.", getpid());
    log_event(log_msg11);
    
    //Visitor found its chair and table and now is waiting to order.
    sem_wait(&data->tables[j].waitOnChair[i]);

    sem_wait(&data->mutex);

    data->shared_stats.visitors_served++;
    
    //Receptionist came to take visitor's order and now is ordering.
    srand(time(NULL) ^ (getpid()<<16));
    int order = rand() % 3;
    if (order == 0) {
        data->shared_stats.water_consumed++;
    } else if (order == 1){
        data->shared_stats.wine_consumed++;
    } else {
        data->shared_stats.water_consumed++;
        data->shared_stats.wine_consumed++;
    }
    int cheese =  rand() % 2;
    if (cheese == 1) {
        data->shared_stats.cheese_consumed++;
    }
    int salad = rand() % 2;
    if (salad == 1) {
        data->shared_stats.salads_consumed++;
    }

    sem_post(&data->mutex);  
    
    srand(time(NULL) ^ (getpid()<<16));
    int random_eating_time;
    if (eating_time > 0) {
        random_eating_time = (0.7 * eating_time) + rand() % (eating_time - (int)(0.7 * eating_time));
    } else {
        random_eating_time = 0;
    }
    
    char log_msg2[100];
    sprintf(log_msg2, "Visitor %d will be eating for %d seconds.", getpid(), random_eating_time);
    log_event(log_msg2);

    //Sleep for a random time to simulate eating and talking.
    sleep(random_eating_time);

    sem_wait(&data->mutex);

    //Set the chair as free.
    data->tables[j].chairs[i] = 0;
    data->tables[j].chairsOccupied--;
    //Check if all the chairs of the table are free and if so set the table as free.
    if (data->tables[j].chairsOccupied == 0) {
        data->tables[j].isOccupied = false;
    }

    data->shared_stats.total_wait_time += on_queue_time;

    t3 = (double) times(NULL);
    
    double stay_time = (t3 - t2) / ticspersec;

    data->shared_stats.total_stay_time += stay_time;

    sem_post(&data->mutex);

    char log_msg3[100];
    sprintf(log_msg3, "Visitor %d stayed for %lf seconds and now is leaving.", getpid(), stay_time);
    log_event(log_msg3);

    if (shmdt(data) < 0) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }
        
    return 0;
}
