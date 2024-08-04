#include "string.h"

void *memcpy(void *dest, const void *src, size_t n)
{
    char *d = (char *)dest;
    const char *s = (const char *)src;

    for (size_t i = 0; i < n; i++)
    {
        d[i] = s[i];
    }

    return dest;
}

void *memset(void *dest, int val, size_t len)
{
    unsigned char *ptr = dest;
    while (len-- > 0)
    {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}

void memzero(void *ptr, size_t size)
{
    unsigned char *p = (unsigned char *)ptr;
    while (size--)
    {
        *p++ = 0;
    }
}
void *memmove(void *dest, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dest;
    const unsigned char *s = (const unsigned char *)src;

    if (d == s)
    {
        return dest;
    }

    if (d < s)
    {
        while (n--)
        {
            *d++ = *s++;
        }
    }
    else
    {
        d += n;
        s += n;
        while (n--)
        {
            *(--d) = *(--s);
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
    const unsigned char *p1 = (const unsigned char *)s1;
    const unsigned char *p2 = (const unsigned char *)s2;

    for (size_t i = 0; i < n; i++)
    {
        if (p1[i] != p2[i])
        {
            return p1[i] - p2[i];
        }
    }
    return 0;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    const char *s = src;
    while ((*d++ = *s++))
        ;
    return dest;
}

char *strncpy(char *dest, const char *src, size_t n)
{
    char *d = dest;
    const char *s = src;
    size_t i = 0;
    while (i < n && (*d++ = *s++))
        i++;
    if (i == n)
        *d = '\0';
    return dest;
}

size_t strlen(const char *str)
{
    size_t len = 0;
    while (str[len])
        len++;
    return len;
}

char *strchr(const char *str, int c)
{
    while (*str != '\0' && *str != (char)c)
        str++;
    return (*str == (char)c) ? (char *)str : NULL;
}

int strcmp(const char *str1, const char *str2)
{
    while (*str1 && (*str1 == *str2))
    {
        str1++;
        str2++;
    }
    return *(const unsigned char *)str1 - *(const unsigned char *)str2;
}

char *strstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    if (needle_len == 0)
    {
        return (char *)haystack;
    }

    while (*haystack)
    {
        if (strncmp(haystack, needle, needle_len) == 0)
        {
            return (char *)haystack;
        }
        haystack++;
    }

    return NULL;
}

size_t strspn(const char *str, const char *accept)
{
    const char *p;
    size_t count = 0;

    while (*str != '\0')
    {
        for (p = accept; *p != '\0'; p++)
        {
            if (*str == *p)
            {
                count++;
                break;
            }
        }
        if (*p == '\0')
        {
            break;
        }
        str++;
    }

    return count;
}

int strncmp(const char *str1, const char *str2, size_t n)
{
    for (size_t i = 0; i < n; i++)
    {
        if (str1[i] != str2[i])
        {
            return (int)(unsigned char)str1[i] - (int)(unsigned char)str2[i];
        }

        if (str1[i] == '\0' || str2[i] == '\0')
        {
            break;
        }
    }
    return 0;
}

char *strtok(char *str, const char *delim)
{
    static char *next_token = NULL;

    if (str != NULL)
    {
        next_token = str;
    }
    else if (next_token == NULL)
    {
        return NULL;
    }

    next_token += strspn(next_token, delim);

    if (*next_token == '\0')
    {
        next_token = NULL;
        return NULL;
    }

    char *token_start = next_token;

    next_token += strcspn(next_token, delim);

    if (*next_token != '\0')
    {
        *next_token = '\0';
        next_token++;
    }
    else
    {
        next_token = NULL;
    }

    return token_start;
}

size_t strcspn(const char *str, const char *delim)
{
    const char *s;
    size_t count = 0;

    for (; *str != '\0'; str++)
    {
        for (s = delim; *s != '\0'; s++)
        {
            if (*str == *s)
            {
                return count;
            }
        }
        count++;
    }

    return count;
}


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