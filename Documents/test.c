#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    for(int i = 1; i< 300; i++) {
        printf("i: %d  -> k: %d\n", i, givekudos(i));
    }
}