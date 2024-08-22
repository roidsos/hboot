#include "int.h"

// basically atoi
int stringToInt(const char *str) {
    int result = 0;     
    int sign = 1;        
    
    if (*str == '\0') {
        return 0; 
    }
    
    if (*str == '-') {
        sign = -1;
        str++; 
    } else if (*str == '+') {
        str++; 
    }
    
    while (*str) {
        if (*str < '0' || *str > '9') {
            return 0; 
        }
        
        result = result * 10 + (*str - '0');
        str++;
    }
    
    return result * sign;
}

int intToStringLength(int num) {
    int length = 0;

    if (num == 0) {
        return 1;
    }
    
    if (num < 0) {
        length++;
        num = -num;
    }

    while (num > 0) {
        length++;
        num /= 10;
    }
    
    return length;
}