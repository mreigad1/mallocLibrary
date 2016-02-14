#include "malloc.h"
#include "stdio.h"
#define TESTING 0

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
        struct chunk* nextAddress;
		struct chunk* leftByAddress;
		struct chunk* rightByAddress;
        struct chunk* leftBySize;
        struct chunk* rightBySize;
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
	chunk* freeSizeRoot = NULL;
    chunk* freeAddressRoot = NULL;

/* THIS SECTION FOR FUNCTION IMPLEMENTATIONS */

    //inserts into size BST
    void insertSize(chunk* newNode, chunk* root){
        if(TESTING){
            printf("\t[insertChunkSize(%p,%p)]\n", newNode, root);
        }
        
        if(newNode == NULL || root == NULL){
            return;
        }
        
        if(root->size < newNode->size){
            if(root->leftBySize == NULL){
                root->leftBySize = newNode;
            }else{
                insertSize(newNode, root->leftBySize);
            }
        }else{
            if(root->rightBySize == NULL){
                root->rightBySize = newNode;
            }else{
                insertSize(newNode, root->rightBySize);
            }
        }
    }

    //inserts into address BST
    void insertAddress(chunk* newNode, chunk* root){
        if(TESTING){
            printf("\t[insertChunkAddress(%p,%p)]\n", newNode, root);
        }
        
        if(newNode == NULL || root == NULL){
            return;
        }
        
        if(root < newNode){
            if(root->leftByAddress == NULL){
                root->leftByAddress = newNode;
            }else{
                insertAddress(newNode, root->leftByAddress);
            }
        }else{
            if(root->rightByAddress == NULL){
                root->rightByAddress = newNode;
            }else{
                insertAddress(newNode, root->rightByAddress);
            }
        }
    }

    //place new chunk at front of list
	void insertChunk(chunk* c){
        if(TESTING){
            printf("\t[insertChunk(%p)]\n", c);
        }
        
        if(c == NULL){  //Do not insert NULL
            return;
        }
        
        if(freeSizeRoot == NULL){
            freeSizeRoot = c;
        }else{
            insertSize(c, freeSizeRoot);
        }
        
        if(freeAddressRoot == NULL){
            freeAddressRoot = c;
        }else{
            insertAddress(c, freeAddressRoot);
        }
	}

    //will insert subtree back into address tree
    void insertAddressTree(chunk* c){
        if(c == NULL){
            return;
        }
        
        chunk* leftAddress = c->leftByAddress;
        chunk* rightAddress = c->rightByAddress;
        c->rightByAddress = c->leftByAddress = NULL;
        
        insertAddress(c, freeAddressRoot);
        insertAddressTree(leftAddress);
        insertAddressTree(rightAddress);
    }

    //will remove chunk from address bst
    void removeAddress(chunk* toRemove, chunk* root){
        if(toRemove == root){
            return; //this should never happen
        }else if(toRemove < root){
            if(root->leftByAddress == NULL){
                return; //not found, should never happen
            }else if(root->leftByAddress == toRemove){
                chunk* leftLeftAddress = root->leftByAddress->leftByAddress;
                chunk* leftRightAddress = root->leftByAddress->rightByAddress;
                insertAddressTree(leftLeftAddress);
                insertAddressTree(leftRightAddress);
            }else{
                removeAddress(toRemove, root->leftByAddress);
            }
        }else{
            if(root->rightByAddress == NULL){
                return;
            }else if(root->rightByAddress == toRemove){
                chunk* rightLeftAddress = root->rightByAddress->leftByAddress;
                chunk* rightRightAddress = root->rightByAddress->rightByAddress;
                insertAddressTree(rightLeftAddress);
                insertAddressTree(rightRightAddress);
            }
        }
    }

    //will insert subtree back into size tree
    void insertSizeTree(chunk* c){
        if(c == NULL){
            return;
        }
    
        chunk* leftSize = c->leftBySize;
        chunk* rightSize = c->rightBySize;
        c->rightBySize = c->leftBySize = NULL;
    
        insertSize(c, freeSizeRoot);
        insertSizeTree(leftSize);
        insertSizeTree(rightSize);
    }

    //will remove chunk from size bst
    void removeSize(chunk* toRemove, chunk* root){
        if(toRemove == root){
            return; //this should never happen
        }else if(toRemove < root){
            if(root->leftBySize == NULL){
                return; //not found, should never happen
            }else if(root->leftBySize == toRemove){
                chunk* leftLeftSize = root->leftBySize->leftBySize;
                chunk* leftRightSize = root->leftBySize->rightBySize;
                insertSizeTree(leftLeftSize);
                insertSizeTree(leftRightSize);
            }else{
                removeSize(toRemove, root->leftBySize);
            }
        }else{
            if(root->rightBySize == NULL){
                return;
            }else if(root->rightBySize == toRemove){
                chunk* rightLeftSize = root->rightBySize->leftBySize;
                chunk* rightRightSize = root->rightBySize->rightBySize;
                insertSizeTree(rightLeftSize);
                insertSizeTree(rightRightSize);
            }
        }
    }

    //will remove specified chunk from free list
	void removeChunk(chunk* c){
        if(TESTING){
            printf("\t[removeChunk(%p)]\n", c);
        }
        
        if(c == NULL){  // do not remove NULL
            return;
        }
        
        if(freeAddressRoot == c){   //remove chunk from addresses
            chunk* rightAddress = freeAddressRoot->rightByAddress;
            chunk* leftAddress = freeAddressRoot->leftByAddress;
            freeAddressRoot->leftByAddress = freeAddressRoot->rightByAddress = NULL;
            if(rightAddress != NULL){
                freeAddressRoot = rightAddress;
                insertAddressTree(leftAddress);
            }else{
                freeAddressRoot = leftAddress;
                insertAddressTree(rightAddress);
            }
        }else if(freeAddressRoot != NULL){
            removeAddress(c, freeAddressRoot);
        }
        
        if(freeSizeRoot == c){  //remove chunk from sizes
            chunk* rightSize = freeSizeRoot->rightBySize;
            chunk* leftSize = freeSizeRoot->leftBySize;
            freeSizeRoot->leftBySize = freeSizeRoot->rightBySize = NULL;
            if(rightSize != NULL){
                freeSizeRoot = rightSize;
                insertSizeTree(leftSize);
            }else{
                freeSizeRoot = leftSize;
                insertSizeTree(rightSize);
            }
        }else if(freeSizeRoot != NULL) {
            removeSize(c, freeSizeRoot);
        }
        
	}

    //Find minimum sized candidate chunk in tree, remove from tree
    chunk* getFreeChunkAux(size_t size, chunk* root){
        if(TESTING){
            printf("\t[getFreeChunkAux(%zu,%p)]\n", size, root);
        }
        size_t s = nextEight(size);
        chunk* retVal = NULL;
        chunk* childCandidate = NULL;
    
        if(s <= 0){
            return NULL;
        }
        
        if(root == NULL){
            return NULL;
        }
    
        chunk* left = root->leftBySize;
        chunk* right = root->rightBySize;
    
        if(root->size >= s){    //if this root is large enough, check that smaller node won't do the trick
            retVal = root;
            childCandidate = getFreeChunkAux(s, left);
            if(childCandidate != NULL){
                if(childCandidate->size >= s){
                    retVal = childCandidate;
                }
            }
        }else{                  //root is not large enough, check for larger sufficient sized node
            retVal = childCandidate = getFreeChunkAux(s, right);
        }
        
        if(root == freeSizeRoot){   //won't happen until leaving function recursion tree
            removeChunk(retVal);
        }
    
        return retVal;
    }

    //Function gets chunk of memory from free list if available, else returns NULL
	chunk* getFreeChunk(size_t size){
		if(TESTING){
            printf("\t[getFreeChunk(%zu)]\n", size);
        }
        
        size_t s = nextEight(size);
        chunk* retVal = NULL;
        
        if(freeSizeRoot == NULL){
            return NULL;
        }
        
        retVal = getFreeChunkAux(s, freeSizeRoot);
        
        if(retVal != NULL){
            //if excess space in chunk is big enough to store a new chunk with 16 bytes
            //and this chunk is less than half used then split
            if(retVal->size > (2*s) && (retVal->size > (s + sizeof(chunk) + 16))){
                size_t oldSize = retVal->size;
                retVal->size = s;
                chunk* newChunk = retVal->memory + s;
                newChunk->size = oldSize - (s + sizeof(chunk));
                insertChunk(newChunk);
            }
        }
        
		return retVal;
	}

    //gets highest address item in tree
    chunk* getHighestNodeAddress(chunk* root){
        if(root == NULL){
            return NULL;
        }
        
        if(root->rightByAddress != NULL){
            return getLowestNodeAddress(root->rightByAddress);
        }else{
            return root;
        }
    }

    //function merges two input nodes if they are adjacent
    chunk* mergeNodes(chunk* n1, chunk* n2){
        if(n1 == NULL || n2 == NULL){
            return NULL;
        }
        if(n1 > n2){    //swap to ensure n1 is lower address node
            chunk* tmp = n1;
            n1 = n2;
            n2 = tmp;
        }
        if(n1->memory + n1->size == n2){    //nodes adjacent, should be merged
            n1->nextAddress = n2->nextAddress;
            n1->size = n1->size + sizeof(chunk) + n2->size;
            return n1;
        }
        return NULL;
    }

    //function defragments memory chunks
    void mergeAdjacents(){
        chunk* list = NULL;
        while(freeAddressRoot != NULL) {
            chunk* highAddress = getHighestNodeAddress(freeAddressRoot);
            removeChunk(highAddress);
            highAddress->nextAddress = list;
            list = highAddress;
        }
        //tail contains list of all nodes in sorted address order, all nodes have been pulled from trees
        chunk* head = list;
        while(head != NULL){
            chunk* tail = head->nextAddress;
            while(head != NULL && mergeNodes(head,tail) == head){//merge as many chunks as possible
                tail = head->nextAddress;
            }
            head = tail;
            if(tail != NULL){
                tail = tail->nextAddress;
            }
        }
        while(list != NULL){
            chunk* next = list->nextAddress;
            list->nextAddress = NULL;
            insertChunk(list);
            list = next;
        }
    }


    //function ceilings to next increment of 8
    size_t nextEight(size_t s){
        if(TESTING){
            printf("\t[nextEight(%zu)]\n", s);
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
            printf("\t[CallForChunk(%zu)]\n", size);
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
            printf("\t[malloc(%zu)]\n", size);
        }
        
        size_t s = nextEight(size);
        
        if(s <= 0){
            return NULL;
        }
        
        //attempt to retrieve chunk of memory already in process
        chunk* c = getFreeChunk(s);
        
        //if new memory must be requested then defer to system call
        if(c == NULL){
            //making single system call for two memory request
            //assuming that if block of size 's' is needed, it is
            //likely a block of size 's' will be needed again soon.
            void* block = CallForChunk( s + sizeof(chunk) );
            c = (chunk*) block;
        }
        
        if(c == NULL){  //should be impossible for c to be NULL unless system call fails
            return NULL;
        }else{
            return (c->memory); //return address following chunk header
        }
	}

    //TODO:
	void free(void *ptr){
        if(TESTING){
            printf("\t[free(%p)]\n", ptr);
        }
        
        chunk* c = (chunk*)(ptr - sizeof(chunk));
        insertChunk(c);
        
        
	}

    //TODO:
	void* calloc(size_t nmemb, size_t size){
        if(TESTING){
            printf("\t[calloc(%zu,%zu)]\n", nmemb, size);
        }
        if(nmemb == 0 || size == 0){
            return NULL;
        }
        size_t s = nextEight(size);
        size_t totSize = nmemb * s;
        return malloc(totSize);
	}

    //TODO:
	void* realloc(void *ptr, size_t size){
        if(TESTING){
            printf("\t[realloc(%p,%zu)]\n", ptr, size);
        }
        
        if(ptr == NULL){
            return malloc(size);
        }
        if(size <= 0){
            free(ptr);
            return NULL;
        }
        
        chunk* c = (chunk*)(ptr - sizeof(chunk));
        size_t s = nextEight(size);
        
        if(c->size >= s){
            return ptr;
        }else{
            chunk* newChunk = (chunk*)(malloc(s) - sizeof(chunk));
            int i;
            for(i = 0; i < c->size; i++){   //c->size is smaller than new size 's'
                newChunk->memory[i] = c->memory[i];
            }
            free(ptr);
            return (chunk*)(&newChunk->memory[0]);
        }
	}

