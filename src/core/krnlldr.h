#ifndef __KRNLLDR_H__
#define __KRNLLDR_H__

#include "elf.h"
#include <core/elf.h>
#include <stddef.h>

int validate_elf(void* elf, size_t size);
uintptr_t get_entry_point(void* elf, size_t size, void* entry);

Elf64_Phdr* get_phdrs(void* elf, size_t size);

#endif // __KRNLLDR_H__