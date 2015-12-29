#include "thread_pool.h"

#define THREAD_POOL_LOG(format, ...) do { \
  #if define OPEN_LOG \
	fprintf(stderr, format, ##__VA_ARGS__); \
 #endif\
} while(0)


void thread_pool_thread_routine(void *args){
  thread_pool *self = (thread_pool*)args;
  assert(self);
  
  while(1){
    
     pthread_mutex_lock(&self->task_deque_mutex);
  }
  
}

int thread_pool_init(thread_pool *self, int max_thread_num){
  int i, ret;
  
  self = (thread_pool*)malloc(sizeof(thread_pool));
  self->cur_task_num = 0;
  self->max_thread_num = max_thread_num;
  self->thread_id_array = (pthread_t)malloc(sizeof(pthread_t) *self->max_thread_num);
  pthread_mutex_init(&self->task_deque_mutex);
  pthread_cond_init(&self->task_read_cond);
  for (i = 0; i < self->max_thread_num; i++){
    ret = pthread_create(&self->thread_id_array[i], NULL, thread_pool_thread_routine, self);
  }
  self->shutdown = 0;
  
  return 0;
}


void thread_pool_destory(thread_pool *self){
  int i;
  
  if (self->shutdown){
    return;
  }
  self->shutdown = 1;
  pthread_cond_broadcast(&self->task_read_cond);
  /*等待所有线程退出*/
  for (i = 0; i < self->max_thread_num; i++){
    pthread_join(&self->thread_id_array[i]);
  }
  free(self->thread_id_array);
  pthread_mutex_destory(&self->task_list_mutex);
  pthread_cond_destroy(&self->task_read_cond);
  free(self);
  self = NULL;
  
  
}
