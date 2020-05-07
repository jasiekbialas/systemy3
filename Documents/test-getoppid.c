#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    pid_t my_pid = getpid();
    printf("my pid: %d\n", my_pid);
    printf("parent pid: %d\n", getppid());
    printf("oparent pid: %d\n", getoppid(my_pid));
}