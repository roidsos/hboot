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
    if(load_config() != HB_SUCESS) {
        goto gigafuxk;
    }

    CHAR16 wide[1024];

    mbstowcs(wide, kernel_path, 1024);

    ST->ConOut->OutputString(ST->ConOut, L"Kernel path:\r\n");
    ST->ConOut->OutputString(ST->ConOut, wide);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    HB_FILE *kernel = file_open(kernel_path, EFI_FILE_MODE_READ);
    if(kernel == NULL) {
        goto fuxk;
    }
    EFI_UINTN size = file_size(kernel);
    ST->ConOut->OutputString(ST->ConOut, L"Booting kernel...\r\n");
    void* kernelbuf = malloc(size);
    if(file_read(kernel, kernelbuf, size) != HB_SUCESS) {
        goto fuxk;
    }
    file_close(kernel);

    if(!validate_elf(kernelbuf, size)){
        goto fuxk;
    }

    EFI_MEMORY_DESCRIPTOR* map = NULL;
    EFI_UINTN   map_size,map_key,map_desc_size,map_desc_version = 0;

    ST->BootServices->GetMemoryMap(&map_size, map, &map_key, &map_desc_size, &map_desc_version);
    ST->BootServices->AllocatePool(EfiLoaderData, map_size, &map); 
    ST->BootServices->GetMemoryMap(&map_size, map, &map_key, &map_desc_size, &map_desc_version);

    //EFI_STATUS status = ST->BootServices->ExitBootServices(IH, map_key);
    //while(status == EFI_SUCCESS) {
    //    ST->BootServices->ExitBootServices(IH, map_key);
    //}

    if(ramfs_path != NULL) {
        //TODO: load ramfs
    }

    //unreachable
    ST->ConOut->OutputString(ST->ConOut, L"Error: kernel exited )=\r\n");
    return;

    fuxk:
    ST->ConOut->OutputString(ST->ConOut, L"Failed to load/parse kernel!\r\n");
    return;

    gigafuxk:
    ST->ConOut->OutputString(ST->ConOut, L"Failed to load/parse config!\r\n");
    return;
}