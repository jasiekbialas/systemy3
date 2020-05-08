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
    int parent_index = mp -> mp_parent;
    if(parent_index == INIT_PROC_NR) {
        return EACCES;
    }

    struct mproc *p_mp;
    p_mp = &mproc[mp->mp_parent];

    if (p_mp -> mp_flags & WAITING) {
       return EPERM; 
    }
    
    mp -> mp_parent = p_mp -> mp_parent;

    return 0;
}