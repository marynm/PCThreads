/* 
*	Producer and consumer threads
*/ 

#include "pc.h"


void *ConsumerThreadFunction(void *arg);
void *QueueBalanceFunction(void *arg);


char input_file [100];
FILE* in;
size_t file_read_size = 15;

int NUM_CPUS = 4;
int running = 1;

//count of the number of process in each run queue
int q_count[12] = {0};

//index of the next available spot in each queue buffer
int queue_index[12] = {0};

struct task_queue run_queues[12];	//twelve run queues: 3 levels for each CPU. [0], [1]], [2] -> RQ0, RQ1, RQ2 for CPU1 (etc.)

int main(int argc, const char * argv[]) 
{

remaining_processes = total_processes;
char input_buffer [file_read_size];
/*Create threads*/
    int res;

    pthread_t con_thread[NUM_CPUS];
    pthread_t qb_thread;
    void *thread_result;
    int CPU_number = 0;

/*Start Consumer threads*/
	for(CPU_number = 0; CPU_number<NUM_CPUS; CPU_number++) {
    		res = pthread_create(&(con_thread[CPU_number]), NULL, ConsumerThreadFunction, (void *)&CPU_number);
   	 	if (res != 0) {
        		perror("Consumer thread creation failed");
        		exit(EXIT_FAILURE);
		}
		sleep(1);
	}

/*Start queue balancer thread*/
	res = pthread_create(&(qb_thread), NULL, QueueBalanceFunction, NULL);
	if (res != 0) {
        		perror("Consumer thread creation failed");
        		exit(EXIT_FAILURE);
		}

	//allow time for consumers to get ready
	sleep(1);

/*Producer (main) thread*/

	/* get input file name */
	strcpy(input_file, "input.txt");	//use defult file input.txt


	/* open file */
	printf("Producer opening file: %s\n", input_file);
	
	in = fopen(input_file, "r");

	if(in == NULL)
	{
		printf("File '%s' does not exist in this location.\n Closing producer.\n", input_file);
		return 0;
	}


	/* set up semaphores	*/
	union semun sem_union;
	int i;
	int key = 1234;
	
	for(i=0; i<12; i++)
	{
		Q_sem[i] = semget((key_t) key, 1, 0666 | IPC_CREAT);
		sem_union.val = 0;
		semctl(Q_sem[i], 0, SETVAL, sem_union);
		key++;
	}
	
	key = 4321;
	
	for(i=0; i<12; i++)
	{
		ME_sem[i] = semget((key_t) key, 1, 0666 | IPC_CREAT);
		sem_union.val = 1;
		semctl(ME_sem[i], 0, SETVAL, sem_union);
		key++;
	}

	int current_CPU = 0;
	int current_queue;
	int value_read = 0; 
	int sem_val;
	/* read from file into input buffer */
	for(i = 0; i<total_processes; i++)			//read 20 "processes" from input file
	{
			//read the "process information" from the file and add to one of the queues

			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);

			//scheduling type determines which queue (RQ0, RQ1, or RQ2) for the current CPU the process goes into
			//if scheduling type = SCHED_NORMAL (2) put into RQ1 for current CPU
			if(value_read == 2) {	
				current_queue = current_queue + 1;
				printf("Putting process in RQ1 of CPU %d (queue %d) -> ", current_CPU, current_queue);
			}
			//if scheduling type = SCHED_FIFO or SCHED_RR (0 or 1) put into RQ0 for current CPU			
			else 
				printf("Putting process in RQ0 of CPU %d (queue %d) -> ", current_CPU, current_queue);

			//enter critial section for altering the queue element
			sem_w(ME_sem[current_queue]);
			
			run_queues[current_queue].q[queue_index[current_queue]].sched_type = value_read;

			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			printf("%d pid = %d", queue_index[current_queue], value_read);
			run_queues[current_queue].q[queue_index[current_queue]].pid = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("%d static_prio = %d\n", queue_index[current_queue], value_read);
			run_queues[current_queue].q[queue_index[current_queue]].static_prio = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			printf(" prio = %d\n", value_read);
			run_queues[current_queue].q[queue_index[current_queue]].prio = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("%d time_slice = %d\n", k, value_read);
			run_queues[current_queue].q[queue_index[current_queue]].time_slice = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("%d accu_time_slice = %d\n", k, value_read);
			run_queues[current_queue].q[queue_index[current_queue]].accu_time_slice = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("%d last_cpu = %d\n", k, value_read);
			run_queues[current_queue].q[queue_index[current_queue]].last_cpu = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("expected_st = %d\n", k, value_read);
			run_queues[current_queue].q[queue_index[current_queue]].expected_st = value_read;
			
			fgets(input_buffer, 10, in);
			value_read = atoi(input_buffer);
			//printf("expected_st = %d\n", k, value_read);
			run_queues[current_queue].q[queue_index[current_queue]].sleep_avg = value_read;
			
			fgets(input_buffer, 10, in);	//ignore space between set
			
			//signal semaphore for queue
			sem_s(Q_sem[current_queue]);
			sem_s(ME_sem[current_queue]);
			
			q_count[current_queue]++;
			
			//incrament index in queue
			queue_index[current_queue]++;
			if(queue_index[current_queue]>QUEUE_SIZE)
					queue_index[current_queue]=0;	//roll-over for circular buffer
					
			//move to next CPU in cycle
			current_CPU++;
			if(current_CPU>3) 
					current_CPU = 0;
					
			current_queue = current_CPU*3;
	}

	fclose(in);
	
	printf("Producer done, wating for CPUs to finish.\n");
	for(CPU_number = 0; CPU_number<NUM_CPUS; CPU_number++) {
    		res = pthread_join(con_thread[CPU_number], &thread_result);
   	 	if (res != 0) {
        		perror("CPU thread join falied.");
        		exit(EXIT_FAILURE);
		}
		sleep(1);
	}
	
	running = 0;
	res = pthread_join(qb_thread, &thread_result);
   	 	if (res != 0) {
        		perror("Thread join falied.");
        		exit(EXIT_FAILURE);
		}
	printf("All threads complete.\n");
	exit(EXIT_SUCCESS);
}



