#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <minix/rs.h>

int main(int argc, char** argv)
{
        message m;
        endpoint_t pm_ep;
        pid_t my_pid = getpid();
        printf("my pid: %d\n", my_pid);
        printf("parent pid: %d\n", getppid());
        m.m1_i1 = my_pid;
        minix_rs_lookup("pm", &pm_ep);
        _syscall(pm_ep, PM_GETOPPID, &m);
        printf("original parent pid: %d\n", m.m1_i1);
        return 0;
}