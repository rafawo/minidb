#include <stdio.h>

int main() {
  printf("{float: ('double', %d), ", (int)sizeof(double));
  printf("str: ('char', %d), ", (int)sizeof(char));
  printf("int: ('int', %d)}", (int)sizeof(int));
  return 0;
}
