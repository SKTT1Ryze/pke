/// test file 1

#include <stdio.h>
#include "../syscall/syscall.h"

int main() {
    printf("Running test [1]\n");
    int syscall_id = 80;
    int ret = syscall(syscall_id);
    printf("[%d] syscall return: %d\n",syscall_id, ret);
    return ret;
}