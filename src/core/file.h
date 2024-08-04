#ifndef __FILE_H__
#define __FILE_H__

#include <core/core.h>

HB_STATUS file_init();

HB_FILE* file_open(CHAR16 *path, int mode);
HB_STATUS file_close(HB_FILE *file);

HB_STATUS file_read(HB_FILE *file, void *buffer, size_t size);
HB_STATUS file_write(HB_FILE *file, void *buffer, size_t size);
HB_STATUS file_seek(HB_FILE *file, int offset, int origin);

HB_SIZE file_size(HB_FILE *file);

#endif // __FILE_H__    