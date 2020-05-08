#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv)
{
    printf("A pid is: %d\n\n", getpid());

    if (fork() == 0) {
        printf("B pid is: %d\n", getpid()); 
        printf("B parent pid is: %d\n", getppid()); 
        printf("B original parent pid is: %d\n\n", getoppid(getpid())); 

        if (fork() == 0) {
            printf("C pid is: %d\n", getpid()); 
            printf("C parent pid is: %d\n", getppid()); 
            printf("C original parent pid is: %d\n\n", getoppid(getpid())); 

            sleep(3);

            int change = changeparent();

            printf("C errno: %d\n", errno);
            printf("C parent pid is: %d\n", getppid()); 
            printf("C original parent pid is: %d\n\n", getoppid(getpid())); 
  
        } else {
            
            int change = changeparent();
            printf("B errno: %d\n", errno);
            printf("B parent pid is: %d\n", getppid()); 
            printf("B original parent pid is: %d\n", getoppid(getpid())); 
        }
  
    } else {
        wait(NULL);
    }

    return 0;
}