extern void printf(const char *, ...);
extern __attribute__((noreturn)) void exit(void);

void main(void)
{
    printf("User task\n");
    exit();
}