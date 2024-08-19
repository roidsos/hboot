#include "config.h"
#include <core/core.h>
#include <core/krnlldr.h>
#include <core/protocol.h>
#include <core/libc/file.h>
#include <core/libc/malloc.h>
#include <stdint.h>
#include <stdbool.h>
#include <efi/efi_types.h>
#include "../elf.h"
#include "../krnlldr.h"
#include "../libc/string.h"
#include "../protocol.h"

extern char* kernel_path;
extern char* kernel_cmdline;
extern char* ramfs_path;

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
    if(file_read(file, buffer, size) != HB_SUCCESS) {
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
    }

    free(buffer);
    file_close(file);
    return HB_SUCCESS;

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