#include <endian.h>
#include <stdio.h>

int main() {
  printf("Little endian %d\n", __BYTE_ORDER == __LITTLE_ENDIAN);
  printf("Big endian %d\n", __BYTE_ORDER == __BIG_ENDIAN);
  return 0;
}
