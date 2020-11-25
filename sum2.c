#include <stdio.h>

int add_nums(int a, int b)
{
    return a + b;
}

int main()
{
    printf("%d\n", add_nums(2, 3));
    printf("%d\n", add_nums(10, -1));
    printf("%d\n", add_nums(-4, -5));
}
