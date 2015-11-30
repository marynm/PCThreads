/* 
*	SYSC 4001 - Assignment 2
*	Maryn Marsland
*	100854916
*	Producer and consumer threads
*/ 

#include "pc.h"


void *ConsumerThreadFunction(void *arg);


int BytesWritten = 0;	//count of the total number of bytes written to the shared memory
int TotalBytes;	//the total number of bytes in the file

char input_file [100];
FILE* in;
size_t file_read_size = 15;
//char input_buffer [process_size];
int NUM_CPUS = 4;

struct task_struct queue1[100];

int main(int argc, const char * argv[]) 
{

char input_buffer [file_read_size];
/*Create threads*/
    int res;

    pthread_t con_thread[NUM_CPUS];
    void *thread_result;
    int CPU_number = 0;

	for(CPU_number = 0; CPU_number<NUM_CPUS; CPU_number++) {
    		res = pthread_create(&(con_thread[CPU_number]), NULL, ConsumerThreadFunction, (void *)&CPU_number);
   	 	if (res != 0) {
        		perror("Consumer thread creation failed");
        		exit(EXIT_FAILURE);
		}
		sleep(1);
	}

	//allow time for consumers to get ready
	sleep(1);

/*Producer thread*/
	/* get input file name */
	strcpy(input_file, "input.txt");	//use defult file input.txt


	/* open file */
	printf("Producer opening file: %s\n", input_file);
	
	in = fopen(input_file, "r");

	if(in == NULL)
	{
		printf("File '%s' does not exist in this location.\n Closing producer.\n", input_file);
		return;
	}


	/* set up/connect to semaphores	*/
	union semun sem_union;
	
	Q1 = semget((key_t) 1234, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q1, 0, SETVAL, sem_union);

	Q2 = semget((key_t) 2134, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q2, 0, SETVAL, sem_union);

	Q3 = semget((key_t) 3124, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q3, 0, SETVAL, sem_union);

	Q4 = semget((key_t) 4123, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q4, 0, SETVAL, sem_union);



	/* prepare variables for counts and connection to shared memory */

	//while(BytesWritten < TotalBytes)	//while not at the end of the input file
	{	int i = 0;
		/* read from file into input buffer */
		fgets(input_buffer, 20, in);


		int item = sscanf(input_buffer, "%d", &res);
		printf("int = %d\n", res);
		item = sscanf(input_buffer, "%d", &res);
		printf("int = %d\n", res);
		//count = fread(input_buffer, 1, file_read_size, in);

					printf("Read bytes from file: %s\n", input_buffer); 
		for(i = 0; i<19; i++)
			printf("ib[%d] = %d\n", i, input_buffer[i]);
		queue1[0].static_prio = 22;
		printf("%d\n", queue1[0].static_prio);
		sem_s(Q1);
		sleep(1);
		//queue1->q[0].static_prio = input_buffer[1];
		//printf("Queue 1 static priority = %d\n", queue1->q[0].static_prio);
		//begin_index = 0;
		//printf("i = %d", i);
}
	fclose(in);
	sleep(3);
}




void *ConsumerThreadFunction(void *arg)
{
	int number = *(int *)arg + 1;
	printf("CPU %d thread started.\n", number);
	char output_file [100];
	FILE* out;

		strcpy(output_file, "output.txt");	//use defult file input.txt
	out = fopen(output_file, "w");
	//open file
	if(out == NULL)
	{
		printf("File '%s' does not exist in this location.\n Closing consumer.\n", output_file);
		return;
	}


	/* set up/connect to semaphores	*/
	union semun sem_union;
	
	Q1 = semget((key_t) 1234, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q1, 0, SETVAL, sem_union);

	Q2 = semget((key_t) 2134, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q2, 0, SETVAL, sem_union);

	Q3 = semget((key_t) 3124, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q3, 0, SETVAL, sem_union);

	Q4 = semget((key_t) 4123, 1, 0666 | IPC_CREAT);
	sem_union.val = 0;
	semctl(Q4, 0, SETVAL, sem_union);

	sem_w(Q1);
	printf("CPU %d read: %d from queue1\n", number, queue1[0].static_prio);
/*
	//get total bytes in file from producer
	printf("	Waiting for producer to connect and send the size of its input file...\n");
	sem_w(N);
	sem_s(S);
	int totalBytesInFile = consumer_buffer->buf[buffer_index].size;
	sem_w(S);
	
	int totalByteCount = 0;
	int count;

	while(totalByteCount < totalBytesInFile)
	{
		buffer_index ++;
		sem_w(N);	//wait on semaphore N: run queue not empty
		//sem_w(S);	//wait on semaphore S: queue not in use

		//"consume" buffer from shared memory
		//printf("Read from buffer[%d]: %s\n", buffer_index, consumer_buffer->buf[buffer_index].data);
		//printf("Bytes read = %d\n", consumer_buffer->buf[buffer_index].size);
		count = fwrite(consumer_buffer->buf[buffer_index].data, 1, consumer_buffer->buf[buffer_index].size, out);
		if(count != consumer_buffer->buf[buffer_index].size)
			printf("Error in writing to file!!");
		totalByteCount += count;

		//sem_s(S);	//signal semaphore S

	}
	//once reached end of file, close output file
	printf("	Total bytes written to file %s = %d.\n", output_file, totalByteCount);
	fclose(out);


*/


}

