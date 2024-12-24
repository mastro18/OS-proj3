#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include "shared_mem.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    argc = argc;
    int order_time = atoi(argv[2]);
    key_t shm_key = atoi(argv[4]);
    int shm_id = shmget(shm_key, sizeof(shared_data), IPC_CREAT | 0666);

    if (shm_id < 0) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    shared_data *data = (shared_data *)shmat(shm_id, NULL, 0);
    if (*(int*)data == -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    log_event("Receptionist started");
    //Variable to keep the table number that is found free.
    int k = -1;
    while(1){

        if(data->fcfs.length == 0){
            log_event("No visitors in the waiting buffer");
            sleep(1);
        } else {
            //Wait until a table get free.
            while(data->tables[0].isOccupied == true && data->tables[1].isOccupied == true && data->tables[2].isOccupied == true){
                log_event("All tables are occupied");
                sleep(1);
            }

            for(int i = 0; i < 4 && data->fcfs.length != 0; i++){
                log_event("Receptionist found a free table for visitors.");

                for(int j = 0; j < 3; j++){
                    if(k != -1){              

                        char log_msg[100];
                        sprintf(log_msg, "Visitor %d is scheduled to sit at table %d and chair %d", data->fcfs.WaitingBuffer[data->fcfs.front], j, data->tables[j].chairsOccupied);
                        log_event(log_msg);

                        data->tables[k].chairs[data->tables[k].chairsOccupied] = data->fcfs.WaitingBuffer[data->fcfs.front];
                        data->tables[k].chairsOccupied++;
                        data->fcfs.WaitingBuffer[data->fcfs.front] = 0;
                        //Remove the visitor from the waiting buffer.
                        sem_post(&data->fcfs.positionSemaphore[data->fcfs.front]);

                        break;
                    }else if (data->tables[j].isOccupied == false){
                        k = j;

                        char log_msg[100];
                        sprintf(log_msg, "Visitor %d is scheduled to sit at table %d and chair 0", data->fcfs.WaitingBuffer[data->fcfs.front], j);
                        log_event(log_msg);  

                        data->tables[j].isOccupied = true;
                        //Put the visitor to the table and its chair.
                        data->tables[j].chairs[0] = data->fcfs.WaitingBuffer[data->fcfs.front];
                        data->tables[j].chairsOccupied++;
                        data->fcfs.WaitingBuffer[data->fcfs.front] = 0;
                        //Remove the visitor from the waiting buffer.
                        sem_post(&data->fcfs.positionSemaphore[data->fcfs.front]);
                        
                        break;
                    }
                }

                sem_wait(&data->mutex);               

                data->fcfs.front++;
                data->fcfs.length--;
                if(data->fcfs.front == MAX_VISITORS){
                    data->fcfs.front = 0;
                }

                sem_post(&data->mutex);
            }

            //Go to the table k an take order from every chair thay is occupied.
            char log_msg[100];
            sprintf(log_msg, "Receptionist is taking orders from table %d", k);
            log_event(log_msg);

            for(int i = 0; i < 4; i++){
                if(data->tables[k].chairs[i] != 0){
                    int random_order_time;
                    if(order_time > 0){
                        random_order_time = 0.5 * order_time + rand() % (int)(order_time - 0.5 * order_time);
                    } else {
                        random_order_time = 0;
                    }

                    char log_msg[100];
                    sprintf(log_msg, "Receptionist is taking order from visitor %d, at table %d and chair %d", data->tables[k].chairs[i],k,i);
                    log_event(log_msg);

                    sleep(random_order_time); 
                    //Order was taken so visitor can start talking and eating.       
                    sem_post(&data->tables[k].waitOnChair[i]);
                }
            }       
        }
        k = -1;
        //Programm will stop when all visitors are served and there are not other visitors
        //waiting in the waiting buffer and the bar is closed.
        if(data->is_closed == true && data->fcfs.length == 0){
            break;
        }
    }

    //Receptionist wait for the last visitors to leave.
    while(data->tables[0].isOccupied == true || data->tables[1].isOccupied == true || data->tables[2].isOccupied == true){
        log_event("Receptionist is waiting for the last visitors to leave.");
        sleep(1);
    }

    //Collect the data.
    data->shared_stats.avg_wait_time = data->shared_stats.total_wait_time / data->shared_stats.visitors_served;
    data->shared_stats.avg_stay_time = data->shared_stats.total_stay_time / data->shared_stats.visitors_served;

    printf("Average wait time: %f\n", data->shared_stats.avg_wait_time);
    printf("Average stay time: %f\n", data->shared_stats.avg_stay_time);
    printf("Wine consumed: %d\n", data->shared_stats.wine_consumed);
    printf("Water consumed: %d\n", data->shared_stats.water_consumed);
    printf("Cheese consumed: %d\n", data->shared_stats.cheese_consumed);
    printf("Salads consumed: %d\n", data->shared_stats.salads_consumed);
    printf("Total visitors served: %d\n", data->shared_stats.visitors_served);

    log_event("Receptionist finished");

    if (shmdt(data) < 0) {
        perror("shmdt failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}