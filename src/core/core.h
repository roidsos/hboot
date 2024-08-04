#ifndef __CORE_H__
#define __CORE_H__

#include <stdint.h>
#include <stddef.h>

typedef uint8_t HB_STATUS;
typedef size_t HB_SIZE;

#define HB_SUCESS 1
#define HB_FAIL 0

#if TARGET == x86_64-pc
#include "arch/x86_64-pc/uefi.h"
#endif

#endif // __CORE_H__