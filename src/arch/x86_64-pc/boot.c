#include <core/core.h>
#include <core/libc/file.h>
#include <efi/efi.h>

EFI_HANDLE IH;
EFI_SYSTEM_TABLE *ST;

void shared_main();

EFI_STATUS boot_entry(EFI_HANDLE _ih, EFI_SYSTEM_TABLE *_st)
{
    IH = _ih;
    ST = _st;

    if(file_init() != HB_SUCCESS){
        _st->ConOut->OutputString(_st->ConOut, L"Failed to init the file system!\r\n");
        for (;;)
            ;
    }

    _st->ConOut->ClearScreen(_st->ConOut);

    _st->ConOut->OutputString(_st->ConOut, L"Booting RoidsOS...\r\n");

    shared_main();

    for (;;)
    {
        asm("hlt");
    }
    
    return EFI_SUCCESS;
}