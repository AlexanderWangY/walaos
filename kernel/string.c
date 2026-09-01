#include <string.h>

void *memset(void *ptr, int value, int count) {
  unsigned char *p = ptr;

  for (int i = 0; i < count; i++) {
    p[i] = (unsigned char)value;
  }
  
  return ptr;
}
