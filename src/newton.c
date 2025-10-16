#include <stdlib.h>//GETRIDOFALLTHESE
#include <time.h>
#include <stdio.h>

#include "../include/newton.h"
#include "../include/complex_utils.h"
#include "../include/config.h"


int newton_loop(void *arg){
  //stackarray is supersmallenough for our purposes
  _Complex double points[BLOCKSIZE];

  while (1) {
    //find block or quit----------------------------
    BlockBuffer *block = NULL;

    mtx_lock(&lock);
    // find a FREE buffer
    for (int i = 0; i < n_buffers; ++i) {
      if (atomic_load(&buffers[i].state) == FREE &&
        atomic_load(&block_counter) < total_blocks) {
        block= &buffers[i];
        atomic_store(&block->state, FILLING);
        block->block_index = atomic_fetch_add(&block_counter, 1);
        break;
      }
    }
    mtx_unlock(&lock);
    //quit if done, wait if not
    if (!block) {
      if (atomic_load(&block_counter) >= total_blocks)
        break;
      thrd_sleep(&(struct timespec){.tv_nsec = 1000000}, NULL); // 1 ms
      continue;
    }
    //compute datapoints
    uint64_t start_idx = block->block_index * BLOCKSIZE;
    size_t end_idx = start_idx + BLOCKSIZE;
    if (end_idx > n_points) end_idx = n_points; //catch last block oversize
    generate_block_points(start_idx,end_idx,points);

    //run newtonloop
    uint32_t count=end_idx-start_idx;
    block->size=count;
    for (uint32_t p_ind=0; p_ind<count; ++p_ind) {

      _Complex double x=points[p_ind];
      for (uint8_t i=0;i<MAX_ITERATIONS;++i ){

        x=newton_update(x);
        int r=getroot_arg(x);

        if (r!=-1){
          block->con_data[p_ind]=i;
          block->att_data[p_ind]=r;
          break;
        }
        if (is_bailout(x)||is_close_to_origin(x)){
          block->con_data[p_ind]=MAX_ITERATIONS-1;
          block->att_data[p_ind]=NUM_COLORS_MAX-1;
          break;
        }
        if (i==MAX_ITERATIONS-1){
          block->con_data[p_ind]=MAX_ITERATIONS-1;
          block->att_data[p_ind]=NUM_COLORS_MAX-1;
          break;
        }
        
      }
    }

    //mark READY and signal writer
    mtx_lock(&lock);
    atomic_store(&block->state, READY);
    cnd_signal(&cond);
    mtx_unlock(&lock);
  
  }

  return 0;
}

