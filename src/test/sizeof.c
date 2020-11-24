#include <stdio.h>

int main() {
    printf("{float: ('double', %d), ", sizeof(double));
    printf("str: ('char', %d), ", sizeof(char));
    printf("int: ('int', %d)}", sizeof(int));
    return 0;
}
