#ifndef __UEFI_H__
#define __UEFI_H__

#define _X64
#include <efi/efi.h>
typedef EFI_INTN INTN;
#include <efi/efilib.h>

extern EFI_HANDLE IH;
extern EFI_SYSTEM_TABLE *ST; 

typedef EFI_FILE_PROTOCOL HB_FILE;
#endif // __UEFI_H__