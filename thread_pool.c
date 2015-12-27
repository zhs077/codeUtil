#include "thread_pool.h"
int thread_pool_init(thread_pool *self, int max_thread_num){
  
  self = (thread_pool*)malloc(sizeof(thread_pool));
  self->cur_task_num = 0;
  self->max_thread_num = max_thread_num;
  pthread_mutex_init(&self->task_list_mutext);
  
  
  
  
}
