#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/ppm_writer.h"
#include "../include/config.h"


char *preformatted[31]; // index 0 unused

// initialize preformatted strings
void init_preformatted() {
  for (int i = 1; i <= 30; ++i) {
    // enough space for "nnn " including null
    preformatted[i] = malloc(6);
    snprintf(preformatted[i], 6, "%3d ", i);
  }
}

// cleanup preformatted strings
void free_preformatted() {
  for (int i = 1; i <= 30; ++i)
    free(preformatted[i]);
}
// ---- Helper to append to output ----
static void append_output(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  char temp[512];
  int len = vsnprintf(temp, sizeof(temp), fmt, args);
  va_end(args);
  if (output_len + len + 1 > output_capacity) {
    output_capacity = (output_capacity + len + 512) * 2;
    output_str = realloc(output_str, output_capacity);
  }
  memcpy(output_str + output_len, temp, len);
  output_len += len;
  output_str[output_len] = '\0';
}



// ---- demo writer ----
int demo_func(void *arg) {
  (void)arg;

  while (1) {
    mtx_lock(&lock);
    BlockBuffer *block = NULL;
    while (!block) {
        // look for block matching next_block_to_write
      for (int i = 0; i < n_buffers; ++i) {
        if (atomic_load(&buffers[i].state) == READY &&
                buffers[i].block_index == atomic_load(&write_idx)) {
          block = &buffers[i];
          atomic_store(&block->state, WRITING);
          break;
        }
      }

      if (!block) {
        if (atomic_load(&write_idx) >= total_blocks) {
          mtx_unlock(&lock);
          return 0; // done
        }
        cnd_wait(&cond, &lock);
      }
    }
    mtx_unlock(&lock);

    // simulate writing
    append_output("Row %3d: ", block->block_index);
    for (size_t i = 0; i < block->size; ++i)
      append_output("%5.1f ", creal(block->data[i]));
    append_output("\n");

    // mark FREE and advance
    mtx_lock(&lock);
    atomic_store(&block->state, FREE);
    atomic_fetch_add(&write_idx, 1);
    cnd_broadcast(&cond);
    mtx_unlock(&lock);
  }
}

// ---- Writer thread using fwrite and preformatted strings ----
int demo_filewriter(void *arg) {
  (void)arg;

  init_preformatted();

  FILE *att_fp = fopen(att_file, "wb");
  FILE *conv_fp = fopen(conv_file, "wb");

  if (!att_fp || !conv_fp) {
    perror("fopen");
    free_preformatted();
    return -1;
  }
  



  // write a simple demo header for PPM
  fprintf(att_fp, "P3\n%zu %d\n255\n", row_size, total_blocks);
  fprintf(conv_fp, "P3\n%zu %d\n255\n", row_size, total_blocks);

  while (1) {
    mtx_lock(&lock);
    BlockBuffer *block = NULL;

    while (!block) {
      for (int i = 0; i < n_buffers; ++i) {
        if (atomic_load(&buffers[i].state) == READY &&
          buffers[i].block_index == atomic_load(&write_idx)) {

          block = &buffers[i];
          atomic_store(&block->state, WRITING);
          break;
        }
      }

      if (!block) {
        if (atomic_load(&write_idx) >= total_blocks) {
          mtx_unlock(&lock);
          fclose(att_fp);
          fclose(conv_fp);
          free_preformatted();
          return 0; // done
        }
        cnd_wait(&cond, &lock);
      }
    }
    mtx_unlock(&lock);

    // ---- write row using preformatted strings ----
    // for demo: map data to 1-30
    for (size_t i = 0; i < block->size; ++i) {
      int val = ((int)block->data[i]) % 30 + 1; // demo mapping
      fwrite(preformatted[val], 1, strlen(preformatted[val]), att_fp);
      fwrite(preformatted[val], 1, strlen(preformatted[val]), conv_fp);
    }
    fwrite("\n", 1, 1, att_fp);
    fwrite("\n", 1, 1, conv_fp);

    // mark FREE and advance
    mtx_lock(&lock);
    atomic_store(&block->state, FREE);
    atomic_fetch_add(&write_idx, 1);
    cnd_broadcast(&cond);
    mtx_unlock(&lock);
  }
}

char *attbuf;
char *convbuf;


void init_convergence(){
  for (int i=0; i<MAX_ITERATIONS; ++i) {
    int val=(i*255)/MAX_ITERATIONS;
    convergence_strings[i]=malloc(13);
    if (val<10) sprintf(convergence_strings[i], "%d   %d   %d   ", val, val, val);
    else if (val<100) sprintf(convergence_strings[i], "%d  %d  %d  ", val, val, val);
    else sprintf(convergence_strings[i], "%d %d %d ",val,val,val);
  }
}

void free_convergence(){
  for (int i=0; i<MAX_ITERATIONS; ++i) {
    free(convergence_strings[i]);
  }
}

int writer_func(void *arg){
  (void)arg;

  init_convergence();
  attbuf=malloc(BLOCKSIZE*STRING_LENGTH+4);
  convbuf=malloc(BLOCKSIZE*STRING_LENGTH+4);

  char *pa=attbuf;
  char *pc=convbuf;

  FILE *att_fp = fopen(att_file, "wb");
  FILE *conv_fp = fopen(conv_file, "wb");

  if (!att_fp || !conv_fp) {
    perror("fopen");
    return -1;
  }

  // write a simple demo header for PPM
  fprintf(att_fp, "P3\n%d %d\n255\n",l, l);
  fprintf(conv_fp, "P3\n%d %d\n255\n",l, l);

  while (1){
    mtx_lock(&lock);
    BlockBuffer *block = NULL;

    while (!block) {
      for (int i = 0; i < n_buffers; ++i) {
        if (atomic_load(&buffers[i].state) == READY &&
          buffers[i].block_index == atomic_load(&write_idx)) {
          block = &buffers[i];
          atomic_store(&block->state, WRITING);
          break;
        }
      }


      if (!block) {
        if (atomic_load(&write_idx) >= total_blocks) {
          mtx_unlock(&lock);
          fclose(att_fp);
          fclose(conv_fp);
          free_preformatted();
          return 0; // done
        }
        cnd_wait(&cond, &lock);
      }
    }
    mtx_unlock(&lock);

    //reset pointers    
    pa=attbuf;
    pc=convbuf;

    //fill string buffers
    for (size_t i = 0; i < block->size; ++i) {
      int aidx = ((int)block->data[i]) % MAX_COLOR_VALUE; // demo mapping
      int cidx = ((int)block->data[i]) % MAX_ITERATIONS;
      memcpy(pa,attractor_strings[aidx],STRING_LENGTH);
      memcpy(pc, convergence_strings[cidx],STRING_LENGTH);
      pa+=STRING_LENGTH;
      pc+=STRING_LENGTH; 
    }
    //add some optional linebreaks (noncorresponding to l)
    *pa++='\n';
    *pc++='\n';
    //fwrite to file
    printf("writing: %.*s\n", (int)(pa - attbuf), attbuf);
    fwrite(attbuf,1,pa-attbuf,att_fp);
    fwrite(convbuf, 1, pc-convbuf, conv_fp);

    // mark FREE and advance
    mtx_lock(&lock);
    atomic_store(&block->state, FREE);
    atomic_fetch_add(&write_idx, 1);
    cnd_broadcast(&cond);
    mtx_unlock(&lock);

  }
  printf("freing\n");
  free(convbuf);
  free(attbuf);

}








