#include "elf.h"
#include "krnlldr.h"
#include "libc/string.h"
#include "protocol.h"
#include <core/core.h>
#include <core/krnlldr.h>
#include <core/protocol.h>
#include <core/libc/file.h>
#include <core/libc/malloc.h>
#include <stdint.h>
#include <stdbool.h>

char* kernel_path = NULL;
char* kernel_cmdline = NULL;
char* ramfs_path = NULL;

//deja vu
void __chkstk(){}

void fix_path(char* path) {
    for(size_t i = 0; i < strlen(path); i++) {
        if (path[i] == '/') {
            path[i] = '\\';
        }
    }
}

void cfg_get_key(const char* buffer, const char* key, char** value) {
    if (buffer == NULL || key == NULL || value == NULL) {
        return;
    }

    const char* key_start = strstr(buffer, key);
    if (key_start == NULL) {
        *value = NULL;
        return;
    }

    key_start += strlen(key);
    if (*key_start != '=') {
        *value = NULL;
        return;
    }
    key_start++;

    const char* value_end = key_start;
    while (*value_end != '\n' && *value_end != '\r' && *value_end != 0x00) {
        value_end++;
    }

    size_t value_length = value_end - key_start;

    *value = (char*)malloc(value_length + 1);
    if (*value == NULL) {
        return;
    }

    strncpy(*value, key_start, value_length);
    (*value)[value_length] = '\0';
}

HB_STATUS load_config() {

    //TODO: make config path congigurable
    HB_FILE* file = file_open(L"\\boot\\hboot.conf", EFI_FILE_MODE_READ);

    int size = file_size(file);
    char *buffer = (char*)malloc(size);
    if(file_read(file, buffer, size) != HB_SUCESS) {
        goto error;
    }

    buffer[size] = '\0';

    cfg_get_key(buffer, "kernel", &kernel_path);
    if (kernel_path == NULL) {
        goto error;
    }
    fix_path(kernel_path);

    cfg_get_key(buffer, "kernel_cmdline", &kernel_cmdline);
    if (kernel_cmdline == NULL) {
        goto error;
    }
    cfg_get_key(buffer, "ramfs", &ramfs_path);
    if (ramfs_path != NULL) {
        fix_path(ramfs_path);
        // no problem if the ramfs is not found
    }

    free(buffer);
    file_close(file);
    return HB_SUCESS;

    error:
    free(buffer);
    file_close(file);
    return HB_FAIL;
}

