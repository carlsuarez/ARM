#include <user/lib/printf.h>
#include <user/lib/task.h>

int main(void)
{
    int a, b;
    a = 50;
    b = 20;
    a = a + b;
    printf("User task\n");
    exit();
}