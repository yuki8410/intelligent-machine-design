#include "rga.h"
#include "print.h"
#include <stdlib.h>

bool sorted = false;

double80 rand_individual(double min_value, double max_value)
{
    return min_value + genrand_real1() * (max_value - min_value);
}

int compute_fitness_greater(const void *a, const void *b)
{
    struct population **pp_a = (struct population **)a;
    struct population **pp_b = (struct population **)b;

    const struct population *pop_a = *pp_a;
    const struct population *pop_b = *pp_b;

    if (pop_a->fitness > pop_b->fitness) {
        return -1; //  pop_a is better
    }
    if (pop_a->fitness < pop_b->fitness) {
        return 1;  // pop_b is better
    }
    return 0;
}

int compute_fitness_less(const void *a, const void *b)
{
    struct population **pp_a = (struct population **)a;
    struct population **pp_b = (struct population **)b;

    const struct population *pop_a = *pp_a;
    const struct population *pop_b = *pp_b;

    if (pop_a->fitness < pop_b->fitness) {
        return -1; // pop_a is better
    }
    if (pop_a->fitness > pop_b->fitness) {
        return 1;  // pop_b is better
    }
    return 0;
}
void print_fitnesses(const RGA *self)
{
    if (self == NULL || self->populations == NULL) {
        PR_ERR("RGA structure or populations is NULL.");
        return;
    }
    for (int i = 0; i < self->properties->individuals; i++)
        PR_DEBUG("Fitness[%d]: %Lf", i, self->populations[i]->fitness);
}

void sort_population(RGA *self)
{
    if (self == NULL || self->populations == NULL) {
        PR_ERR("RGA structure or populations is NULL.");
        return;
    }

    struct population **populations = self->populations;
    int size                        = self->properties->individuals;
    int (*compare_func)(const void *, const void *) =
        self->eval->is_minimize ? compute_fitness_less : compute_fitness_greater;

    qsort(populations, size, sizeof(struct population *), compare_func);

    self->sorted = true;
}

struct population *greatest_individual(RGA *self)
{
    if (self == NULL || self->populations == NULL) {
        PR_ERR("RGA structure or populations is NULL.");
        return NULL;
    }
    if (!self->sorted)
        sort_population(self);

    return self->populations[0];
}

void update_rga(RGA *self)
{
    if (self == NULL)
        return;

    // Implement the logic to update the RGA population
    // This is a placeholder for the actual implementation
    PR_DEBUG("Updating RGA population...");
    // 1. Selection
    sort_population(self);
    struct population *first = self->populations[0], *second = self->populations[1];
    PR_DEBUG("Selected individuals: %Lf, %Lf", first->fitness, second->fitness);

    // 2. Crossover and Mutation
    for (int i = 2; i < self->properties->individuals; i++) {
        struct population *pop = self->populations[i];
        for (int j = 0; j < self->eval->dimension; j++) {
            double rate = genrand_real1();
            rate -= self->properties->mutation_rate;
            // if mutation occurs
            if (rate < 0.0) {
                pop->x[j] = rand_individual(self->eval->min_value, self->eval->max_value);
                continue;
            }
            // if crossover occurs
            rate -= self->properties->crossover_rate;
            if (rate < 0.0) {

                // Crossover logic (simple average for demonstration)
                double80 min = first->x[j] < second->x[j] ? first->x[j] : second->x[j];
                double80 max = first->x[j] > second->x[j] ? first->x[j] : second->x[j];
                if (min == max)
                    pop->x[j] = min; // Avoid division by zero
                else {
                    double80 range = max - min;
                    pop->x[j]      = min + (genrand_real1() * range);
                }
                continue;
            }
            // No mutation or crossover, retain the parent's original value
            if (genrand_real1() < 0.5)
                pop->x[j] = first->x[j];
            else
                pop->x[j] = second->x[j];
        }
    }
    // 4. Evaluation
    for (int i = 0; i < self->properties->individuals; i++) {
        struct population *pop = self->populations[i];
        pop->fitness           = self->eval->evaluate(pop->x);
    }
    // 5. Sort the population after update
    self->sorted = false; // Reset sorted flag after update
    sort_population(self);
}

void init_populations(RGA *self)
{
    if (self == NULL) {
        PR_ERR("RGA structure is NULL.\n");
        exit(EXIT_FAILURE);
    }
    if (self->populations != NULL) {
        free(self->populations);
        self->populations = NULL;
    }
    PR_DEBUG("Initializing populations with %d individuals...\n", self->properties->individuals);
    self->populations = malloc(self->properties->individuals * sizeof(struct population *));
    if (self->populations == NULL) {
        PR_ERR("Failed to allocate memory for populations.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < self->properties->individuals; i++) {
        struct population *pop = malloc(sizeof(struct population));
        if (pop == NULL) {
            PR_ERR("Failed to allocate memory for population %d.\n", i);
            exit(EXIT_FAILURE);
        }
        pop->x = malloc(self->eval->dimension * sizeof(double80));
        if (pop->x == NULL) {
            PR_ERR("Failed to allocate memory for individuals in population %d.\n", i);
            exit(EXIT_FAILURE);
        }
        pop->fitness         = 0.0;
        self->populations[i] = pop;
    }

    // Initialize populations with random values
    init_genrand(SEED);
    for (int i = 0; i < self->properties->individuals; i++) {
        struct population *pop = self->populations[i];
        for (int j = 0; j < self->eval->dimension; j++)
            pop->x[j] = rand_individual(self->eval->min_value, self->eval->max_value);

        pop->fitness = self->eval->evaluate(pop->x);
        PR_DEBUG("Population %d initialized with fitness: %Lf\n", i, pop->fitness);
    }
}

void free_rga(RGA *self)
{
    if (self == NULL)
        return;
    for (int i = 0; i < self->properties->individuals; i++) {
        free(self->populations[i]->x);
        free(self->populations[i]);
    }
    free(self->populations);
    self->populations = NULL;
    return;
}