void *ConsumerThreadFunction(void *arg)
{
	int CPUnumber = *(int *)arg;
	int Qnumber;			 //RQ0, 1, or 2 for the current CPU
	printf("CPU %d thread started.\n", CPUnumber);
    sleep(6);
    
    struct timeval sleep_start, sleep_end;
    int service_time, ticks, bonus;

	int i = 0;		//selected process
	int process_selected;

	while(!queues_empty())
	{
		Qnumber = CPUnumber *3;
		
		//select queue (RQ0 first, then RQ1, then RQ0)
		if(q_count[Qnumber] < 1)	//if RQ0 empty go to RQ1
		{
			Qnumber++;
			if(q_count[Qnumber] < 1)	//if RQ1 empty go to RQ2
			{
					Qnumber++;
					if(q_count[Qnumber] < 1)	//if all queues empty keep checking for processes to be put in queues (unless all processes are done for this simulation)
								process_selected = 0; 
								
					else
					{
						printf("\nCPU %d: Selected RQ2. (queue %d) ", CPUnumber, Qnumber);
						process_selected = 1;
					}
			}
			else
			{
				printf("\nCPU %d: Selected RQ1. (queue %d) ", CPUnumber, Qnumber);
				process_selected = 1;
			}
		}
		else
		{
			printf("\nCPU %d: Selected RQ0. (queue %d) ", CPUnumber, Qnumber);
			process_selected = 1;
		}

	  if(process_selected)
	  {	
		printf("Currently contains %d processes.\n", q_count[Qnumber]);

		//sem_w(Q_sem[Qnumber]);

		//sem_w(ME_sem[Qnumber]);		//enter critical section for using queue

		//decrese the count of waiting processes in this queue
		q_count[Qnumber]--;

		//select process from queue (select the process with the highest priority)
		i = select_highest_prio_in_queue(Qnumber);

		//print process information
		printf("CPU %d: process PID %d selected. Priority is %d. ", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].prio);

		sem_s(ME_sem[Qnumber]);
		
		//Handle SCHED_FIFO
		if(run_queues[Qnumber].q[i].sched_type == 0)
		{
			printf("Scheduling type SCHED_FIFO. Executing..\n\n");
			
			//"execute" process for complete execution time
			usleep(run_queues[CPUnumber].q[i].expected_st * 1000);
			sleep(1); 	//artificial sleep for output clarity
			
			//FIFO runs to completion, no preemtion, so process is complete after being selected once
			remaining_processes--;
			
			sem_w(ME_sem[Qnumber]);
			printf("CPU %d: process PID %d complete. %d processes remaining in RQ0. %d processes remaining total.\n", CPUnumber, run_queues[Qnumber].q[i].pid, q_count[Qnumber], remaining_processes);
			run_queues[Qnumber].q[i].pid = 0;
			sem_s(ME_sem[Qnumber]);
			
		}
		
		//Handle SCHED_RR
		else if(run_queues[Qnumber].q[i].sched_type == 1)
		{
			printf("Scheduling type SCHED_RR. Executing...\n\n");
			
			//calculate time quantum
			if(run_queues[Qnumber].q[i].static_prio > 120)
				run_queues[Qnumber].q[i].time_slice = (140 - run_queues[Qnumber].q[i].static_prio)*20;
			else
				run_queues[Qnumber].q[i].time_slice = (140 - run_queues[Qnumber].q[i].static_prio)*5;
		
			//printf("CPU %d: Process PID %d time quantum calculated to be %d.\n", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].time_slice);
			
			//"execute" process for time quantum
			usleep(run_queues[CPUnumber].q[i].time_slice * 1000);
			sleep(1); 	//artificial sleep for output clarity
		
			//update process
			run_queues[Qnumber].q[i].accu_time_slice += run_queues[Qnumber].q[i].time_slice;
			run_queues[Qnumber].q[i].last_cpu = CPUnumber;
			
			sem_w(ME_sem[Qnumber]);
			//if process now complete
			if(run_queues[Qnumber].q[i].accu_time_slice >= run_queues[Qnumber].q[i].expected_st)
			{
				remaining_processes--;
				printf("CPU %d: Process PID %d complete. Run for %d out of %d total needed. %d processes remaining in queue\n", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].accu_time_slice, run_queues[Qnumber].q[i].expected_st, q_count[Qnumber]);
				run_queues[Qnumber].q[i].pid = 0;
			}
			//otherwise put it back in the run queue
			else
			{
				q_count[Qnumber]++;
				printf("CPU %d: Process PID %d not yet complete. Run for %d out of %d total needed. Puting back in RQ0 (queue %d). %d processes remaining in queue.\n", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].accu_time_slice, run_queues[Qnumber].q[i].expected_st, Qnumber, q_count[Qnumber]);
				run_queues[Qnumber].q[queue_index[Qnumber]] = run_queues[Qnumber].q[i];
				queue_index[Qnumber]++;
				if(queue_index[Qnumber]>QUEUE_SIZE)
					queue_index[Qnumber]=0;		//roll-over for circular buffer
				sem_s(Q_sem[Qnumber]);
				
			}
			sem_s(ME_sem[Qnumber]);
		}
		
		//Handle SCHED_NORMAL
		else if(run_queues[Qnumber].q[i].sched_type == 2)
		{
			printf("Scheduling type SCHED_NORMAL. Executing...\n");
			
			gettimeofday(&sleep_end, NULL);
			
			if(run_queues[Qnumber].q[i].accu_time_slice != 0)		//can't add to sleep time if this is the first time the process has run
			{
				ticks = ((sleep_end.tv_sec * 1000000 + sleep_end.tv_usec) - (sleep_start.tv_sec * 1000000 + sleep_start.tv_usec))/200;
				
				//add to sleep_avg how many ticks it was blocked (up to MAX_SLEEP_AVG)
				run_queues[Qnumber].q[i].sleep_avg += ticks;
				if(run_queues[Qnumber].q[i].sleep_avg > MAX_SLEEP_AVG)
					run_queues[Qnumber].q[i].sleep_avg = MAX_SLEEP_AVG;

			}	
						
			//calculate time quantum
			if(run_queues[Qnumber].q[i].static_prio > 120)
				run_queues[Qnumber].q[i].time_slice = (140 - run_queues[Qnumber].q[i].static_prio)*20;
			else
				run_queues[Qnumber].q[i].time_slice = (140 - run_queues[Qnumber].q[i].static_prio)*5;
			
			//"execute"
			
			//use random period as emulated service time (in range 10ms to time_slice in um)
			service_time = rand() % (run_queues[Qnumber].q[i].time_slice) + 10;
			usleep(service_time * 1000);
			run_queues[Qnumber].q[i].accu_time_slice += service_time;
			
			//wait for period (3x time slice)
			usleep(3*run_queues[Qnumber].q[i].time_slice);

			sleep(1); 	//artificial sleep for output clarity
			
			sem_w(ME_sem[Qnumber]);
			
			//if process now complete
			if(run_queues[Qnumber].q[i].accu_time_slice >= run_queues[Qnumber].q[i].expected_st)
			{
				remaining_processes--;
				printf("CPU %d: Process PID %d complete. Run for %d out of %d total needed. %d processes remaining in queue\n", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].accu_time_slice, run_queues[Qnumber].q[i].expected_st, q_count[Qnumber]);
				run_queues[Qnumber].q[i].pid = 0;
			}
			//otherwise put it back in the run queue after changing the dynamic priority (and moving to RQ2 if dynamic prio>130
			else
			{
				//adjust dynamic priorit based on formula
				printf("CPU %d: Process PID dynamic priority updated from %d ", CPUnumber, run_queues[Qnumber].q[i].prio);
				run_queues[Qnumber].q[i].prio = max(100, min(run_queues[Qnumber].q[i].prio - run_queues[Qnumber].q[i].sleep_avg + 5, 139));
				printf("to %d.\n", run_queues[Qnumber].q[i].prio);
				gettimeofday(&sleep_start, NULL);
			
				//subtract from to sleep_avg how many ticks it ran for this iteration (to minimum 0)
				ticks = service_time/200;
				run_queues[Qnumber].q[i].sleep_avg -= ticks;
				if(run_queues[Qnumber].q[i].sleep_avg < 0)
					run_queues[Qnumber].q[i].sleep_avg = 0;
				
				printf("CPU %d: Process PID %d not yet complete. Run for %d out of %d total needed. ", CPUnumber, run_queues[Qnumber].q[i].pid, run_queues[Qnumber].q[i].accu_time_slice, run_queues[Qnumber].q[i].expected_st);
				
				if(run_queues[Qnumber].q[i].prio < 130)
				{
					run_queues[Qnumber].q[queue_index[Qnumber]] = run_queues[Qnumber].q[i];
					q_count[Qnumber]++;
					printf("Puting back in RQ1 (queue %d). %d processes remaining in queue.\n", Qnumber, q_count[Qnumber]);
				}
				else
				{
					run_queues[(CPUnumber*3)+2].q[queue_index[(CPUnumber*3)+2]] = run_queues[Qnumber].q[i];
					Qnumber = (CPUnumber*3)+2;
					q_count[Qnumber]++;
					printf("Puting in RQ2 (queue %d). %d processes remaining in this queue.\n", Qnumber, q_count[Qnumber]);
				}
				
				run_queues[Qnumber].q[queue_index[Qnumber]] = run_queues[Qnumber].q[i];
				queue_index[Qnumber]++;
				if(queue_index[Qnumber]>QUEUE_SIZE)
					queue_index[Qnumber]=0;		//roll-over for circular buffer
				sem_s(Q_sem[Qnumber]);
				
			}
			sem_s(ME_sem[Qnumber]);
		
		}
		

      }
      else
		sleep(3);
	}
	
	printf("CPU %d completed execution of run queue. Exiting.\n\n", CPUnumber);
	pthread_exit("Thread complete.");
}


