#include "krnlldr.h"
#include "libc/string.h"
#include <core/core.h>
#include <core/krnlldr.h>
#include <core/protocol.h>
#include <core/libc/file.h>
#include <core/libc/malloc.h>

char* kernel_path = NULL;
char* kernel_cmdline = NULL;
char* ramfs_path = NULL;

void fix_path(char* path) {
    for(int i = 0; i < strlen(path); i++) {
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
    if (file == NULL) {
        goto error;
    }

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

    if(!validate_elf(kernelbuf, size)){
        goto fuxk;
    }
    ST->ConOut->OutputString(ST->ConOut, L"Finally the goddamn thing read the file\r\n");

    EFI_MEMORY_DESCRIPTOR* map = NULL;
    EFI_MEMORY_DESCRIPTOR tmpmap[1];
    EFI_UINTN   map_size = sizeof(tmpmap);
    EFI_UINTN   map_key = 0;
    EFI_UINTN   map_desc_size = 0;
    EFI_UINT32  map_desc_version = 0;

    ST->BootServices->GetMemoryMap(&map_size, tmpmap, &map_key, &map_desc_size, &map_desc_version);

    map_size += 4096;

    //s = ST->BootServices->FreePool(map); 
    //if(s != EFI_SUCCESS) {
    //    CHAR16 err[5];
    //    err[0] = '0' + s;
    //    err[1] = 'a';
    //    err[2] = '\r';
    //    err[3] = '\n';
    //    err[4] = '\0';
    //    ST->ConOut->OutputString(ST->ConOut, err);
    //    goto fuxk2;
    //}

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


    if(ramfs_path != NULL) {
        //TODO: load ramfs
    }

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