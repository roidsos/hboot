#ifndef __MALLOC_H__
#define __MALLOC_H__

#include <core/core.h>

void *malloc(size_t size);
void free(void *ptr);

#endif // __MALLOC_H__