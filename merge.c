#include "hw1.h"
#include<dirent.h>

typedef struct storage {
	int order;
	int key;
	unsigned char value[32];
} ST_TABLE;

ST_TABLE* st1_table[50];
ST_TABLE* st2_table[50];
ST_TABLE* merge_table[50];

// file pointer for read for two storage table for merge
FILE* fp_st1;
FILE* fp_st2;

// number of elements of each storage tables
int num_elems_st1;
int num_elems_st2;
int num_elems_merged;

int minorder;

// find least recent two storage table
void st_open(SHM_FLAGS* flags){
	// find first storage table to open
	int first_st = 0;
	int second_st = 0;

	if (flags->st_num == 2){
		first_st = flags->st[1];
		second_st = flags->st[0];
	}
	else if (flags->st_num == 3){
		first_st = flags->st[2];
		second_st = flags->st[1];
	}

	char filename[10];
	char filename2[10];
	sprintf(filename, "%d.st", first_st);
	sprintf(filename2, "%d.st", second_st);
	fprintf(stderr, "merge %d.st and %d.st ...", first_st, second_st);

	fp_st1 = fopen(filename, "r");
	fp_st2 = fopen(filename2, "r");
	if (fp_st1 == NULL || fp_st2 == NULL){
		printf("there is not enough storage table to merge!\n");
	}

	int read_elems;
	int elem_order, elem_key;
	unsigned char elem_value[32];
	int cur_idx = 0;
	for (int i=0; i<50; i++){
		st1_table[i] = (ST_TABLE*)malloc(sizeof(ST_TABLE));
		st2_table[i] = (ST_TABLE*)malloc(sizeof(ST_TABLE));
	}


	while ((read_elems = fscanf(fp_st1, "%d %d %s", &elem_order, &elem_key, (char*)elem_value)) != EOF){
		// load each element of first storage table
		fprintf(stderr, "k");
		fprintf(stderr, "%d %d %sjj", elem_order, elem_key, (char*)elem_value);
		st1_table[cur_idx]->order = elem_order;
		st1_table[cur_idx]->key = elem_key;
		strncpy(st1_table[cur_idx++]->value, elem_value, sizeof(elem_value));
		fprintf(stderr, "%d %d %s", elem_order, elem_key, (char*)st1_table[cur_idx-1]->value);
	}
	fprintf(stderr, "quit here\n");
	st1_table[cur_idx]->key = -1;
	minorder = st1_table[0]->order;
	num_elems_st1 = cur_idx;
	
	cur_idx = 0;
	while ((read_elems = fscanf(fp_st2, "%d %d %s", &elem_order, &elem_key, (char*)elem_value)) != EOF){
                // load each element of second storage table
		fprintf(stderr,"h\n");
                st2_table[cur_idx]->order = elem_order;
                st2_table[cur_idx]->key = elem_key;
                strncpy(st2_table[cur_idx++]->value, elem_value, sizeof(elem_value));
        }
	st2_table[cur_idx]->key = -1;
	num_elems_st2 = cur_idx;
	// find out min value of order
	// need for saving merged table again
	minorder = (minorder < st2_table[0]->order) ? minorder : st2_table[0]->order;

	fclose(fp_st1);
	fclose(fp_st2);
	fprintf(stderr, "merge save\n");
}

void self_sort(){
	int idx = 0;
	int swap = 0;
	for (idx = 0; idx < num_elems_st1-1; idx++){
		swap = 0;
		for (int j=0; j < num_elems_st1 - idx - 1; j++){
			// bigger key goes to back
			if (st1_table[j]->key > st1_table[j+1]->key){
				ST_TABLE* tmp = st1_table[j];
				st1_table[j] = st1_table[j+1];
				st1_table[j+1] = tmp;
				swap = 1;
			}
			else if ((st1_table[j]->key == st1_table[j+1]->key) && (st1_table[j]->order < st1_table[j+1]->order)){
                                ST_TABLE* tmp = st1_table[j];
                                st1_table[j] = st1_table[j+1];
                                st1_table[j+1] = tmp;
                                swap = 1;
                        }
		}
		// no swap : everything is in order
		if (!swap){
			break;
		}
	}
	//same for stable 2
	swap = 0;
	for (idx = 0; idx < num_elems_st2-1; idx++){
                swap = 0;
                for (int j=0; j < num_elems_st2 - idx - 1; j++){
                        if (st2_table[j]->key > st2_table[j+1]->key){
                                ST_TABLE* tmp = st2_table[j];
                                st2_table[j] = st2_table[j+1];
                                st2_table[j+1] = tmp;
                                swap = 1;
                        }
			else if ((st2_table[j]->key == st2_table[j+1]->key) && (st2_table[j]->order < st2_table[j+1]->order)){
				ST_TABLE* tmp = st2_table[j];
				st2_table[j] = st2_table[j+1];
				st2_table[j+1] = tmp;
				swap = 1;
			}
                }
                if (!swap){
                        break;
                }
        }
}

