#include "thread_pool.h"



void thread_pool_thread_routine(void *args){
  
}

int thread_pool_init(thread_pool *self, int max_thread_num){
  int i;
  
  self = (thread_pool*)malloc(sizeof(thread_pool));
  self->cur_task_num = 0;
  self->max_thread_num = max_thread_num;
  self->thread_id_array = (pthread_t)malloc(sizeof(pthread_t) *self->max_thread_num);
  pthread_mutex_init(&self->task_list_mutext);
  pthread_cond_init(&self->task_read_cond);
  for (i = 0; i < self->max_thread_num; i++){
    pthread_create(&self->thread_id_array[i], NULL, thread_pool_thread_routine, NULL);
  }
  
  return 0;
}
