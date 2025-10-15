#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdatomic.h>
#include <threads.h>
#include <complex.h>
#include <string.h>

// Shared configuration constants
#define _USE_MATH_DEFINES
#define EPSILON 1e-3
#define EPS_SQR 1e-6
#define MAX_ITERATIONS 128
#define BAILOUT_MAG 1e10
#define BAILOUT_NORM_SQ 1e-6       // Squared norm for closeness to origin
#define MAX_COLOR_VALUE 255
#define BLOCKSIZE 10000

// Data type typedefs
typedef uint8_t TYPE_ATTR; 
typedef uint8_t TYPE_CONV;

typedef enum { FREE, FILLING, READY, WRITING } BlockState;
//struct defintions

typedef struct {
    TYPE_ATTR *att_data;
    TYPE_CONV *con_data;
    size_t size;
    atomic_int state;
    int block_index;
} BlockBuffer;

typedef struct {
    int thread_id;
} ThreadArg;

// Color palette definitions (example colors for roots)
#define NUM_COLORS_MAX 29
#define STRING_LENGTH 12
extern char *attractor_strings[NUM_COLORS_MAX];
extern char *convergence_strings[MAX_ITERATIONS];

// atomics
extern atomic_int block_counter;
extern atomic_int write_idx;
extern int total_blocks;

extern mtx_t lock;
extern cnd_t cond;

//arguments
extern int n_threads;
extern int l;
extern int d;

extern double newton_c1;
extern double newton_c2;
extern double delta_arg;
extern double delta_arg_inv;

//blocks
extern _Complex double *roots;
  
extern BlockBuffer *buffers;
extern int n_buffers;

//filenames
extern char att_file[40];   // buffer large enough to hold the formatted string
extern char conv_file[40];


#endif // CONFIG_H
