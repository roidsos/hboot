#ifndef __LOG_H__
#define __LOG_H__

#include <efi/efi.h>

void log_info(CHAR16* string);
void log_warn(CHAR16* string);
void log_error(CHAR16* string);
void log_debug(CHAR16* string); // Only works if the compiler flag -DUEFI_LOG_DEBUG is set

#endif // __LOG_H__