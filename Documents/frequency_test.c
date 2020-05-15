#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>


double calculate_speed(int num_of_operations, double* cpu_time_used) {
    long long start = (long long)time(NULL);

    int a = 435234545;
    for (int i = 0; i < num_of_operations*8; i++) {
        for (int j = 0; j < 1000; j++)
            if (a % 2 == 0)
                a /= 2;
            else
                a = 3 * a + 1;
    }

    long long end = (long long)time(NULL);

    *cpu_time_used = ((double)(end - start));

    return ((*cpu_time_used)*1000 * 1000) / num_of_operations;
}

const int base_number = 1000 * 500;

int get_priority(int kudos_number) {
    /*
    [0, 10)	3
    [10, 25)	2
    [25, 50)	1
    [50, +∞)
    */
    int bariers[] = {10, 25, 50};

    int to_return = 3;

    for (int i = 0; i < 3; i++) {
        if (kudos_number >= bariers[i])
            to_return -= 1;
        else
            break;
    }

    return to_return;
}

void error(int nr) {
    printf("\n\n\nERROR %d\n\n\n", nr);
    fflush(stdout);
    exit(1);
}

#define NUM_OF_PRIORITIES 4

void give_workers_kudos(int* pids_of_workers) {
    int kudos_to_give[] = {50, 25, 10, 0};
    for (int i = 0; i < NUM_OF_PRIORITIES; i++) {
        int given_kudos = 0;
        for (int k = 0; k < kudos_to_give[i]; k++) {
            int current_priority = givekudos(pids_of_workers[i]);
            given_kudos += 1;
            if (current_priority != get_priority(given_kudos)) {
                printf(
                    "nie zgodzila sie priority dla kudos = %d, current to %d",
                    given_kudos, current_priority);
                error(45);
            }
        }
    }
}

void wait_for_children() {
    fflush(stdout);
    for (int i = 0; i < NUM_OF_PRIORITIES + 1; i++) {
        wait(NULL);
        fflush(stdout);
    }
}

int main(int argc, char** argv) {
    int pids_of_workers[NUM_OF_PRIORITIES];
    int our_number;
    double our_speed;

    printf("test ma sprawdzic, czy procesy o roznych priorytetach dzialaja\n z innymi predkosciami. chcemy by wypisane czasy\n na operacje mialy odpowiednie stosunki bliskie 2, zas wypisane cale czasy dzialania procesow zblizone\n");

    fflush(stdout);
    // create workers
    for (int i = 0; i < NUM_OF_PRIORITIES; i++) {
        int fork_code = fork();
        switch (fork_code) {
            case -1:

                perror("fork");
                exit(1);
                break;
            case 0:
                // worker
                // musi pospac, by w tym czasie gdy spi jego priorytet wzrastal
                sleep(2);
                our_number = (base_number << (NUM_OF_PRIORITIES - i - 1));
                double time_used = 0.;
                our_speed = calculate_speed(our_number, &time_used);
                printf("priority %d czas na operacje %lf, caly czas %lf\n", i, our_speed, time_used);
                fflush(stdout);
                return 0;
            default:
                pids_of_workers[i] = fork_code;
        }
    }

    // create kudos giver

    int fork_code = fork();

    switch (fork_code) {
        case -1:
            perror("fork");
            exit(1);
            break;
        case 0:
            give_workers_kudos(pids_of_workers);
            fflush(stdout);
            return 0;
        default:
            wait_for_children();
    }

    fflush(stdout);
    return 0;
}
