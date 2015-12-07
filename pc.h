/* 
*	Components
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <math.h>

#define QUEUE_SIZE 10
#define MAX_SLEEP_AVG 10

void *ConsumerThreadFunction(void *arg);
void *QueueBalanceFunction(void *arg);
int check_balanced(int a, int b, int c, int d);
int find_max(int value1, int index1, int value2, int index2, int value3, int index3, int value4, int index4);
int find_min(int value1, int index1, int value2, int index2, int value3, int index3, int value4, int index4);

int total_processes = 20;
int remaining_processes;

//run queue for CPUs
struct task_queue{
	int a;
	struct task_struct{
		int sched_type;		//0 -> SCHED_FIFO, 1 -> SCHED_RR, 2 -> SHED_NORMAL
	  	int pid;	
	  	int static_prio;		//static priority
		int prio;				//dynamic priority (initially, and always for FIFO/RR, prio = static prio)
		int time_slice;			//time slice
		int accu_time_slice;	//accumulated time slice
		int last_cpu;			//the CPU (thread) that the process last ran
		int expected_st;		//expected service time
		int sleep_avg;
		
	}q[QUEUE_SIZE];			//large enough queue size sto avoid overflow for this 20-process scenario
};


//for semaphores
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int Q_sem[12];		//semaphores for the twelve run queues: For synchronization, indicating the number of 'processes' in the queue. Initialized to 0 as each run queue begins empty
int ME_sem[12];		//semaphores for mutual exclusion on each of the queues (primarily to ensure the queue balancer doesn't change a queue while the CPU is changing it)

sem_t CPU_sem[4];

//semaphore wait
int sem_w(int semaphore)
{
	//printf("Waiting on a semaphore\n");
    struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* "wait" operation: decrese semaphore value by 1 */
    sem_b.sem_flg = 0;
    if (semop(semaphore, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);	
}

//semaphore signal
int sem_s(int semaphore)
{
	//printf("Signalling a semaphore\n");
struct sembuf sem_b;
    
    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* "signal" operation: increases semaphore value by 1 */
    sem_b.sem_flg = 0;
    if (semop(semaphore, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}

int max(int a, int b)
{
	
	if(a>b)
		return a;
	else
		return b;
}

int min(int a, int b)
{
	if(a<b)
		return a;
	else
		return b;
}
