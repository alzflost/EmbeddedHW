#include "hw1.h"

// each element of Memory table
typedef struct record {
	int order;
	int key;
	unsigned char value[32];
} Record;

// structure of memory table
typedef struct memory_table {
	int entry;
	Record* r[3];
} Mtable;

// memory table
Mtable mtable;

//SHM_FLAGS* flags;
//SHM_DATA* data;

// current order of generation
int cur_order;

void save_st_table(SHM_FLAGS* flags){
	// no entry in memory table -> can't save anything in storage table
	if (mtable.entry == 0){
		return;
	}

	/*
	int last_file = 0;

	// metadata file has three integer : most_recent_st second_recent_st last_recent_st
	// we only need most recent one to find out number of next storage table
	int fd_meta = fopen("metadata", "r");
	if (fd_meta){
		fscanf(fd_meta, "%d", &last_file);
		last_file++;
	}
	close(fd_meta);
	*/


	char filename[10];
	sprintf(filename, "%d.st", flags->st[0]+1);
	
	// open new file (n+1.st) and write memory table
	int fd_new = fopen(filename, "w");
	if (fd_new < 0){
		printf("file open error!");
	}
	for (int i=0; i<mtable.entry; i++){
		Record* temp = mtable.r[i];
		fprintf(fd_new, "%d %d %s", temp->order, temp->key, temp->value);
	}
	close(fd_new);
	
	flags->st_num++;
	// flags->st saves number of each storage table files
	// shift it, to make it sorted by recently generated order
	if (flags->st[1]) flags->st[2] = flags->st[1];
	if (flags->st[0]) flags->st[1] = flags->st[0];
	flags->st[0]++;
}

void clear_mem_table(void){
	// clear contents of mtable
	mtable.entry = 0;
	for (int i=0; i<3; i++){
		free(mtable.r[i]);
	}
	
	for (int i=0; i<3; i++){
		mtable.r[i] = (Record *)malloc(sizeof(Record));
	}
}

void put(SHM_FLAGS* flags, SHM_DATA* data){
	fprintf(stderr, "put start\n");
	// mtable.r[mtable.entry] = (Record *)malloc(sizeof(Record));
	mtable.r[mtable.entry]->order = cur_order++;
	mtable.r[mtable.entry]->key = data->keynum; // shared memory
	strncpy(mtable.r[mtable.entry]->value, data->value, sizeof(data->value)); // shared memory
	mtable.entry++;
	data->order = cur_order-1;
	fprintf(stderr, "put complete\n");
	flags->response = 1;
}

int get(SHM_FLAGS* flags, SHM_DATA* data){
	// search for memory table first
	int target_key = data->keynum;
	data->order = 0;
	memset(data->value, 0, sizeof(data->value));
	for (int i=mtable.entry - 1; i>0; i--){
		// same key
		if (mtable.r[i]->key == target_key){
			// write order and value in shared memory
			data->order = mtable.r[i]->order;
			strncpy(data->value, mtable.r[i]->order, sizeof(mtable.r[i]->order));
			// found in memory table : no need to search storage table
			return 1;
		}
	}
	Record* tmp[100];
	for (int i=0; i<3; i++){
		if (flags->st[i] == 0) break;

		memset(tmp, NULL, sizeof(tmp));
		int tmp_idx = 0;

		char filename[10];
		sprintf(filename, "%d.st", flags->st[i]);
                int fd_search = fopen(filename, "r");
		if (fd_search){
			int order, key;
			unsigned char value[32];
			while ((fscanf(fd_search, "%d %d %s", &order, &key, value))){
				// we need to search descending order of (generated) order
				// so temporarily save data from storage table
				tmp[tmp_idx] = (Record *)malloc(sizeof(Record));
				tmp[tmp_idx]->order = order;
				tmp[tmp_idx]->key = key;
				strncpy(tmp[tmp_idx]->value, value, sizeof(value));
				tmp_idx++;
			}
		}
		close(fd_search);
		// search descending order of tmp table
		for (int j=tmp_idx-1; j>0; j--){
			if (tmp[j]->key == target_key){
				data->order = tmp[j]->order;
				strncpy(data->value, tmp[j]->value, sizeof(tmp[j]->value));
				flags->response = 1;
				return 1;
			}
		}
	}
	return -1;
	/*
	int fd_meta = fopen("metadata", "r");
	
	if (fd_meta){
                int files[3];
		int idx = 0;
		while ((fscanf(fd_meta, "%d", &files[idx])) > 0){
			idx++;
		}
		
		for (int i=0; i<idx; i++){
			char filename[10];
			sprintf(filename, "%d.st", files[i]);
			int fd_search = fopen(filename, "r");
			if (fd_search < 0){
				int order;
				int key;
				unsigned char value[32];
				while ((fscanf(fd_search, "%d %d %s", &order, &key, value))){
					
				}
			}
		}
        }
	*/
}

