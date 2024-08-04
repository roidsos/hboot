#include <core/core.h>
#include <core/libc/malloc.h>

void *malloc(size_t size) {
    void* ptr;
    ST->BootServices->AllocatePool(EfiLoaderData, size, &ptr);
    return ptr;
}

void free(void *ptr) {
    ST->BootServices->FreePool(ptr);
}