void *QueueBalanceFunction(void *arg)
{
		printf("Queue balancer started.\n");
		int max_index;
		int min_index;
		
	/*Queue balancer checks the number of processes in each of the run queues every 3 seconds and balances them as needed*/
		while(running)
		{
			sleep(1);
			
			//check number of items in each queue and adjust if unbalanced
			
			 //if RQ0 queues not balanced, move one process from the queue with the most to the queue with the least until the queues are balanced
			while(!check_balanced(q_count[0], q_count[3], q_count[6], q_count[9]))
			{
				
				//critical section for queue alteration
				sem_w(Q_sem[max_index]);
				sem_w(ME_sem[max_index]);
				
				printf("RQ0 run queues not balanced (Currently %d, %d, %d, & %d)\n", q_count[0], q_count[3], q_count[6], q_count[9]);
				max_index = find_max(q_count[0], 0, q_count[3], 3, q_count[6], 6, q_count[9], 9);
				min_index = find_min(q_count[0], 0, q_count[3], 3, q_count[6], 6, q_count[9], 9);
				
				queue_index[max_index]--;
				printf("Moving process PID %d from queue %d to queue %d\n", run_queues[max_index].q[queue_index[max_index]].pid, max_index, min_index);
				run_queues[min_index].q[queue_index[min_index]] = run_queues[max_index].q[queue_index[max_index]];
				run_queues[max_index].q[queue_index[max_index]].pid = 0;
				queue_index[min_index]++;
				q_count[min_index]++;
				q_count[max_index]--;
				
				sem_s(ME_sem[max_index]);		//end critical section
				sem_s(Q_sem[min_index]);
				
			}
			printf("RQ0 run queues for all CPUs balanced. (Currently %d, %d, %d, & %d)\n", q_count[0], q_count[3], q_count[6], q_count[9]);
				
			//now repeat for RQ1s
			while(!check_balanced(q_count[1], q_count[4], q_count[7], q_count[10]))
			{
				sem_w(Q_sem[max_index]);
				sem_w(ME_sem[max_index]);
				
				printf("RQ1 run queues not balanced (Currently %d, %d, %d, & %d)\n", q_count[1], q_count[4], q_count[7], q_count[10]);
				max_index = find_max(q_count[1], 1, q_count[4], 4, q_count[7], 7, q_count[10], 10);
				min_index = find_min(q_count[1], 1, q_count[4], 4, q_count[7], 7, q_count[10], 10);
				
				queue_index[max_index]--;
				printf("Moving process PID %d from queue %d to queue %d\n", run_queues[max_index].q[queue_index[max_index]].pid, max_index, min_index);
				run_queues[min_index].q[queue_index[min_index]] = run_queues[max_index].q[queue_index[max_index]];
				run_queues[max_index].q[queue_index[max_index]].pid = 0;
				queue_index[min_index]++;
				q_count[min_index]++;
				q_count[max_index]--;
				
				sem_s(ME_sem[max_index]);
				sem_s(Q_sem[min_index]);
				
			}
			printf("RQ1 run queues for all CPUs balanced. (Currently %d, %d, %d, & %d)\n", q_count[1], q_count[4], q_count[7], q_count[10]);
			
			//And RQ2s
			while(!check_balanced(q_count[2], q_count[5], q_count[8], q_count[11]))
			{
				sem_w(Q_sem[max_index]);
				sem_w(ME_sem[max_index]);
				
				printf("RQ2 run queues not balanced (Currently %d, %d, %d, & %d)\n", q_count[2], q_count[5], q_count[8], q_count[11]);
				max_index = find_max(q_count[2], 2, q_count[5], 5, q_count[8], 8, q_count[11], 11);
				min_index = find_min(q_count[2], 2, q_count[5], 5, q_count[8], 8, q_count[11], 11);
				
				queue_index[max_index]--;
				printf("Moving process PID %d from queue %d to queue %d\n", run_queues[max_index].q[queue_index[max_index]].pid, max_index, min_index);
				run_queues[min_index].q[queue_index[min_index]] = run_queues[max_index].q[queue_index[max_index]];
				run_queues[max_index].q[queue_index[max_index]].pid = 0;
				queue_index[min_index]++;
				q_count[min_index]++;
				q_count[max_index]--;
				
				sem_s(ME_sem[max_index]);
				sem_s(Q_sem[min_index]);
				
			}
			printf("RQ2 run queues for all CPUs balanced. (Currently %d, %d, %d, & %d)\n", q_count[2], q_count[5], q_count[8], q_count[11]);

		}
		printf("Queue balance thread exiting.\n");
		pthread_exit("Thread complete.");
}

