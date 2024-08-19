#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

// Hornet boot protocol
#define HB_PROTOCOL_VERSION 0x1
#define HB_PROTOCOL_MAGIC   0xdeadbeef

typedef struct {
    size_t memmap_size;
    uintptr_t memmap;
} __attribute__((packed)) memmap_t;

typedef struct {
    size_t ramfs_size;
    uintptr_t ramfs_addr;
} __attribute__((packed)) ramfs_t;

typedef struct {
    uint32_t magic;
    uint32_t version;

    memmap_t memmap;

    ramfs_t ramfs;

} __attribute__((packed)) boot_info;

#endif // __PROTOCOL_H__