#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

int main()
{
    syscall(332);
    printf("system call sys_mycall\n");
    return 0;
}
