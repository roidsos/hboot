#include <efi/efi.h>
#include <core/core.h>
#include <core/file.h>

int mbtowc (wchar_t * __pwc, const char *s, size_t n)
{
    wchar_t arg;
    int ret = 1;
    if(!s || !*s) return 0;
    arg = (wchar_t)*s;
    if((*s & 128) != 0) {
        if((*s & 32) == 0 && n > 0) { arg = ((*s & 0x1F)<<6)|(*(s+1) & 0x3F); ret = 2; } else
        if((*s & 16) == 0 && n > 1) { arg = ((*s & 0xF)<<12)|((*(s+1) & 0x3F)<<6)|(*(s+2) & 0x3F); ret = 3; } else
        if((*s & 8) == 0 && n > 2) { arg = ((*s & 0x7)<<18)|((*(s+1) & 0x3F)<<12)|((*(s+2) & 0x3F)<<6)|(*(s+3) & 0x3F); ret = 4; }
        else return -1;
    }
    if(__pwc) *__pwc = arg;
    return ret;
}

size_t mbstowcs (wchar_t *__pwcs, const char *__s, size_t __n)
{
    int r;
    wchar_t *orig = __pwcs;
    if(!__s || !*__s) return 0;
    while(*__s) {
        r = mbtowc(__pwcs, __s, __n - (size_t)(__pwcs - orig));
        if(r < 0) return (size_t)-1;
        __pwcs++;
        __s += r;
    }
    *__pwcs = 0;
    return (size_t)(__pwcs - orig);
}

EFI_HANDLE IH;
EFI_SYSTEM_TABLE *ST;

EFI_STATUS boot_entry(EFI_HANDLE _ih, EFI_SYSTEM_TABLE *_st)
{
    IH = _ih;
    ST = _st;

    _st->ConOut->ClearScreen(_st->ConOut);
    _st->ConOut->OutputString(_st->ConOut, L"Readin' the config:\r\n");
    
    if(file_init() != HB_SUCESS){
        _st->ConOut->OutputString(_st->ConOut, L"Failed to init the file system!\r\n");
        for (;;)
            ;
    }

    HB_FILE* f = file_open(L"\\boot\\hboot.conf", EFI_FILE_MODE_READ);
    
    if(f == NULL){
        _st->ConOut->OutputString(_st->ConOut, L"Failed to open the file!\r\n");
        for (;;)
            ;
    }

    HB_SIZE size = file_size(f);
    //load the contents and display on screen

    char* buf; 

    ST->BootServices->AllocatePool(EfiLoaderData, size, &buf);

    file_read(f, buf, size);
    CHAR16 wide[1024];

    mbstowcs(wide, buf, size);

    _st->ConOut->OutputString(_st->ConOut, wide);

    ST->BootServices->FreePool(buf);
    file_close(f);

    for (;;)
        ;
}