//check if values are balanced: balanced if all equal or max and min are less than one number apart
int check_balanced(int a, int b, int c, int d)
{
	//check is all queues are exactly equal
	if(a == b && b == c && c == d)
		return 1;
		
	int max = a;
	
	if(b>max)
		max = b;
	if(c>max)
		max = b;
	if(d>max)
		max = d;
		
	max--;
	
	//check if the largest is no more than one greater than all the other values
	if(max<=a && max<=b && max<=c && max<=d)
		return 1;
		
	else
		return 0;
	
}

int find_max(int value1, int index1, int value2, int index2, int value3, int index3, int value4, int index4)
{
	int max;
	int max_index;
	
		max = value1;
		max_index = index1;
		
		if(value2>max) {
			max = value2;
			max_index = index2;
		}
		
		if(value3>max) {
			max = value3;
			max_index = index3;
		}
		
		if(value4>max) {
			max = value4;
			max_index = index4;
		}
		
		return max_index;
}


int find_min(int value1, int index1, int value2, int index2, int value3, int index3, int value4, int index4)
{
	int min;
	int min_index;
	
		min = value1;
		min_index = index1;
		
		if(value2<min) {
			min = value2;
			min_index = index2;
		}
		
		if(value3<min) {
			min = value3;
			min_index = index3;
		}
		
		if(value4<min) {
			min = value4;
			min_index = index4;
		}
		
		return min_index;
}


int select_highest_prio_in_queue(int queue)
{
	int i;
	int max_prio = 0;
	int max_index;
	for(i=0; i<QUEUE_SIZE; i++)
	{
		if(run_queues[queue].q[i].pid != 0)
		{
			if(run_queues[queue].q[i].prio > max_prio)
			{
				max_prio = run_queues[queue].q[i].prio;
				max_index = i;
			}
		}
		
	}
	
	return max_index;
}

int queues_empty()
{
	int i;
	for(i = 0; i<12; i++)
	{
		if(q_count[i] != 0)
			return 0;
	}
	
	return 1;
	
	
}
