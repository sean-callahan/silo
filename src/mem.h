#ifndef MEM_H
#define MEM_H

#include <stdlib.h>

#define MEMORY_SIZE 65536L // 64KB

struct arena {
	size_t p;
	char d[MEMORY_SIZE];
};

extern struct arena arena;

void* alloc(const size_t n);

#endif