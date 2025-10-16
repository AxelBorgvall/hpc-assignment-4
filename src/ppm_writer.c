#include <stdarg.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/ppm_writer.h"
#include "../include/config.h"


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
          free(convbuf); 
          free(attbuf);
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
      int aidx = ((int)block->att_data[i]); // demo mapping
      int cidx = ((int)block->con_data[i]);
      memcpy(pa,attractor_strings[aidx],STRING_LENGTH);
      memcpy(pc, convergence_strings[cidx],STRING_LENGTH);
       
      pa+=STRING_LENGTH;
      pc+=STRING_LENGTH; 
    }
    //add some optional linebreaks (noncorresponding to l)
    *pa++='\n';
    *pc++='\n';
    //fwrite to file
		/* diagnostics
    fwrite(attbuf, 1, pa - attbuf, stdout);
    fputc('\n', stdout);
    fwrite(convbuf, 1, pc - convbuf, stdout);
    fputc('\n', stdout);
		*/
    fwrite(attbuf,1,pa-attbuf,att_fp);
    fwrite(convbuf, 1, pc-convbuf, conv_fp);

    // mark FREE and advance
    mtx_lock(&lock);
    atomic_store(&block->state, FREE);
    atomic_fetch_add(&write_idx, 1);
    cnd_broadcast(&cond);
    mtx_unlock(&lock);

  }
}