uintptr_t *get_next_lvl(uintptr_t *lvl, uintptr_t index, uintptr_t flags, bool alloc) {

    if (lvl[index] & 1) {
        return (uintptr_t*)((lvl[index] & 0x000FFFFFFFFFF000) + 0xFFFFFFFF80000000);
    }
    if (alloc) {
        uintptr_t *new_lvl = NULL;
        ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, 1, (void*)&new_lvl);
        memset(new_lvl, 0, 4096);
        lvl[index] = (uintptr_t)new_lvl + 0xFFFFFFFF80000000 | flags;
        return new_lvl;
    }
    return 0;
}
extern int shared_end;
void shared_main()
{
    EFI_STATUS s;

    if(load_config() != HB_SUCESS) {
        goto gigafuxk;
    }

    CHAR16 wide[1024];

    mbstowcs(wide, kernel_path, 1024);

    ST->ConOut->OutputString(ST->ConOut, L"Kernel path:\r\n");
    ST->ConOut->OutputString(ST->ConOut, wide);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    HB_FILE *kernel = file_open(wide, EFI_FILE_MODE_READ);
    if(kernel == NULL) {
        goto fuxk;
    }
    EFI_UINTN size = file_size(kernel);
    void* kernelbuf = malloc(size);
    if(file_read(kernel, kernelbuf, size) != HB_SUCESS) {
        goto fuxk;
    }
    file_close(kernel);

    Elf64_Ehdr* h = (Elf64_Ehdr*)kernelbuf;
    if(!validate_elf(kernelbuf)) {
        goto fuxk;
    }

    boot_info* bootinfo;
    s = ST->BootServices->AllocatePool(EfiLoaderData, sizeof(boot_info), (void**)&bootinfo);
    if(s != EFI_SUCCESS) {
        goto fuxk2;
    }
    bootinfo->magic = HB_PROTOCOL_MAGIC;
    bootinfo->version = HB_PROTOCOL_VERSION;

    if(ramfs_path != NULL) {
        CHAR16 ramfs_wide[1024];
        mbstowcs(ramfs_wide, ramfs_path, 1024);
        HB_FILE *ramfs = file_open(ramfs_wide, EFI_FILE_MODE_READ);
        if(ramfs == NULL) {
            goto fuxk2;
        }
        bootinfo->ramfs_size = file_size(ramfs);
        s = ST->BootServices->AllocatePool(EfiLoaderData, bootinfo->ramfs_size, (void**)bootinfo->ramfs);
        if(s != EFI_SUCCESS) {
            goto fuxk2;
        }
        file_read(ramfs, (void*)bootinfo->ramfs, bootinfo->ramfs_size);
        file_close(ramfs);

    } else {
        bootinfo->ramfs = 0;
        bootinfo->ramfs_size = 0;
    }

    EFI_MEMORY_DESCRIPTOR* map = NULL;
    EFI_MEMORY_DESCRIPTOR tmpmap[1];
    EFI_UINTN   map_size = sizeof(tmpmap);
    EFI_UINTN   map_key = 0;
    EFI_UINTN   map_desc_size = 0;
    EFI_UINT32  map_desc_version = 0;

    ST->BootServices->GetMemoryMap(&map_size, tmpmap, &map_key, &map_desc_size, &map_desc_version);

    map_size += 4096;

    s = ST->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&map); 
    if(s != EFI_SUCCESS) {
        CHAR16 err[5];
        err[0] = '0' + s;
        err[1] = 'b';
        err[2] = '\r';
        err[3] = '\n';
        err[4] = '\0';
        ST->ConOut->OutputString(ST->ConOut, err);
        goto fuxk2;
    }

    retry:
    EFI_UINTN retries = 4;
    s = ST->BootServices->GetMemoryMap(&map_size, map, &map_key, &map_desc_size, &map_desc_version);
    if(s != EFI_SUCCESS) {
        retries--;
        if(retries == 0) 
            goto fuxk2; 
        CHAR16 err[5];
        err[0] = '0' + s;
        err[1] = 'c';
        err[2] = '\r';
        err[3] = '\n';
        err[4] = '\0';
        ST->ConOut->OutputString(ST->ConOut, err);
        goto retry;
    }

    s = ST->BootServices->ExitBootServices(IH, map_key);
    if(s != EFI_SUCCESS) {
        retries--;
        if(retries == 0)
            goto fuxk2;
        CHAR16 err[5];
        err[0] = '0' + s;
        err[1] = 'd';
        err[2] = '\r';
        err[3] = '\n';
        err[4] = '\0';
        ST->ConOut->OutputString(ST->ConOut, err);
        goto retry;
    }
    bootinfo->memmap_size = map_size;
    bootinfo->memmap = (uintptr_t)map;

    uintptr_t *pml4 = NULL;
    ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, 1, (void*)&pml4);
    for (Elf64_Phdr* phdr = get_phdrs(kernelbuf);
        (uint64_t)phdr < (uint64_t)kernelbuf + size;
        phdr = (Elf64_Phdr*)((uint64_t)phdr + sizeof(Elf64_Phdr))) {

        if (phdr->p_type == PT_LOAD) {
            uint64_t pages = (phdr->p_memsz + 0xfff) / 0x1000;
            ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, pages, (void*)&phdr->p_paddr)

            //HACK: vmm shit, not very portable
            for(size_t i = 0; i < (phdr->p_memsz + 0xFFF) / 0x1000; i++) {
                size_t pml4_index = ((phdr->p_vaddr + i) >> 39) & 0x1ff;
                size_t pml3_index = ((phdr->p_vaddr + i) >> 30) & 0x1ff;
                size_t pml2_index = ((phdr->p_vaddr + i) >> 21) & 0x1ff;
                size_t pml1_index = ((phdr->p_vaddr + i) >> 12) & 0x1ff;

                pml4 = (uintptr_t*)((uintptr_t)pml4 & 0xFFFFFFFFFFFFF000);
                uintptr_t *pml3 = get_next_lvl(pml4, pml4_index, 3, true);
                uintptr_t *pml2 = get_next_lvl(pml3, pml3_index, 3, true);
                uintptr_t *pml1 = get_next_lvl(pml2, pml2_index, 3, true);
                pml1[pml1_index] = phdr->p_paddr + i*0x1000 | 3; // flags: present and writable
            }

            memcpy((void*)(phdr->p_paddr), (void*)(phdr->p_offset + (uintptr_t)kernelbuf), phdr->p_filesz);
        }
    }
    typedef int (*entry_t)(boot_info* bootinfo);
    entry_t kernel_entry = (entry_t)h->e_entry;
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4));
    kernel_entry(bootinfo);

    //HACK: port e9
    asm volatile("outb %%al, %1" : : "a" ('E'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('r'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('r'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('o'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('r'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" (':'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" (' '), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('T'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('h'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('e'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" (' '), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('k'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('e'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('r'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('n'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('e'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('l'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" (' '), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('e'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('x'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('i'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('t'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('e'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('d'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('!'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('\r'), "Nd" (0xe9) : "memory");
    asm volatile("outb %%al, %1" : : "a" ('\n'), "Nd" (0xe9) : "memory");
    return;

    fuxk:
    ST->ConOut->OutputString(ST->ConOut, L"Failed to load/parse kernel!\r\n");
    return;

    fuxk2:
    ST->ConOut->OutputString(ST->ConOut, L"Failed to exit boot services!\r\n");
    return;

    gigafuxk:
    ST->ConOut->OutputString(ST->ConOut, L"Failed to load/parse config!\r\n");
    return;
}
int shared_end = 0;