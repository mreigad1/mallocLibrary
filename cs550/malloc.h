#include "stddef.h"

//for debugging
void breakHere(void* value);

void* cs_malloc(size_t size);

void cs_free(void *ptr);

void* cs_calloc(size_t nmemb, size_t size);

void* cs_realloc(void *ptr, size_t size);

