#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <minix/rs.h>

int main(int argc, char** argv)
{
        message m;
        endpoint_t ipc_ep;
        minix_rs_lookup("pm", &ipc_ep);
        printf("start\n");


        for(int i = 1; i < 301; i++) {
            printf("pid: %d", i);
            m.m1_i1 = i;

            int r =_syscall(ipc_ep, PM_GIVEKUDOS, &m);
            printf("      r: %d        k: %d\n", r, m.m1_i1);

        }

        return 0;
}