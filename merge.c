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

FILE* fp_st1;
FILE* fp_st2;

// number of elements of storage tables
int num_elems_st1;
int num_elems_st2;
int num_elems_merged;

// filename of two open storage table
// needed for delete after merge
int first_st, second_st;
// filename of new storage table
int new_st;

int find_file(){
	int files[3];
	int file_cnt;
	DIR *dir;
	struct dirent *ent;
	dir = opendir(".");
	if (dir != NULL){
		while ((ent = readdir(dir)) != NULL){
			char* dot = strrchr(ent->d_name, '.');
			if (dot && !strcmp(dot, ".st")) {
				char* fname = strtok(ent->d_name, ".");
				files[file_cnt++] = atoi(fname);
				if (file_cnt >= 3) break;
			}
		}
	}
	if (file_cnt <= 1){
		printf("there are not enough tables to merge!");
		return -1;
	}
	// sort by descending order
	if (files[1] > files[2]){
		int temp = files[2];
		files[2] = files[1];
		files[1] = temp;
	}
	if (files[0] > files[1]){
		int temp = files[1];
		files[1] = files[0];
		files[0] = temp;
	}
	if (files[1] > files[2]){
		int temp = files[2];
		files[2] = files[1];
		files[1] = temp;
	}
	first_st = files[0];
	second_st = files[1];
	new_st = files[2]+1;
}

// find least recent two storage table
void st_open(void){
	// find first storage table to open
	/*
	char filename[10] = '\0';
	while (i++){
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%d.st", i);
		// open first storage file (least recent one)
		if ((fp_st1 = fopen(filename, 'r')) != NULL) {
			first_st = i
			break;
		}
		fclose(fp_st1);
	}
	// find second storage table to open
	while (i++){
		memset(filename, 0, sizeof(filename));
		sprintf(filename, "%d.st", i);
		// open second storage file
		if ((fp_st2 = fopen(filename, 'r')) != NULL) {
			second_st = i
			break;
		}
		fclose(fp_st2);
	}
	*/
	char filename[10];
	char filename2[10];
	sprintf(filename, "%d.st", first_st);
	sprintf(filename2, "%d.st", second_st);
	fp_st1 = fopen(filename, "r");
	fp_st2 = fopen(filename2, "r");
	if (fp_st1 == NULL || fp_st2 == NULL){
		printf("there is not enough storage table to merge!\n");
	}

	int read_elems;
	int elem_order, elem_key;
	unsigned char elem_value[32];
	int cur_idx = 0;
	while ((read_elems = fscanf(fp_st1, "%d %d %s", &elem_order, &elem_key, elem_value)) == 3){
		// load each element of first storage table
		st1_table[cur_idx]->order = elem_order;
		st1_table[cur_idx]->key = elem_key;
		strncpy(st1_table[cur_idx++]->value, elem_value, sizeof(elem_value));
	}
	st1_table[cur_idx]->key = -1;
	num_elems_st1 = cur_idx;
	cur_idx = 0;
	while ((read_elems = fscanf(fp_st2, "%d %d %s", &elem_order, &elem_key, elem_value)) == 3){
                // load each element of second storage table
                st2_table[cur_idx]->order = elem_order;
                st2_table[cur_idx]->key = elem_key;
                strncpy(st2_table[cur_idx++]->value, elem_value, sizeof(elem_value));
        }
	st2_table[cur_idx]->key = -1;
	num_elems_st2 = cur_idx;
	fclose(fp_st1);
	fclose(fp_st2);
}

void merge_sort(){

	int st1_idx = 0;
	int st2_idx = 0;
	int merged_idx = 0;

	// start point of save order of new st file
	// int minorder = (st1_table[st1_idx]->order < st2_table[st2_idx]->order) ? st1_table[st1_idx]->order : st2_table[st2_idx]->order;
	
	while (st1_table[st1_idx]->key != -1 && st2_table[st2_idx]->key != -1){
		if (st1_table[st1_idx]->key < st2_table[st2_idx]->key){
			merge_table[merged_idx++] = st1_table[st1_idx++];	
		}
		else if (st2_table[st2_idx]->key > st1_table[st1_idx]->key){
			merge_table[merged_idx++] = st2_table[st2_idx++];
		}
		// two keys are same : compare key and save one with bigger order (recently saved)
		else {
			if (st1_table[st1_idx]->order < st2_table[st2_idx]->order){
				merge_table[merged_idx - 1] = st2_table[st2_idx++];
			}
			else {
				merge_table[merged_idx - 1] = st2_table[st1_idx++];
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
	/*
	int valid_elem = 1;
	for (int i=1; i<merged_idx; i++){
		if (merge_st[i].key != merge_st[i-1].key){
			valid_elem++;
		}
	}
	*/
	num_elems_merged = merged_idx;
}

void delete_old(){
	// remove old files
	char filename[10];
	sprintf(filename, "%d.st", first_st);
	char filename2[10];
	sprintf(filename2, "%d.st", second_st);
	int status = remove(filename);
	if (status != 0){
		printf("file delete error : %d.st", first_st);
	}
	status = remove(filename2);
	if (status != 0){
		printf("file delete error : %d.st", second_st);
	}
}

void make_new_st(){
	char filename[10];
	sprintf(filename, "%d.st", new_st);
	int fd_new = fopen(filename, "w");
	
	int order_cnt = merge_table[0]->order;
	fprintf(fd_new, "%d %d %s", order_cnt++, merge_table[0]->key, merge_table[0]->value);
	for (int i=1; i<num_elems_merged; i++) {
		if (merge_table[i]->key != merge_table[i-1]->key) {
			fprintf(fd_new, "%d %d %s", order_cnt++, merge_table[i]->key, merge_table[i]->value);
		}
	}
}

void merge(){
	// shared memory attach
	mode = (int *)shmat(shm_mode_id, (int*)NULL, 0);
        quit = (int *)shmat(shm_exit_id, (int*)NULL, 0);
        st_num = (int *)shmat(shm_st_num_id, (int*)NULL, 0);
	

	while(!quit){
		// merge request received from I/O
		if (flags->request == 3 || flags->st_num == 2){
			if (find_file() == -1) {
				flags->request = 0;
				continue;
			}
			st_open();
			merge_sort();
			delete_old();
			make_new_st();
			flags->request = 0;
		}
	}

	// shared memory detach
	shmdt((char*)mode);
	shmdt((char*)exit);
	shmdt((char*)st_num);
	
	return;
}
