#include <core/core.h>
#include <core/libc/file.h>
#include <efi/efi.h>
#include <core/log.h>

EFI_HANDLE IH;
EFI_SYSTEM_TABLE *ST;

void shared_main();

EFI_STATUS boot_entry(EFI_HANDLE _ih, EFI_SYSTEM_TABLE *_st)
{
    IH = _ih;
    ST = _st;

    if(file_init() != HB_SUCCESS){
        log_error(L"Failed to load file system! Reboot system!\r\n");
        for (;;)
            ;
    }

    _st->ConOut->ClearScreen(_st->ConOut);

    log_info(L"Booting Shared Entrypoint...\r\n");

    log_error(L"test error log!\r\n");

    shared_main();

    for (;;)
    {
        asm("hlt");
    }
    
    return EFI_SUCCESS;
}