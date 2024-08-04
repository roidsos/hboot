
#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int val, size_t len);
void memzero(void *ptr, size_t size);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
size_t strlen(const char *str);
char *strchr(const char *str, int c);
int strcmp(const char *str1, const char *str2);
char *strstr(const char *haystack, const char *needle);
int strncmp(const char *str1, const char *str2, size_t n);
size_t strspn(const char *str, const char *accept);
size_t strcspn(const char *str, const char *delim);
char *strtok(char *str, const char *delim);

int mbtowc (wchar_t * __pwc, const char *s, size_t n);
size_t mbstowcs (wchar_t *__pwcs, const char *__s, size_t __n);

#endif // STRING_H