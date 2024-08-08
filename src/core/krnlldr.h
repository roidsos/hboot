#ifndef __KRNLLDR_H__
#define __KRNLLDR_H__

#include "elf.h"
#include <core/elf.h>
#include <stddef.h>

int validate_elf(void* elf);

Elf64_Phdr* get_phdrs(void* elf);

#endif // __KRNLLDR_H__