#include "memory.h"
#include "stdio.h"
#define TESTING 1

/*
    TODO:
    Given deadline time constraints for this program, simple/inefficient
    data structures have been selected to prioritize quickly implemented
    functionality over efficiency.  Functions have been abstracted to
    allow optimization later as time permits.  Once a free tree has been
    implemented over a free list, please remove this comment.
 */

/* THIS SECTION FOR FILE TYPES */
	typedef struct chunk{
		struct chunk* next;
		//struct chunk* previous;
		int size;
		char memory[0];
	}chunk;

/* THIS SECTION FOR FUNCTION DECLARATIONS */

    //insert chunk into free list
    void insertChunk(chunk* c);

    //remove chunk from free list
    void removeChunk(chunk* c);

    //retrieve available chunk and removes chunk from free list
    chunk* getFreeChunk(size_t s);

    //function ceilings to next increment of 8
    size_t nextEight(size_t s);

    //function makes system call to retrieve new memory chunk
    void* CallForChunk(size_t size);

/* THIS SECTION FOR FILE GLOBAL VARIABLES */
	chunk* freeListHead = NULL;

/* THIS SECTION FOR FUNCTION IMPLEMENTATIONS */

    //place new chunk at front of list
	void insertChunk(chunk* c){
        
        if(TESTING){
            printf("[insertChunk(%p)]", c);
        }
        
		c->next = freeListHead;
		freeListHead = c;
	}

    //will remove specified chunk from free list
	void removeChunk(chunk* c){
        
        if(TESTING){
            printf("[removeChunk(%p)]", c);
        }
        
        //if list is empty or chunk pointer is NULL, do nothing and return
		if(freeListHead == NULL || c == NULL){
			return;
		}

		//if chunk is first item in free list
        //move free list head to next item and return
		if(freeListHead == c){
			freeListHead = freeListHead->next;
			return;
		}

		//create variable for list iteration
		chunk* tmp = freeListHead;
        //iterate until pointer is item before removed chunk or end of list
		while(tmp->next != c && tmp != NULL){
			tmp = tmp->next;
		}

		//if not end of list/next item is removed chunk
        //then move pointer to skip chunk, set removed chunk to point next NULL
		if(tmp->next == c){
			tmp->next = c->next;
			c->next = NULL;
		}
	}

    //Function gets chunk of memory from free list if available, else returns NULL
	chunk* getFreeChunk(size_t size){
		
        if(TESTING){
            printf("[getFreeChunk(%zu)]", size);
        }
        
        size_t s = nextEight(size);
        
		chunk* tmp = freeListHead;  //create pointer to head
		chunk* rec = NULL;          //create pointer for return value
        
        //iterate over free list to retrieve chunk
		while(tmp != NULL){
            //if rec not yet assigned and current chunk is not excessively large
			if( (rec == NULL) && (tmp->size >= s) && (tmp->size <= (2*s)) ){
				rec = tmp;
			}
			tmp = tmp->next;
		}
        
        //if no unexcessively sized chunk available then seek excessively large chunk
		if(rec == NULL){
			tmp = freeListHead;
			while(tmp != NULL){
				if( (rec == NULL) && (tmp->size >= s) ){
					rec = tmp;
				}
				tmp = tmp->next;
			}
		}

        //if a chunk has been selected then remove it from the free list
		if(rec != NULL){
			removeChunk(rec);
		}
        
		//will return first sizable chunk not more than twice size requested,
		//else returns first sizable chunk if any available, else returns null
		
		return rec;
	}


    //function ceilings to next increment of 8
    size_t nextEight(size_t s){
        
        if(TESTING){
            printf("[nextEight(%zu)]", s);
        }
        
        size_t r = s;
        while((r % 8) != 0){
            r++;
        }
        return r;
    }

    //function makes system call to retrieve new memory chunk
    void* CallForChunk(size_t size){
        
        if(TESTING){
            printf("[CallForChunk(%zu)]", size);
        }
        
        size_t s = nextEight(size);
        void* retVal = NULL;
        
        //TODO:
        //Make system call here
        
        
        return retVal;
    }

    //TODO:
	void* malloc(size_t size){
        
        if(TESTING){
            printf("[malloc(%zu)]", size);
        }
        
        size_t s = nextEight(size);
        
        //attempt to retrieve chunk of memory already in process
        chunk* c = getFreeChunk(s);
        
        //if new memory must be requested then defer to system call
        if(c == NULL){
            c = (chunk*) CallForChunk(s + sizeof(chunk));
        }
        
        if(c == NULL){
            return NULL;
        }else{
            return &(c->memory[0]);
        }
	}

    //TODO:
	void free(void *ptr){
        if(TESTING){
            printf("[free(%p)]", ptr);
        }
        
        ;
	}

    //TODO:
	void* calloc(size_t nmemb, size_t size){
        if(TESTING){
            printf("[calloc(%zu,%zu)]", nmemb, size);
        }
        
        return NULL;
	}

    //TODO:
	void* realloc(void *ptr, size_t size){
        if(TESTING){
            printf("[realloc(%p,%zu)]", ptr, size);
        }
        
        return NULL;
	}

	//TODO:
	//for testing purposes, please remove later.
	int main(){
        if(TESTING){
            printf("[main()]");
        }
        
		chunk arr[10];
		int i = 0;
		for(i = 0; i < 10; i++){
			arr[i].size = ((((1 << (i+6)) % 100) / 10) * 16) + 2;
			printf("%p,%d,%d\n", &arr[i], i, arr[i].size);
		}

		printf("\n**creating*\n");

		for(i = 9; i >= 0; i--){
			insertChunk(&arr[i]);
		}

		chunk* head = freeListHead;
		while(head != NULL){
			printf("%p,%d\n", head, head->size);
			head = head->next;
		}
		printf("\n**getting*\n");

		chunk* chs[10];
		for(i = 0; i <= 9; i++){
			chs[i] = getFreeChunk(i+1);
			printf("%p,%d,%d\n", chs[i], ((chs[i] != NULL)?(chs[i]->size):(0)), i);
		}

		printf("\n**iterating free*\n");
		head = freeListHead;
		while(head != NULL){
			printf("%p,%d\n", head, head->size);
			head = head->next;
		}

		printf("\nDone\n");
	}

