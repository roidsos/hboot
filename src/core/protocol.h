#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

// Hornet boot protocol
#define HB_PROTOCOL_VERSION 0x1
#define HB_PROTOCOL_MAGIC   0xdeadbeef

typedef struct {
    uint32_t magic;
    uint32_t version;

    size_t memmap_size;
    uintptr_t memmap;

    size_t ramfs_size;
    uintptr_t ramfs;

} __attribute__((packed)) boot_info;

#endif // __PROTOCOL_H__