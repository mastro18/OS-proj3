#include <unistd.h>
#include <semaphore.h>
#include <stdbool.h>

#define MAX_VISITORS 100    //Maximum number of visitors that can be in the waiting buffer.
#define SHM_KEY "12347"

struct Stats {
    //Time in queue.
    double avg_wait_time;
    double total_wait_time;

    //Time from taking a sit until to leave.
    double avg_stay_time;
    double total_stay_time;
 
    int wine_consumed;
    int water_consumed;
    int cheese_consumed;
    int salads_consumed;

    int visitors_served;
};

struct WaitingBuffer {
    pid_t WaitingBuffer[MAX_VISITORS];
    int front;
    int rear;
    int length;
    sem_t positionSemaphore[MAX_VISITORS];
    sem_t bufferSemaphore;                  //A semaphore to control how many visitors are in the waiting buffer.
};

struct Table {
    bool isOccupied;
    pid_t chairs[4];
    int chairsOccupied;
    sem_t waitOnChair[4];       //Semaphore in which chair the visitor is waiting to order.
};

typedef struct {
    struct Stats shared_stats;
    
    struct WaitingBuffer fcfs;

    struct Table tables[3];

    sem_t mutex;                //mutex for shared memory access.

    bool is_closed;             //Bool variable to close the bar.

} shared_data;