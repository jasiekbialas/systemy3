#include "pm.h"
#include "mproc.h"

int do_getoppid(void) {
    pid_t pid = m_in.m1_i1;
    struct mproc* proc = find_proc(pid);
    if(proc == NULL) {
        errno = EINVAL;
        return -1;
    }
    
    mp -> mp_reply.m1_i1 = proc -> mp_oparent;
    return 0;
}

int do_changeparent(void) {
    struct mproc *me = mp;
    struct mproc *parent;
}