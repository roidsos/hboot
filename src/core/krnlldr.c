#include "core.h"
#include "krnlldr.h"
#include "elf.h"

int validate_elf(void* elf) { 
    Elf64_Ehdr* h = (Elf64_Ehdr*)elf;

    if(h->e_ident[0] != 0x7f || h->e_ident[1] != 'E' || h->e_ident[2] != 'L' || h->e_ident[3] != 'F') {
        return HB_FAIL;
    }

    if(h->e_type != ET_EXEC) {
        return HB_FAIL;
    }

    //TODO: more architectures
    if(h->e_machine != EM_X86_64) {
        return HB_FAIL;
    }

    if(h->e_entry < 0xfffffff800000000) {
        return HB_FAIL;
    }

    if(h->e_phoff == 0) {
        return HB_FAIL;
    }

    return HB_SUCCESS;
}

Elf64_Phdr* get_phdrs(void* elf) {
    Elf64_Ehdr *h = (Elf64_Ehdr*)elf;
    return (Elf64_Phdr*)((uintptr_t)elf + h->e_phoff);
}