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


void *ConsumerThreadFunction(void *arg);
void *QueueBalanceFunction(void *arg);


//run queue for CPUs
struct task_queue{
	int a;
	struct task_struct{
	  	int pid;
	  	int static_prio;	//static priority
		int prio;		//dynamic priority
		int ex_time;		//expected execution time
		int time_slice;		//time slice
		int accu_time_slice;	//accumulated time slice
		int last_cpu;		//the CPU (thread) that the process last ran
		int expected_st;	//expected service time
		int sched_type;		//0 -> SCHED_FIFO, 1 -> SCHED_RR, 2 -> SHED_NORMAL
		
	}q[100];			//large run queue size, so that there is no overflow
};


//for semaphores
union semun {
	int val;
	struct semid_ds *buf;
	unsigned short *array;
};

int Q_sem[4];		//semaphores for the four run queues: sempahore for run queue 1, indicating when there is a 'process' in the queue. Initialized to 0, each run queue begins empty
//int Q2;		//semaphore Q2: sempahore for run queue 2, indicating when there is a 'process' in the queue. Initialized to 0, each run queue begins empty
//int Q3;		//semaphore Q3: sempahore for run queue 3, indicating when there is a 'process' in the queue. Initialized to 0, each run queue begins empty
//int Q4;		//semaphore Q4: sempahore for run queue 4, indicating when there is a 'process' in the queue. Initialized to 0, each run queue begins empty

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


