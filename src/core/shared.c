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
#include <efi/efi_types.h>
#include "log.h"
#include <core/util/config.h>
#include <core/util/int.h>

char* kernel_path = NULL;
char* kernel_cmdline = NULL;
char* ramfs_path = NULL;
char *graphics_mode = NULL;

extern int shared_end;
void shared_main()
{
    EFI_STATUS s;

    if(load_config() != HB_SUCCESS) {
        log_error(L"Failed to load config");
        return;
    }

    CHAR16 wide[1024];

    mbstowcs(wide, kernel_path, 1024);

    CHAR16 kernel_path_wide[1024];
    mbstowcs(kernel_path_wide, kernel_path, 1024);

    ST->ConOut->OutputString(ST->ConOut, L"Loading kernel: ");
    ST->ConOut->OutputString(ST->ConOut, kernel_path_wide);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    CHAR16 cmdline[1024];
    mbstowcs(cmdline, kernel_cmdline, 1024);
    ST->ConOut->OutputString(ST->ConOut, L"Kernel command line: ");
    ST->ConOut->OutputString(ST->ConOut, cmdline);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    CHAR16 wide_graphics_mode[1024];
    mbstowcs(wide_graphics_mode, graphics_mode, 1024);
    ST->ConOut->OutputString(ST->ConOut, L"Graphics mode: ");
    ST->ConOut->OutputString(ST->ConOut, wide_graphics_mode);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");

    CHAR16 wide_ramfs_path[1024];
    mbstowcs(wide_ramfs_path, ramfs_path, 1024);
    ST->ConOut->OutputString(ST->ConOut, L"Ramfs path: ");
    ST->ConOut->OutputString(ST->ConOut, wide_ramfs_path);
    ST->ConOut->OutputString(ST->ConOut, L"\r\n");


    HB_FILE *kernel = file_open(wide, EFI_FILE_MODE_READ);
    if(kernel == NULL) {
        log_error(L"Failed to open kernel file");
        return;
    }
    EFI_UINTN size = file_size(kernel);
    void* kernelbuf = malloc(size);
    if(file_read(kernel, kernelbuf, size) != HB_SUCCESS) {
        log_error(L"Failed to read kernel file");
        return;
    }
    file_close(kernel);

    Elf64_Ehdr* h = (Elf64_Ehdr*)kernelbuf;
    if(!validate_elf(kernelbuf)) {
        log_error(L"Invalid kernel file");
        return;
    }

    boot_info* bootinfo;
    s = ST->BootServices->AllocatePool(EfiLoaderData, sizeof(boot_info), (void**)&bootinfo);
    if(s != EFI_SUCCESS) {
        log_error(L"Failed to allocate bootinfo");
        return;
    }
    bootinfo->magic = HB_PROTOCOL_MAGIC;
    bootinfo->version = HB_PROTOCOL_VERSION;

    if(ramfs_path != NULL) {
        CHAR16 ramfs_wide[1024];
        mbstowcs(ramfs_wide, ramfs_path, 1024);
        HB_FILE *ramfs = file_open(ramfs_wide, EFI_FILE_MODE_READ);
        if(ramfs == NULL) {
            log_error(L"Failed to open ramfs file");
            return;
        }
        bootinfo->ramfs.ramfs_size = file_size(ramfs);
        s = ST->BootServices->AllocatePool(EfiLoaderData, bootinfo->ramfs.ramfs_size, (void**)bootinfo->ramfs.ramfs_addr);
        if(s != EFI_SUCCESS) {
            log_error(L"Failed to allocate ramfs");
            return;
        }
        file_read(ramfs, (void*)bootinfo->ramfs.ramfs_addr, bootinfo->ramfs.ramfs_size);
        file_close(ramfs);

    } else {
        bootinfo->ramfs.ramfs_addr = 0;
        bootinfo->ramfs.ramfs_addr = 0;
    }

    EFI_MEMORY_DESCRIPTOR* map = NULL;
    EFI_MEMORY_DESCRIPTOR tmpmap[1];
    EFI_UINTN   map_size = sizeof(tmpmap);
    EFI_UINTN   map_key = 0;
    EFI_UINTN   map_desc_size = 0;
    EFI_UINT32  map_desc_version = 0;

    framebuffer_t fb;

    EFI_STATUS fb_status;
    EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info;
    EFI_UINTN info_size;

    fb_status = ST->BootServices->LocateProtocol(&gop_guid, NULL, (void**)&gop);
    if(EFI_ERROR(fb_status)) {
        log_error(L"Failed to locate GOP");
    }

    fb_status = gop->QueryMode(gop, gop->Mode == NULL ? 0 : gop->Mode->Mode, &info_size, &info);
    if(fb_status == (EFI_UINTN)EFI_NOT_STARTED) {
        log_error(L"GOP not started");
    }

    if (EFI_ERROR(fb_status)) {
        log_error(L"Failed to query GOP mode");
    }

    switch(gop->Mode->Info->PixelFormat) {
        case PixelBlueGreenRedReserved8BitPerColor:
            fb.bpp = 32;
            fb.red_mask_size = 8;
            fb.red_mask_shift = 16;
            fb.green_mask_size = 8;
            fb.green_mask_shift = 8;
            fb.blue_mask_size = 8;
            fb.blue_mask_shift = 0;
            break;
        case PixelRedGreenBlueReserved8BitPerColor:
            fb.bpp = 32;
            fb.red_mask_size = 8;
            fb.red_mask_shift = 0;
            fb.green_mask_size = 8;
            fb.green_mask_shift = 8;
            fb.blue_mask_size = 8;
            fb.blue_mask_shift = 16;
            break;
        default:
            break;
    }

    fb.base = gop->Mode->FrameBufferBase;
    fb.width = gop->Mode->Info->HorizontalResolution;
    fb.height = gop->Mode->Info->VerticalResolution;
    fb.pitch = gop->Mode->Info->PixelsPerScanLine * 4;
    for (int i = 0; i < 8; i++) {
        fb.reserved[i] = 0;
    }

    bootinfo->framebuffer = fb;

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
        log_error(L"Failed to allocate memory map");
    }

    retry:
    EFI_UINTN retries = 4;
    s = ST->BootServices->GetMemoryMap(&map_size, map, &map_key, &map_desc_size, &map_desc_version);
    if(s != EFI_SUCCESS) {
        retries--;
        if(retries == 0) 
            log_error(L"Failed to get memory map"); 
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
            log_error(L"Failed to exit boot services");
        CHAR16 err[5];
        err[0] = '0' + s;
        err[1] = 'd';
        err[2] = '\r';
        err[3] = '\n';
        err[4] = '\0';
        ST->ConOut->OutputString(ST->ConOut, err);
        goto retry;
    }
    bootinfo->memmap.memmap_size = map_size;
    bootinfo->memmap.memmap = (uintptr_t)map;

    uintptr_t *pml4 = NULL;
    ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, 1, (void*)&pml4);
    for (Elf64_Phdr* phdr = get_phdrs(kernelbuf);
        (uint64_t)phdr < (uint64_t)kernelbuf + size;
        phdr = (Elf64_Phdr*)((uint64_t)phdr + sizeof(Elf64_Phdr))) {

        if (phdr->p_type == PT_LOAD) {
            uint64_t pages = (phdr->p_memsz + 0xfff) / 0x1000;
            ST->BootServices->AllocatePages(AllocateAddress, EfiLoaderCode, pages, (void*)&phdr->p_paddr);

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
    
    int graphics_mode_int = stringToInt(graphics_mode);

    switch (graphics_mode_int) {
        case 0: 
            gop->SetMode(gop, 0);
            break;
        case 1:
            gop->SetMode(gop, 1);
            break;
        case 2:
            gop->SetMode(gop, 2);
            break;
        case 3:
            gop->SetMode(gop, 3);
            break;
        case 4:
            gop->SetMode(gop, 4);
            break;
        case 5:
            gop->SetMode(gop, 5);
            break;
        case 6:
            gop->SetMode(gop, 6);
            break;
        case 7:
            gop->SetMode(gop, 7);
            break;
        case 8: 
            gop->SetMode(gop, 8);
            break;
        default:
            gop->SetMode(gop, 1);
            break;
    }

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
}
int shared_end = 0;