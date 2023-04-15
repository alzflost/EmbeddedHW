#include "hw1.h"

typedef struct record {
	int order;
	int key;
	unsigned char value[32];
} Record;

typedef struct memory_table {
	int entry;
	struct record r[3];
} Mtable;

Mtable mtable[3];

void save_mem_table(void){
	
}

void main_process(void){
	flags = (SHM_FLAGS *)shmat(shm_flags_id, (SHM_FLAGS*)NULL, 0);
	/*
	mode = (int *)shmat(shm_mode_id, (int*)NULL, 0);
	quit = (int *)shmat(shm_exit_id, (int*)NULL, 0);
	st_num = (int *)shmat(shm_st_num_id, (int*)NULL, 0);
	*/
	while(!quit){
		// PUT request received
		if (flags->mode == 0 && flags->request == 1){
			
			// END request
			flags->request = 0;
		}
		// GET request received
		if (flags->mode == 1 && flags->request == 1){
			
			flags->request = 0;
		}
	}
	save_mem_table();
}

int main(void){
	shm_flags_id = shmget(SHM_KEY_FLAGS, sizeof(SHM_FLAGS), IPC_CREAT|0600);
	/*
	shmid_st_num_id = shmget(SHM_KEY_ST, sizeof(int), IPC_CREAT|0600);
	shmid_mode_id = shmget(SHM_KEY_MODE, sizeof(int), IPC_CREAT|0600);
	shmid_exit_id = shmget(SHM_KEY_EXIT, sizeof(int), IPC_CREAT|0600);
	
	
	if (shmid_mode_id == -1 | shmid_st_num_id == -1 | shmid_exit_id == -1){
		perror("shared memory create  error");
	}
	*/
	if (shm_flags_id == -1){
		printf("shared memory create error");
	}

	sem_id = semget(SEMA_KEY, 2, IPC_CREAT);
	if (sem_id == -1){
		printf("semaphore create error");
	}

	pid_t pid1;
	pid_t pid2;
	pid1 = fork();
	if (pid1 < 0){
		printf("I/O process fork failed");
		return 1;
	}
	else if (pid1 == 0){
		io();
	}
	else {
		pid2 = fork();
		if (pid2 < 0){
			printf("merge process fork failed");
			return 1;
		}
		else if (pid2 == 0){
			merge();
		}
		else {
			main_process();
		}
	}
	wait(NULL);
	wait(NULL);

	if (pid1&&pid2){
		shmctl(shm_flags_id, IPC_RMID, NULL);
		/*
		shmctl(shm_data_id, IPC_RMID, NULL);
		shmctl(shm_request_id, IPC_RMID, NULL);
		shmctl(shm_mode_id, IPC_RMID, NULL);
		*/
	}
}