void merge_sort(){
	self_sort();

	int st1_idx = 0;
	int st2_idx = 0;
	int merged_idx = 0;

	// start point of save order of new st file
	
	while (st1_table[st1_idx]->key != -1 && st2_table[st2_idx]->key != -1){
		if (st1_table[st1_idx]->key < st2_table[st2_idx]->key){
			merge_table[merged_idx++] = st1_table[st1_idx++];	
		}
		else if (st2_table[st2_idx]->key > st1_table[st1_idx]->key){
			merge_table[merged_idx++] = st2_table[st2_idx++];
		}
		// two keys are same : compare order and save one with bigger order (recently saved)
		else {
			if (st1_table[st1_idx]->order < st2_table[st2_idx]->order){
				merge_table[merged_idx - 1] = st2_table[st2_idx++];
				st1_idx++;
			}
			else {
				merge_table[merged_idx - 1] = st1_table[st1_idx++];
				st2_idx++;
			}
		}
	}
	// save rest of the st
	while (st1_idx < num_elems_st1){
		merge_table[merged_idx++] = st1_table[st1_idx++];
	}
	while (st2_idx < num_elems_st2){
		merge_table[merged_idx++] = st2_table[st2_idx++];
	}

	num_elems_merged = merged_idx;
}

void delete_old(SHM_FLAGS* flags){
	// remove old files
	int first_st = 0;
	int second_st = 0;
	if (flags->st_num == 3){
		first_st = flags->st[2];
		second_st = flags->st[1];
	}
	else if (flags->st_num == 2){
		first_st = flags->st[1];
		second_st = flags->st[0];
	}

	char filename[10];
	sprintf(filename, "%d.st", first_st);
	char filename2[10];
	sprintf(filename2, "%d.st", second_st);
	fprintf(stderr, "merge: %d %d delete\n", first_st, second_st);
	int status = remove(filename);
	if (status != 0){
		printf("file delete error : %d.st", first_st);
		return;
	}
	flags->st_num--;
	flags->st[2] = 0;
	status = remove(filename2);
	if (status != 0){
		printf("file delete error : %d.st", second_st);
		return;
	}
	flags->st_num--;
	flags->st[1] = 0;
}

void make_new_st(SHM_FLAGS* flags){
	int new_st = flags->st[0] + 1;
	char filename[10];
	sprintf(filename, "%d.st", new_st);
	FILE* fp_new = fopen(filename, "w");
	
	//int order_cnt = merge_table[0]->order;
	fprintf(fp_new, "%d %d %s\n", minorder++, merge_table[0]->key, (char*)merge_table[0]->value);
	for (int i=1; i<num_elems_merged; i++) {
		if (merge_table[i]->key != merge_table[i-1]->key) {
			fprintf(fp_new, "%d %d %s\n", minorder++, merge_table[i]->key, (char*)merge_table[i]->value);
		}
	}
	// storage table info init
	// st[0] : new table, st[1] : most recent table before new table
	flags->st[1] = flags->st[0];
	flags->st[0] = new_st;
	flags->st_num++;
	fprintf(stderr, "new st %d generated\n", new_st);
	fclose(fp_new);
}

void merge(){
	// shared memory attach
	
	SHM_FLAGS* flags = (SHM_FLAGS *)shmat(shm_flags_id, (char*)NULL, 0);
        SHM_DATA* data = (SHM_DATA *)shmat(shm_data_id, (char*)NULL, 0);
	usleep(1000000);
	flags->quit = 0;
	while(!flags->quit){
		usleep(80000);
		semop(sem_id, &p[0], 1);
		// merge request received from io or st_num becomes 3 (auto merge)
		if (flags->request == 3 || flags->st_num >= 3){
			if (flags->st_num < 2) {
				fprintf(stderr, "not enough storage tables to merge\n");
				flags->request = 0;
				flags->response = 0;
				semop(sem_id, &v[0], 1);
				continue;
			}
			st_open(flags);
			merge_sort();
			delete_old(flags);
			make_new_st(flags);
			flags->response = 1;
			flags->request = 0;
		}
		semop(sem_id, &v[0], 1);
		usleep(80000);
	}

	// shared memory detach
	shmdt((char*)flags);
	shmdt((char*)data);
	
	return;
}
