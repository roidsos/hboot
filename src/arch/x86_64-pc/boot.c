#include <efi/efi.h>

EFI_STATUS boot_entry(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello from Hboot!\r\n");
    for (;;)
        ;
}