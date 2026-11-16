#ifndef __PRINT_H__
#define __PRINT_H__
#include <stdio.h>

#define PRINT_DEBUG 1
#define PRINT_INFO 2
#define PRINT_WARN 3
#define PRINT_ERR 4
#define PRINT_CRIT 5

extern int print_level;
static FILE *print_file = NULL; // Default to standard error

inline void set_print_level(int level)
{
    if (level >= PRINT_DEBUG && level <= PRINT_CRIT)
        print_level = level;
}

#define set_print_file(file)                                                                       \
    do {                                                                                           \
        print_file = file;                                                                         \
    } while (0)

#define PRINT(fmt, ...)                                                                               \
    do {                                                                                           \
        if (print_file == NULL)                                                                    \
            print_file = stderr;                                                                   \
        fprintf(print_file, "" fmt "\n", ##__VA_ARGS__);                                   \
    } while (0)

#define PR_DEBUG(fmt, ...)                                                                         \
    do {                                                                                           \
        if (PRINT_DEBUG >= print_level)                                                            \
            PRINT("[DEBUG] " fmt, ##__VA_ARGS__);                                                  \
    } while (0)
#define PR_INFO(fmt, ...)                                                                          \
    do {                                                                                           \
        if (PRINT_INFO >= print_level)                                                             \
            PRINT("[INFO] " fmt, ##__VA_ARGS__);                                                  \
    } while (0)
#define PR_WARN(fmt, ...)                                                                          \
    do {                                                                                           \
        if (PRINT_WARN >= print_level)                                                             \
            PRINT("[WARN] " fmt, ##__VA_ARGS__);                                                  \
    } while (0)
#define PR_ERR(fmt, ...)                                                                           \
    do {                                                                                           \
        if (PRINT_ERR >= print_level)                                                              \
            PRINT("[ERROR] " fmt, ##__VA_ARGS__);                                          \
    } while (0)
#define PR_CRIT(fmt, ...)                                                                          \
    do {                                                                                           \
        if (PRINT_CRIT >= print_level)                                                             \
            PRINT("[CRIT] " fmt, ##__VA_ARGS__);                                       \
    } while (0)

#endif /* __PRINT_H__ */