#include <stdio.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

int main() {
  printf("FLOAT MAX: %f\n FLOAT MIN: %f\n", FLT_MAX, FLT_MIN);
  printf("DOUBLE MAX: %f\n DOUBLE MIN: %f\n", DBL_MAX, DBL_MIN);
  printf("INT MAX: %d\n INT MIN: %d\n", INT_MAX, INT_MIN);
  printf("CHAR MAX: %d\n CHAR MIN: %d\n", CHAR_MAX, CHAR_MIN);

  printf("char: %d\n", sizeof(char));
  printf("int: %d\n", sizeof(int));
  printf("float: %d\n", sizeof(float));
  printf("double: %d\n", sizeof(double));

  float x = 340282346638528859811704183484516925440.000000;

  printf("x = %f\n", x);
  printf("y = %f\n", -x);
}
