#include <core/core.h>
#include <core/libc/file.h>
EFI_HANDLE IH;
EFI_SYSTEM_TABLE *ST;

void shared_main();

EFI_STATUS boot_entry(EFI_HANDLE _ih, EFI_SYSTEM_TABLE *_st)
{
    IH = _ih;
    ST = _st;

    if(file_init() != HB_SUCESS){
        _st->ConOut->OutputString(_st->ConOut, L"Failed to init the file system!\r\n");
        for (;;)
            ;
    }

    _st->ConOut->OutputString(_st->ConOut, L"Booting RoidsOS...\r\n");

    shared_main();

    return EFI_SUCCESS;
}