void main_process(void){
	SHM_FLAGS* flags = (SHM_FLAGS *)shmat(shm_flags_id, (char*)NULL, 0);
	SHM_DATA* data = (SHM_DATA *)shmat(shm_data_id, (char*)NULL, 0);
	
	// flags = (SHM_FLAGS *)malloc(sizeof(SHM_FLAGS));
	//data = (SHM_DATA *)malloc(sizeof(SHM_DATA));
	// fprintf(stderr, "flags and data malloc");

	cur_order = 1;
	flags->mode = 0;
	for (int i=0; i<3; i++){
		flags->st[i] = 0;
	}
	flags->st_num = 0;
	flags->quit = 0;
	flags->request = 0;
	// fprintf(stderr, "flags init");
	//data->key = 0;
	//data->order = 0;
	// fprintf(stderr, "data init");	

	mtable.entry = 0;
	for (int i=0; i<3; i++){
		mtable.r[i] = (Record *)malloc(sizeof(Record));
	}

	flags->quit = 0;
	while(!flags->quit){
		// semaphore P
		usleep(50000);
		semop(sem_id, &p[0], 1);
		// PUT request received
		//
		fprintf(stderr, "req:%d", flags->request);
		if (flags->mode == 0 && flags->request == 1){
			if (mtable.entry == 3){
				// flush if mtable has 3 elements and need to put new one
				save_st_table(flags);
				clear_mem_table();
			}
			fprintf(stderr, "comehere");
	 		put(flags, data);
			// if three storage table exists, request merge
			if (flags->st_num == 3) {
				flags->request = 3;
			}
			// end of request : flag init
			else {
				flags->request=0;
			}
		}
		// GET request received
		if (flags->mode == 1 && flags->request == 1){
			// get request
			get(flags, data);
			// flag init
			flags->request = 0;
		}
		// semaphore V
		semop(sem_id, &v[0], 1);
		usleep(50000);
	}
	save_st_table(flags);
}

int main(void){
	// create shared memory
	// flags contains needed flags (number of st table, current mode, current request...)
	// data consists of order, key, value for put, get requests
	shm_flags_id = shmget(SHM_KEY_FLAGS, sizeof(SHM_FLAGS), IPC_CREAT|0600);
	shm_data_id = shmget(SHM_KEY_DATA, sizeof(SHM_DATA), IPC_CREAT|0600);
	
	if (shm_flags_id == -1){
		printf("shared memory create error");
	}

	sem_id = semget(SEMA_KEY, 2, IPC_CREAT);
	if (sem_id == -1){
		printf("semaphore create error");
	}

	union semun op;
	op.val = 1;

	for (int i=0; i<2; i++){
		int res = semctl(sem_id, i, SETVAL, op);
		if (res == -1){
			printf("semaphore set error");
			exit(-1);
		}
		p[i].sem_num = i;
		v[i].sem_num = i;
		p[i].sem_flg = SEM_UNDO;
		v[i].sem_flg = SEM_UNDO;
		p[i].sem_op = -1;
		v[i].sem_op = 1;
	}

	pid_t pid1;
	pid_t pid2;
	// fork for io process (child)
	pid1 = fork();
	if (pid1 < 0){
		printf("I/O process fork failed");
		return 1;
	}
	else if (pid1 == 0){
		// io() in io.c file
		fprintf(stderr, "io start");
		io();
	}
	else {
		// fork for merge process (child)
		pid2 = fork();
		if (pid2 < 0){
			fprintf(stderr, "merge process fork failed");
			return 1;
		}
		else if (pid2 == 0){
			// merge() in merge.c file
			fprintf(stderr, "merge start");
			merge();
		}
		else {
			// function in this file
			fprintf(stderr, "main start");
			main_process();
		}
	}

	// wait till io and merge ends
	wait(NULL);
	wait(NULL);

	// if both pid exists, it is main process and non of them had error with fork.
	if (pid1&&pid2){
		// remove shared memory
		shmctl(shm_flags_id, IPC_RMID, NULL);
		shmctl(shm_data_id, IPC_RMID, NULL);
	}
}
