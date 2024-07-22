
#ifndef PUB_H
#define PUB_H

#define _X64
#include <efi.h>

// Temporary fix.
typedef EFI_INTN INTN;
#include <efilib.h>

extern EFI_HANDLE *imageHandle;
extern EFI_SYSTEM_TABLE *systemTable;

extern EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stdout;
extern EFI_SIMPLE_TEXT_INPUT_PROTOCOL *stdin;
extern EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stderr;

#endif // PUB_H