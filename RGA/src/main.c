#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define _POSIX_C_SOURCE 199309L

#define DIMENSION 50
#define POPULATION_VALUE_MIN -5.12
#define POPULATION_VALUE_MAX 5.12

#define LOOP 100000

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "print.h"
#include "rga.h"

int print_level = PRINT_INFO;

double80 Rastrigin50(double80 *x)
{
    double80 sum = 0.0;
    for (int i = 0; i < DIMENSION; i++)
        sum += x[i] * x[i] - 10.0 * cos(2.0 * M_PI * x[i] )+ 10.0;
    return sum;
}



int main(int argc, char *argv[])
{
    int individuals       = 0;
    double crossover_rate = 0.0;
    double mutation_rate  = 0.0;

    if (argc < 4) {
        PR_ERR("Usage: %s <individuals> <crossover_rate> <mutation_rate>", argv[0]);
        return EXIT_FAILURE;
    }
    individuals    = atoi(argv[1]);
    crossover_rate = atof(argv[2]);
    mutation_rate  = atof(argv[3]);

    if (individuals <= 0 || crossover_rate < 0.0 || crossover_rate > 1.0 || mutation_rate < 0.0 ||
        mutation_rate > 1.0) {
        PR_ERR("Invalid input: individuals must be > 0 and crossover_rate and mutation_rate must "
               "be in [0, 1]");
        return EXIT_FAILURE;
    }
    if (individuals < 3)
        PR_ERR("Error: Number of individuals must be at least 3.");

    if (individuals > 1000)
        PR_WARN("Warning: High number of individuals may lead to high memory usage.");

    RGA rga;
    property prop = {.individuals    = individuals,
                     .crossover_rate = crossover_rate,
                     .mutation_rate  = mutation_rate};
    evaluate eval = {.evaluate    = Rastrigin50,
                     .is_minimize = true,
                     .dimension   = DIMENSION,
                     .min_value   = POPULATION_VALUE_MIN,
                     .max_value   = POPULATION_VALUE_MAX};

    RGA_INIT(&rga, &prop, &eval);

    PR_INFO("RGA initialized with %d individuals and mutation rate %.2f",
            rga.properties->individuals, rga.properties->mutation_rate);

    double80 bests[LOOP];
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < LOOP; i++) {
        struct population *best = rga.greatest(&rga);
        bests[i]                = best->fitness;
        rga.update(&rga);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    double80 elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    PR_INFO("RGA completed in %.5Lf seconds", elapsed_time);

    int fd = open("fitness.csv", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        PR_ERR("Failed to open fitness.csv for writing.");
        return EXIT_FAILURE;
    }
    write(fd, "gen,fitness\n", 12);
    char buffer[64];
    for (int i = 0; i < LOOP; i++) {
        snprintf(buffer, sizeof(buffer), "%d,%.18Lf\n", i + 1, bests[i]);
        write(fd, buffer, strlen(buffer));
    }
    close(fd);
    rga.free(&rga);
    return EXIT_SUCCESS;
}
