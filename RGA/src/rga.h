#ifndef __RGA_H__
#define __RGA_H__

#include <stdlib.h>
#include "types.h"
#include "MT.h"
#define SEED 0x54554154 // "TUAT"

typedef struct _evalute {
    double80 (*evaluate)(double80 *);
    bool is_minimize; // true for minimization, false for maximization
    int dimension;
    double min_value;
    double max_value;
} evaluate;

typedef struct _property {
    int individuals;
    double crossover_rate;
    double mutation_rate;
} property;

struct population {
    double80 *x;
    double80 fitness;
};

typedef struct _RGA {
    property *properties;
    evaluate *eval;
    // Add other fields as necessary for the RGA structure
    struct population **populations; // Example field for population representation
    bool sorted; // Flag to indicate if populations are sorted

    void (*free)(struct _RGA *);
    void (*update)(struct _RGA *);
    struct population *(*greatest)(struct _RGA *);

} RGA;

void init_populations(RGA *self);
void free_rga(RGA *self);

struct population *greatest_individual(RGA *self);
void update_rga(RGA *self);

#define RGA_INIT(rga, prop_p, eval_p)                                                              \
    do {                                                                                           \
        (rga)->properties = (prop_p);                                                              \
        (rga)->eval       = (eval_p);                                                              \
        (rga)->populations = NULL;                                                                   \
        (rga)->sorted     = false;                                                                   \
        (rga)->free       = free_rga;                                                              \
        (rga)->update     = update_rga;                                                            \
        (rga)->greatest   = greatest_individual;                                                   \
        init_populations(rga);                                                                     \
    } while (0)

#endif /* __RGA_H__ */