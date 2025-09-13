#include <common/abort.h>
__attribute__((noreturn)) void abort(void)
{
    while (1)
        ;
}