#include "malloc.h"
//#include "stdio.h"
//#include "stdlib.h"
#include "unistd.h"
//#include "time.h"
#define TESTING 0
#define TESTNUM 1000

/* THIS SECTION FOR FILE TYPES */
	typedef struct chunk{
		struct chunk* LeftAddress;
		struct chunk* RightAddress;
		struct chunk* nextAddress;
		//struct chunk* leftBySize;
		//struct chunk* rightBySize;
		size_t size;
		char memory[0];
	}chunk;

/* THIS SECTION FOR FUNCTION DECLARATIONS */

	//insert chunk into free list
	int insertChunk(chunk* c);

	//retrieve available chunk and removes chunk from free list
	chunk* getFreeChunk(size_t s);

	//function ceilings to next increment of 8
	size_t nextEight(size_t s);

	//function makes system call to retrieve new memory chunk
	void* CallForChunk(size_t size);

/* THIS SECTION FOR FILE GLOBAL VARIABLES */
	chunk* addressRoot = NULL;
	chunk* listHead = NULL;
	void* heapEnd = NULL;
	void* breakVar = 0;

/* THIS SECTION FOR FUNCTION IMPLEMENTATIONS */

	//function for debugging
	void breakHere(void* value){
		breakVar = value;
	}


	//returns reference to highest address node in tree
	chunk* highestAddress(chunk* root){
		if(root == NULL){
			return NULL;
		}else if(RightAddress != NULL){
			return RightAddress;
		}else{
			return root;
		}
	}

	//flattens tree into list, merges nodes, creates new tree
	void defragmentMemory(){
		chunk* greatestNode = highestAddress(addressRoot);
		while(greatestNode != NULL){
			removeChunk(greatestNode);
			greatestNode->LeftAddress = greatestNode->RightAddress = NULL;
			greatestNode->nextAddress = listHead;
			listHead = greatestNode;
			greatestNode = highestAddress(addressRoot);
		}
		chunk* tmp = listHead;
		while(tmp != NULL){
			void* nxt = (void*)(tmp->nextAddress);
			if((tmp->memory + tmp->size) == nxt){
				chunk* nextChunk = (chunk*)nxt;
				tmp->size += (sizeof(chunk) + nextChunk->size);
				tmp->nextAddress = nextChunk->nextAddress;
				nextChunk->nextAddress = NULL;
			}
			tmp = tmp->nextAddress;
		}
		while(listHead != NULL){
			chunk* toTree = listHead;
			listHead = toTree->nextAddress;
			toTree->nextAddress = NULL;
			insertChunk(toTree);
		}
	}

	//Returns -1 if c has children (will not insert), 0 if c is dropped, 1 if inserted, will not merge adjacent blocks
	int addressInsert(chunk* c, chunk* root){
		//void* lower  = (c < root)?c:root;
		//void* higher = (c > root)?c:root;
		//chunk* Lower = (chunk*)lower;
		//chunk* Higher = (chunk*)higher;

		if(root == NULL || c == NULL){
			return 0;	//root or child is NULL
		}

		if(c->LeftAddress != NULL || c->RightAddress != NULL){
			return -1;
		}

		//node not adjacent to root, recurse or insert
		chunk* child = NULL;

		if(c < root){
			child = root->LeftAddress;	//left recursion child
			if(root->LeftAddress == NULL){	//no left child, insert left
				root->LeftAddress = c;
				return 1;
			}
		}else{
			child = root->RightAddress;	//right recursion child
			if(root->RightAddress == NULL){	//no right child, insert right
				root->RightAddress = c;
				return 1;
			}
		}

		return addressInsert(c, child);	//not inserted then return result of child recursion
	}

	//return chunk to OS if possible, otherwise place in free tree
	//return merge result, or 2 if given back to OS
	int insertChunk(chunk* c){
		if(c == NULL){
			return 0;
		}
		void* ptr = c;
		size_t s = sizeof(chunk) + c->size;
		void* end = ptr + s;
		if(end == heapEnd){	//determine if chunk is to be given back to OS
			intptr_t ns = -s;
			sbrk(ns);
			heapEnd = ptr;
			return 2;		//do nothing more if memory returned to system
		}else{	//not returned to OS, place chunk in tree
			return addressInsert(c, addressRoot);
		}
	}

	//remove nodes from and place into root
	void insertAddressTree(chunk* fromTree){
		if(fromTree == NULL){
			return;
		}

		chunk* L = fromTree->LeftAddress;
		chunk* R = fromTree->RightAddress;

		fromTree->LeftAddress = NULL;
		fromTree->RightAddress = NULL;

		insertChunk(fromTree);
		insertAddressTree(L);
		insertAddressTree(R);
	}

	//will remove specified chunk from tree 'root'
	void removeAddress(chunk* c, chunk* root){
		if(c == NULL || root == NULL){  // do not remove NULL
			return;
		}

		chunk* L = root->LeftAddress;
		chunk* R = root->RightAddress;
		chunk* LL = (L == NULL)?NULL:(L->LeftAddress);
		chunk* LR = (L == NULL)?NULL:(L->RightAddress);
		chunk* RL = (R == NULL)?NULL:(R->LeftAddress);
		chunk* RR = (R == NULL)?NULL:(R->RightAddress);

		if(c == addressRoot && c == root){
			addressRoot->LeftAddress = addressRoot->RightAddress = NULL;
			if(L != NULL){
				addressRoot = L;
				insertAddressTree(R);
			}else{
				addressRoot = R;
				insertAddressTree(L);
			}
		}else if(c < root){
			if(c == L){
				L->LeftAddress = L->RightAddress = NULL;
				if(LL != NULL){
					root->LeftAddress = LL;
					insertAddressTree(LR);
				}else{
					root->LeftAddress = LR;
					insertAddressTree(LL);
				}				
			}else{
				removeAddress(c, L);
			}
		}else if(c >= root){
			if(c == R){
				R->LeftAddress = R->RightAddress = NULL;
				if(RL != NULL){
					root->RightAddress = RL;
					insertAddressTree(RR);
				}else{
					root->RightAddress = RR;
					insertAddressTree(RL);
				}
			}else{
				removeAddress(c, R);
			}
		}
	}

	void removeChunk(chunk* c){
		removeAddress(c , addressRoot);
	}

	//search tree for large enough chunk, do NOT remove yet
	chunk* getFreeChunkAux(size_t s, chunk* root){		
		if(root == NULL){
			return NULL;
		}
		chunk* retVal = NULL;

		if(root->size >= s){
			retVal = root;
		}
		if(retVal == NULL){
			retVal = getFreeChunkAux(s, root->LeftAddress);
		}
		if(retVal == NULL){
			retVal = getFreeChunkAux(s, root->RightAddress);
		}
		return retVal;
	}

	//Function gets reference to properly sized chunk in BST and removes it from tree
	chunk* getFreeChunk(size_t size){        
		size_t s = nextEight(size);
		chunk* retVal = NULL;

		if(addressRoot == NULL){
			return NULL;
		}

		retVal = getFreeChunkAux(s, addressRoot);

		if(retVal != NULL){

			removeChunk(retVal);
			size_t adjustedReqSize = s + sizeof(chunk);
			size_t retrievedChunkSize = retVal->size + sizeof(chunk);
			size_t minimumSplitRemainder = 2*sizeof(chunk);

			if(retrievedChunkSize > (minimumSplitRemainder + adjustedReqSize)){	//split chunks, don't bother if created chunk usable space smaller than overhead of chunk
				void* newChunk = (void*)retVal;
				newChunk += adjustedReqSize;							//get address of new chunk
				chunk* nChunk = (chunk*)newChunk;						//cast new chunk back to record ptr
				retVal->size = s;								//resize old block to requested size
				nChunk->size = retrievedChunkSize - (sizeof(chunk) + sizeof(chunk) + s);	//size new block to 
				insertChunk(nChunk);
			}
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
	//preloads chunk size
	void* CallForChunk(size_t size){
		size_t s = nextEight(size);
		void* retVal = NULL;
		retVal = sbrk(s);
		
		if(retVal != NULL){
			heapEnd = sbrk(0);
			chunk* c = retVal;
			c->size = s;
		}
		return retVal;
	}

	//implemented
	void* malloc(size_t size){
		size_t s = nextEight(size + sizeof(chunk));

		if(s <= 0){
			//breakHere(NULL);
			return NULL;
		}
		
		chunk* c = NULL;

		void* block = getFreeChunk( s );	//get chunk in space

		if(block == NULL){
			defragmentMemory();		//if no sufficiently large node found in free tree, try defragmenting and searching
			block = getFreeChunk( s );
		}

		if(block == NULL){
			block = CallForChunk( s );	//no candidate chunk in space, get new chunk
		}
		
		if(block != NULL){
			c = (chunk*) block;
		}else{
			return NULL;	//if block is still NULL then return NULL
		}

		return (c->memory); //return address following chunk header
	}

	void free(void *ptr){

		if(ptr == NULL || ptr == (NULL-sizeof(chunk))){
			breakHere(ptr);
		}

		if(ptr == NULL){
			return;
		}
		ptr -= sizeof(chunk);
		chunk* c = (chunk*)ptr;
		insertChunk(c);
	}

	//implemented
	void* calloc(size_t nmemb, size_t size){
		if(nmemb == 0 || size == 0){
			//breakHere(NULL);
			return NULL;
		}
		size_t s = nextEight(size);
		size_t totSize = nmemb * s;
		void* retVal = malloc(totSize);

		if(retVal == NULL){
			//breakHere(NULL);
		}

		return retVal;
	}

	//implemented
	void* realloc(void *ptr, size_t size){
		void* retVal = NULL;
		if(ptr == NULL){
			return malloc(size);
		}
		if(size <= 0){
			free(ptr);
			//breakHere(NULL);
			return NULL;
		}

		chunk* c = (chunk*)(ptr - sizeof(chunk));
		size_t s = nextEight(size);

		if(c->size >= s){
			return ptr;
		}else{
			chunk* newChunk = (chunk*)(malloc(s) - sizeof(chunk));
			size_t i;
			for(i = 0; i < c->size; i++){   //c->size is smaller than new size 's'
				newChunk->memory[i] = c->memory[i];
			}
			retVal = (&newChunk->memory[0]);
			free(ptr);

			if(retVal == NULL){
				//breakHere(NULL);
			}

			return retVal;
		}
	}


/*int main(){
	int* arr[TESTNUM];
	int sizes[TESTNUM];
	int i;


	for(i = 0; i < TESTNUM; i++){
		arr[i] = malloc(sizeof(int));
		arr[i][0] = sizeof(int) + sizes[i];
	}

	for(i = 0; i < TESTNUM; i++){
		free(arr[i]);
	}


	for(i = 0; i < TESTNUM; i++){
		arr[i] = malloc(sizeof(int));
		arr[i][0] = sizeof(int) + i*i + 100;
	}

	for(i = 0; i < TESTNUM; i++){
		free(arr[i]);
	}
}
*/
