#include "krnlldr.h"
#include "elf.h"

int validate_elf(void* elf, size_t size) { 
    Elf64_Ehdr* h = (Elf64_Ehdr*)elf;

    if(h->e_ident[0] != 0x7f || h->e_ident[1] != 'E' || h->e_ident[2] != 'L' || h->e_ident[3] != 'F') {
        return 0;
    }

    if(h->e_type != ET_EXEC) {
        return 0;
    }

    //TODO: more architectures
    if(h->e_machine != EM_X86_64) {
        return 0;
    }

    if(h->e_entry < 0xfffffff800000000) {
        return 0;
    }

    if(h->e_phoff == 0) {
        return 0;
    }

    return 1;
}

uintptr_t get_entry_point(void* elf, size_t size, void* entry) {
    Elf64_Ehdr *h = (Elf64_Ehdr*)elf;

    return h->e_entry;
}

Elf64_Phdr* get_phdrs(void* elf, size_t size) {
    Elf64_Ehdr *h = (Elf64_Ehdr*)elf;
    return (Elf64_Phdr*)((uintptr_t)elf + h->e_phoff);
}