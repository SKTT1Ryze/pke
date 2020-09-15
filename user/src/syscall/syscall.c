/// syscall implement

#include "syscall.h"
#include <stdio.h>

int syscall(int syscall_id) {
    int ret = 0;
    printf("syscall id: %d\n",syscall_id);
    asm volatile(
        "lw t0, %1 \n\t"
        "add x17, t0, x0 \n\t"
        "ecall \n\t"
        "sd t0, %0"
        :"=m"(ret)
        :"m"(syscall_id)
        :"memory"
    );
    return ret;    
}

