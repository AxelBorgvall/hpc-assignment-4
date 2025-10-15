
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

#include "../include/complex_utils.h"
#include "../include/config.h"
#include "../include/newton.h"
#include "../include/ppm_writer.h"
#include "../include/roots.h"
#include "../include/thread_utils.h"

// Global variables visible to all threads

atomic_int block_counter;
atomic_int write_idx;

mtx_t lock;
cnd_t cond;

int total_blocks;
int n_buffers;

int l;
int d;
int n_threads;

char att_file[40];
char conv_file[40];

char *attractor_strings[NUM_COLORS_MAX]={"127 63 63 ","125 127 63 ","63 127 68 ","63 120 127 ","72 63 127 ","127 63 116 ","127 76 63 ","112 127 63 ","63 127 81 ","63 107 127 ","85 63 127 ","127 63 103 ","127 90 63 ","98 127 63 ","63 127 94 ","63 94 127 ","98 63 127 ","127 63 90 ","127 103 63 ","85 127 63 ","63 127 107 ","63 81 127 ","112 63 127 ","127 63 76 ","127 116 63 ","72 127 63 ","63 127 120 ","63 68 127 ","125 63 127 "};
char *convergence_strings[MAX_ITERATIONS];

BlockBuffer *buffers = NULL;
int n_threads;

//demo variables
size_t row_size = 8;//Change (obviously)
char *output_str = NULL;
size_t output_capacity = 0;
size_t output_len = 0;


int main(int argc, char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s [-t <num_threads>] [-l <num_lines>] <degree>\n",
            argv[0]);
    fprintf(stderr, "  -t: Number of threads (default: 1)\n");
    fprintf(stderr, "  -l: Image size (rows and columns, default: 1000)\n");
    fprintf(stderr, "  <degree>: Exponent d for x^d - 1 (1 <= d < 10)\n");
    exit(1);
  }

  // Parse command line arguments
  bool degree_set = false;
  for (int i = 1; i < argc; ++i) {

    if (strcmp(argv[i], "-t") == 0) {

      if (++i >= argc) {
        fprintf(stderr, "Error: -t requires an argument\n");
        exit(1);
      }

      n_threads = (size_t)atoi(argv[i]);
      if (n_threads == 0) {
        fprintf(stderr, "Error: Number of threads must be greater than 0\n");
        exit(1);
      }

    } else if (strcmp(argv[i], "-l") == 0) {
      if (++i >= argc) {
        fprintf(stderr, "Error: -l requires an argument\n");
        exit(1);
      }

      l = (size_t)atoi(argv[i]);
      if (l <= 0) {
        fprintf(stderr,
                "Error: Number of lines must be at least 1\n");
        exit(1);
      }

    } else {
      // Positional argument: degree (last argument)
      d = atoi(argv[i]);
      degree_set = true;
    }
  }
  total_blocks=l;
  if (!degree_set || d < 1 ) {
    fprintf(stderr, "Error: Degree must be provided and between 1 and 9\n");
    exit(1);
  }
  //set n_buffers and filenames
  n_buffers=n_threads+2;
  snprintf(att_file, sizeof(att_file), "newton_attractors_x%d.ppm", d);
  snprintf(conv_file, sizeof(conv_file), "newton_convergence_x%d.ppm", d);

  printf("threads: %d, rows: %d, degree: %d, buffers: %d",n_threads,total_blocks,d,n_buffers);

  mtx_init(&lock,mtx_plain);
  cnd_init(&cond);

  // allocate buffers
  buffers = malloc(n_buffers * sizeof(BlockBuffer));
  for (int i = 0; i <n_buffers; ++i) {
    buffers[i].data = malloc(row_size * sizeof(_Complex double));
    buffers[i].size = row_size;
    atomic_init(&buffers[i].state, FREE);
    buffers[i].block_index= -1;
  }

  // launch compute threads
  thrd_t *threads = malloc(n_threads * sizeof(thrd_t));
  ThreadArg *args = malloc(n_threads * sizeof(ThreadArg));
  for (int i = 0; i < n_threads; ++i) {
    args[i].thread_id = i;
    thrd_create(&threads[i], demo_loop, &args[i]);
  }

  // launch writer thread
  thrd_t writer;
  thrd_create(&writer,writer_func , NULL);

  // join all threads
  for (int i = 0; i < n_threads; ++i)
    thrd_join(threads[i], NULL);
  thrd_join(writer, NULL);


  printf("=== All rows complete ===\n%s", output_str);

  // cleanup
  for (int i = 0; i < n_buffers; ++i)
    free(buffers[i].data);
  free(buffers);
  free(threads);
  free(args);
  free(output_str);
  free_convergence();
  mtx_destroy(&lock);
  cnd_destroy(&cond);

  return 0;
}
