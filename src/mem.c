#include "mem.h"

struct arena arena = {0};

void* alloc(const size_t n) {
	const size_t p = arena.p;
	arena.p += n;
	return arena.d + p;
}