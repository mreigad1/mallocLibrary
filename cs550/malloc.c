#include "malloc.h"
//#include "stdio.h"
#include "unistd.h"
#define TESTING 0
#define TESTNUM 10000

/* THIS SECTION FOR FILE TYPES */
	typedef struct chunk{
		struct chunk* nextAddress;
		struct chunk* leftBySize;
		struct chunk* rightBySize;
		size_t size;
		char memory[0];
	}chunk;

/* THIS SECTION FOR FUNCTION DECLARATIONS */

	//insert chunk into free list
	void insertChunk(chunk* c);

	//remove chunk from free list
	void removeChunk(chunk* c, chunk* root);

	//retrieve available chunk and removes chunk from free list
	chunk* getFreeChunk(size_t s);

	//function ceilings to next increment of 8
	size_t nextEight(size_t s);

	//function makes system call to retrieve new memory chunk
	void* CallForChunk(size_t size);

/* THIS SECTION FOR FILE GLOBAL VARIABLES */
	chunk* freeSizeRoot = NULL;
	unsigned long long memoryUsed = 0;

/* THIS SECTION FOR FUNCTION IMPLEMENTATIONS */

	//inserts node into tree
	void insertTree(chunk* newNode, chunk* root){
		if(newNode == NULL){
			return;		//do not insert null values
		}
		if(root == NULL){
			return;		//cannot insert into null tree
		}

		if(newNode->size < root->size){
			if(root->leftBySize == NULL){
				root->leftBySize = newNode;
			}else{
				insertTree(newNode, root->leftBySize);
			}
		}else{
			if(root->rightBySize == NULL){
				root->rightBySize = newNode;
			}else{
				insertTree(newNode, root->rightBySize);
			}
		}
	}

	//place new chunk at front of list
	void insertChunk(chunk* c){
		if(c == NULL){  //Do not insert NULL
			return;
		}

		if(freeSizeRoot == NULL){
			freeSizeRoot = c;
		}else{
			insertTree(c, freeSizeRoot);
		}
	}

	void insertTreeValues(chunk* from){
		if(from == NULL){
			return;
		}

		chunk* L = from->leftBySize;
		chunk* R = from->rightBySize;
		from->leftBySize = NULL;
		from->rightBySize = NULL;
		insertChunk(from);
		insertTreeValues(L);
		insertTreeValues(R);
	}

	//will remove specified chunk from tree 'root'
	void removeChunk(chunk* c, chunk* root){
		if(c == NULL){  // do not remove NULL
			return;
		}
		if(root == NULL){
			return;
		}

		chunk* L = root->leftBySize;
		chunk* R = root->rightBySize;
		chunk* LL = (L == NULL)?NULL:(L->leftBySize);
		chunk* LR = (L == NULL)?NULL:(L->rightBySize);
		chunk* RL = (R == NULL)?NULL:(R->leftBySize);
		chunk* RR = (R == NULL)?NULL:(R->rightBySize);

		if(c == freeSizeRoot && c == root){
			freeSizeRoot->leftBySize = freeSizeRoot->rightBySize = NULL;
			if(L != NULL){
				freeSizeRoot = L;
				insertTreeValues(R);
			}else{
				freeSizeRoot = R;
				insertTreeValues(L);
			}
		}else if(c->size < root->size){
			if(c == L){
				L->leftBySize = L->rightBySize = NULL;
				if(LL != NULL){
					root->leftBySize = LL;
					insertTreeValues(LR);
				}else{
					root->leftBySize = LR;
					insertTreeValues(LL);
				}				
			}else{
				removeChunk(c, L);
			}
		}else if(c->size >= root->size){
			if(c == R){
				R->leftBySize = R->rightBySize = NULL;
				if(RL != NULL){
					root->rightBySize = RL;
					insertTreeValues(RR);
				}else{
					root->rightBySize = RR;
					insertTreeValues(RL);
				}
			}else{
				removeChunk(c, R);
			}
		}
	}

	//recursively gets reference to proper sized chunk in memory, does not yet remove it
	chunk* getFreeChunkAux(size_t size, chunk* root){
		if(size <= 0){
			return NULL;
		}
		if(root == NULL){
			return NULL;
		}
		chunk* retVal = NULL;

		if(root->size >= size){
			retVal = root;
			if(root->leftBySize != NULL){
				if(root->leftBySize->size >= size){	//if left is smaller than root but larger than size use that instead
					chunk* t = getFreeChunkAux(size, root->leftBySize);
					if(t != NULL){	//make sure search returned valid chunk before overwriting retVal
						retVal = t;
					}
				}
			}
		}else{	//too small, search right
			if(root->rightBySize != NULL){
				chunk* t = getFreeChunkAux(size, root->rightBySize);
				if(t != NULL){
					retVal = t;
				}
			}
		}

		return retVal;
	}

	//Function gets reference to properly sized chunk in BST, does NOT yet remove it
	chunk* getFreeChunk(size_t size){        
		size_t s = nextEight(size);
		chunk* retVal = NULL;

		if(freeSizeRoot == NULL){
			return NULL;
		}

		retVal = getFreeChunkAux(s, freeSizeRoot);

		if(retVal != NULL){
			
			//if excess space in chunk is big enough to store a new chunk with 16 bytes
			//and this chunk is less than half used then split
			/*if(retVal->size > (2*s) && (retVal->size > (s + sizeof(chunk) + 16))){
				size_t oldSize = retVal->size;
				retVal->size = s;
				chunk* newChunk = (chunk*)(retVal->memory + s);
				newChunk->size = oldSize - (s + sizeof(chunk));
				insertChunk(newChunk);
			}*/
		}

		return retVal;
	}

	//implemented
	//function ceilings to next increment of 8
	size_t nextEight(size_t s){
		size_t r = s;
		while((r % 8) != 0){
			r++;
		}
		return r;
	}

	//implemented
	//function makes system call to retrieve new memory chunk
	void* CallForChunk(size_t size){
		size_t s = nextEight(size);
		void* retVal = NULL;
		retVal = sbrk(s);
		if(retVal != NULL){
			memoryUsed += s;
		}
		return retVal;
	}

	//implemented
	void* malloc(size_t size){
		size_t s = nextEight(size);

		if(s <= 0){
			return NULL;
		}

		//attempt to retrieve chunk of memory already in process
		chunk* c = getFreeChunk(s);

		//if new memory must be requested then defer to system call
		if(c == NULL){
			void* block = CallForChunk( s + sizeof(chunk) );
			if(block != NULL){
				c = (chunk*) block;
				c->size = s;
			}
		}else{
			removeChunk(c, freeSizeRoot);
		}

		if(c == NULL){  //should be impossible for c to be NULL unless system call fails
			return NULL;
		}else{
			return (c->memory); //return address following chunk header
		}
	}

	//TODO: Add chunk merging
	void free(void *ptr){
		if(ptr == NULL){
		//	return;
		}//else{
			chunk* c = (chunk*)(ptr - sizeof(chunk));
			insertChunk(c);
		//}
	}

	//implemented
	void* calloc(size_t nmemb, size_t size){
		if(nmemb == 0 || size == 0){
			return NULL;
		}
		size_t s = nextEight(size);
		size_t totSize = nmemb * s;
		return malloc(totSize);
	}

	//implemented
	void* realloc(void *ptr, size_t size){
		void* retVal = NULL;
		if(ptr == NULL){
			return (retVal = malloc(size));
		}
		if(size <= 0){
			free(ptr);
			return (retVal = NULL);
		}

		chunk* c = (chunk*)(ptr - sizeof(chunk));
		size_t s = nextEight(size);

		if(c->size >= s){
			return (retVal = ptr);
		}else{
			chunk* newChunk = (chunk*)(malloc(s) - sizeof(chunk));
			size_t i;
			for(i = 0; i < c->size; i++){   //c->size is smaller than new size 's'
				newChunk->memory[i] = c->memory[i];
			}
			retVal = (&newChunk->memory[0]);
			free(ptr);
			return retVal;
		}
	}


/*int main(){
	int* arr[TESTNUM];
	int i;

	for(i = 0; i < TESTNUM; i++){
		arr[i] = malloc(sizeof(int) + i*i);
		arr[i][0] = sizeof(int) + i*i;
	}

	for(i = 0; i < TESTNUM; i++){
		free(arr[i]);
	}

	for(i = 0; i < TESTNUM; i++){
		arr[i] = malloc(sizeof(int) + i*i);
		arr[i][0] = sizeof(int) + i*i + 100;
	}

	for(i = 0; i < TESTNUM; i++){
		free(arr[i]);
	}
}*/
