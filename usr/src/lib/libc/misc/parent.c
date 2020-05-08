#include <lib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <minix/rs.h>

int get_pm_endpt(endpoint_t *pt)
{
        return minix_rs_lookup("pm", pt);
}

pid_t getoppid(pid_t pid)
{
        endpoint_t pm_pt;
        message m;
        if (get_pm_endpt(&pm_pt) != 0)
        {
                errno = ENOSYS;
                return -1;
        }
        m.m1_i1 = pid;
        if(_syscall(pm_pt, PM_GETOPPID, &m)) {
            return -1;
        }
        return m.m1_i1;
}

int changeparent(void) {
        endpoint_t pm_pt;
        message m;
        if (get_pm_endpt(&pm_pt) != 0)
        {
                errno = ENOSYS;
                return -1;
        }

        return _syscall(pm_pt, PM_CHANGE_PARENT, &m);
}