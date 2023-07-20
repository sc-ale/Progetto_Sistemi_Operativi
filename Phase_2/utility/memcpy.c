#include <memcpy.h>

void *memcpy(void *dest, const void *src, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    return dest;
} 