#include "memory.h"

/* THIS SECTION FOR FILE TYPES */
	typedef struct chunk{
		struct chunk* next;
		//struct chunk* previous;
		int size;
		char memory[0];
	}chunk;

/* THIS SECTION FOR FUNCTION DECLARATIONS */
	void insertChunk(chunk* c);
	void removeChunk(chunk* c);
	chunk* getFreeChunk(size_t s);

/* THIS SECTION FOR FILE GLOBAL VARIABLES */
	chunk* freeListHead = NULL;

/* THIS SECTION FOR FUNCTION IMPLEMENTATIONS */
	
	void insertChunk(chunk* c){
		c->next = freeListHead;
		freeListHead = c;
	}

	void removeChunk(chunk* c){
		printf("(1)");
		if(freeListHead == NULL || c == NULL){
			return;
		}

		printf("(2)");
		if(freeListHead == c){
			freeListHead = freeListHead->next;
			return;
		}

		printf("(3)");
		chunk* tmp = freeListHead;
		while(tmp->next != c && tmp != NULL){
			tmp = tmp->next;
		}

		printf("(4)");
		if(tmp->next == c){
			tmp->next = c->next;
			c->next = NULL;
		}
	}

	chunk* getFreeChunk(size_t s){
		if(s <= 0){
			return NULL;
		}
		int k = s;
		while((k % 8) != 0){
			k++;
		}
		//printf("\nCharlie\n");
		chunk* tmp = freeListHead;
		chunk* rec = NULL;
		while(tmp != NULL){
			if( (rec == NULL) && (tmp->size >= k) && (tmp->size <= (2*k)) ){
				rec = tmp;
			}
			tmp = tmp->next;
		}
		//printf("\nBravo\n");
		if(rec == NULL){
			tmp = freeListHead;
			while(tmp != NULL){
				if( (rec == NULL) && (tmp->size >= k) ){
					rec = tmp;
				}
				tmp = tmp->next;
			}
		}

		//printf("\ndelta6\n");
		if(rec != NULL){
			removeChunk(rec);
			//printf("\nepsilon\n");
		}
		//will return first sizable chunk not more than twice size requested,
		//else returns first sizable chunk if any available, else returns null
		
		return rec;
	}

	void* malloc(size_t size){
	
	}

	void free(void *ptr){

	}

	void* calloc(size_t nmemb, size_t size){

	}

	void* realloc(void *ptr, size_t size){

	}

	//TODO:
	//for testing purposes, please remove later.
	int main(){
		chunk arr[10];
		int i = 0;
		for(i = 0; i < 10; i++){
			arr[i].size = ((((1 << (i+6)) % 100) / 10) * 16) + 2;
			printf("%u,%d,%d\n", &arr[i], i, arr[i].size);
		}

		printf("\n**creating*\n");

		for(i = 9; i >= 0; i--){
			insertChunk(&arr[i]);
		}

		chunk* head = freeListHead;
		while(head != NULL){
			printf("%u,%d\n", head, head->size);
			head = head->next;
		}
		printf("\n**getting*\n");

		chunk* chs[10];
		for(i = 0; i <= 9; i++){
			chs[i] = getFreeChunk(i+1);
			printf("%u,%d,%d\n", chs[i], ((chs[i] != NULL)?(chs[i]->size):(0)), i);
		}

		printf("\n**iterating free*\n");
		head = freeListHead;
		while(head != NULL){
			printf("%u,%d\n", head, head->size);
			head = head->next;
		}

		printf("\nDone\n");
	}

