#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

// Hornet boot protocol
#define HB_PROTOCOL_VERSION 0x1
#define HB_PROTOCOL_MAGIC   0x4842424F // HBOOT in ASCII

typedef struct {
    size_t memmap_size;
    uintptr_t memmap;
} __attribute__((packed)) memmap_t;

typedef struct {
    size_t ramfs_size;
    uintptr_t ramfs_addr;
} __attribute__((packed)) ramfs_t;

typedef struct {
    uint64_t base;            // 4 bytes
    uint32_t width;           // 4 bytes
    uint32_t height;          // 4 bytes
    uint32_t pitch;           // 4 bytes
    uint16_t bpp;             // 2 bytes
    uint8_t red_mask_size;    // 1 byte
    uint8_t red_mask_shift;   // 1 byte
    uint8_t green_mask_size;  // 1 byte
    uint8_t green_mask_shift; // 1 byte
    uint8_t blue_mask_size;   // 1 byte
    uint8_t blue_mask_shift;  // 1 byte
    uint8_t reserved[8];      // 8 bytes
    // Total: 32 bytes
} __attribute__((packed)) framebuffer_t;

typedef struct {
    uint32_t magic;
    uint32_t version;

    memmap_t memmap;

    ramfs_t ramfs;

    framebuffer_t framebuffer;

} __attribute__((packed)) boot_info;

#endif // __PROTOCOL_H__