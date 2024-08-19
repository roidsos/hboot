#include "log.h"
#include "arch/x86_64-pc/uefi.h"
#include <efi/efi.h>

void log_info(CHAR16* string) {
    ST->ConOut->OutputString(ST->ConOut, L"[EFI-LOG-INFO]: ");
    ST->ConOut->OutputString(ST->ConOut, string);
}

void log_warn(CHAR16* string) {
    ST->ConOut->OutputString(ST->ConOut, L"[EFI-LOG-WARN]: ");
    ST->ConOut->OutputString(ST->ConOut, string);
}

void log_error(CHAR16* string) {
    ST->ConOut->OutputString(ST->ConOut, L"[EFI-LOG-ERROR]: ");
    ST->ConOut->OutputString(ST->ConOut, string);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
void log_debug(CHAR16* string) {
#ifdef UEFI_LOG_DEBUG
    ST->ConOut->OutputString(ST->ConOut, L"[EFI-LOG-DEBUG]: ");
    ST->ConOut->OutputString(ST->ConOut, string);
#endif // UEFI_LOG_DEBUG
}
#pragma clang diagnostic pop