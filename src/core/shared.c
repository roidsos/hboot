#include "libc/string.h"
#include <core/core.h>
#include <core/libc/file.h>
#include <core/libc/malloc.h>

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
    while (*value_end != '\0' && *value_end != '\n' && *value_end != '\r') {
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
        return HB_FAIL;
    }
    int size = file_size(file);
    char *buffer = (char*)malloc(size);
    if(file_read(file, buffer, size) != HB_SUCESS) {
        return HB_FAIL;
    }

    char* value = NULL;
    cfg_get_key(buffer, "kernel", &value);
    if (value == NULL) {
        return HB_FAIL;
    }
    CHAR16 wide[1024];
    mbstowcs(wide, value, 1024);
    ST->ConOut->OutputString(ST->ConOut, L"Kernel: ");
    ST->ConOut->OutputString(ST->ConOut, wide);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");
    free(value);

    file_close(file);

    return HB_SUCESS;
}


void shared_main()
{
    if(load_config() != HB_SUCESS) {
        ST->ConOut->OutputString(ST->ConOut, L"Failed to load config!\r\n");
    }

    while(1);
}