#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define THREAD_POOL_LOG(format, ...) do { \
	fprintf(stderr, format, ##__VA_ARGS__); \
} while(0)


#define MAX_THREAD_NUM 64


void deque_init(task_deque_t *self){
	self->deque_len = 0;
	self->head = NULL;
    pthread_mutex_init(&self->mutex, NULL);
}
void deque_enter(task_deque_t *self,task_node_t *task_node){
	task_node_t  *tmp;
	tmp = self->head;
	if (tmp == NULL){
		self->head = task_node;
	}else{
		while(tmp->next != NULL){
			tmp = tmp->next;
		}
		tmp->next = task_node;
	}
	self->deque_len++;
	
}
task_node_t* deque_leave(task_deque_t *self){
	
	task_node_t *task_node = self->head;
	assert(task_node != NULL);
	self->head = self->head->next;
	self->deque_len--;
	return task_node;
}
void deque_destory(task_deque_t *self){
	self->deque_len = 0;
	self->head = NULL;
	pthread_mutex_destroy(&self->mutex);
}


void* thread_pool_thread_do(void *args){
	thread_pool_t *self = (thread_pool_t*)args;
	task_node_t  *task = NULL;

	assert(self);

	THREAD_POOL_LOG("start thread 0x%x\n", pthread_self());
	pthread_mutex_lock(&self->threads_num_mutex);
	self->alive_threads_num++;
	pthread_mutex_unlock(&self->threads_num_mutex);

	while(self->keep_alive){
		pthread_mutex_lock(&self->task_mutex);
		while (self->v != 1){
			pthread_cond_wait(&self->task_cond, &self->task_mutex);
		}
		self->v = 0;
		pthread_mutex_unlock(&self->task_mutex);
		
		if (self->keep_alive){
			
			pthread_mutex_lock(&self->threads_num_mutex);
			self->working_threads_num++;
			pthread_mutex_unlock(&self->threads_num_mutex);
			
			pthread_mutex_lock(&self->task_deque->mutex);
			task = deque_leave(self->task_deque);
			
			/*如果队列还有任务则唤醒一个线程*/
			if (self->task_deque->deque_len > 0){
				pthread_mutex_lock(&self->task_mutex);
				self->v = 1;
				pthread_cond_signal(&self->task_cond);
				pthread_mutex_unlock(&self->task_mutex);
			}
			pthread_mutex_unlock(&self->task_deque->mutex);
			assert(task);
			task->thread_func(task->arg);
			
			pthread_mutex_lock(&self->threads_num_mutex);
			self->working_threads_num--;
			if (self->working_threads_num == 0) {
				pthread_cond_signal(&self->threads_all_idle_cond);
			}
			pthread_mutex_unlock(&self->threads_num_mutex);
			
		
		}
	}

	THREAD_POOL_LOG("thread 0x%x exit\n", pthread_self());
	pthread_mutex_lock(&self->threads_num_mutex);
	self->alive_threads_num--;
	pthread_mutex_unlock(&self->threads_num_mutex);

}

thread_pool_t* thread_pool_create(int max_thread_num){
	int i, ret;

    if (max_thread_num <= 0 || max_thread_num > MAX_THREAD_NUM){
		return NULL;
	}
	thread_pool_t *self = (thread_pool_t*)malloc(sizeof(thread_pool_t));
	self->working_threads_num = 0;
	self->alive_threads_num = 0;
	self->keep_alive = 0;
	self->max_thread_num = max_thread_num;
	self->thread_id_array = (pthread_t*)malloc(sizeof(pthread_t) *self->max_thread_num);
	self->task_deque = (task_deque_t*)malloc(sizeof(task_deque_t));
	deque_init(self->task_deque);
	pthread_mutex_init(&self->threads_num_mutex, NULL);
	pthread_cond_init(&self->threads_all_idle_cond, NULL);
	pthread_mutex_init(&self->task_mutex, NULL);
	self->v = 0;
	pthread_cond_init(&self->task_cond, NULL);
	
	for (i = 0; i < self->max_thread_num; i++){
		ret = pthread_create(&self->thread_id_array[i], NULL, thread_pool_thread_do, self);
	}

	/*等待线程全部创建好*/
	while(self->alive_threads_num != self->max_thread_num){
		
	}
	return self;
}


int thread_pool_destroy(thread_pool_t *self){
	int i;

	if (self->keep_alive == 0){
		return -1;
	}
	
	self->keep_alive = 0;
	/*等待所有线程退出*/
	while(self->alive_threads_num){
		
		pthread_mutex_lock(&self->task_mutex);
		self->v = 1;
		pthread_cond_broadcast(&self->task_cond);
		pthread_mutex_unlock(&self->task_mutex);
		sleep(1);
	}
	free(self->thread_id_array);
	pthread_mutex_destroy(&self->threads_num_mutex);
	pthread_cond_destroy(&self->threads_all_idle_cond);
	
	pthread_mutex_destroy(&self->task_mutex);
	pthread_cond_destroy(&self->task_cond);
	deque_destory(self->task_deque);
	free(self->task_deque);
	free(self);
	self = NULL;
	return 0;

}

void thread_pool_add_task(thread_pool_t *self, thread_func_t thread_func, void *arg){
	task_node_t  *task = NULL;
	
	task = (task_node_t*)malloc(sizeof(task_node_t));
	task->arg = arg;
	task->thread_func = thread_func;
	task->next = NULL;
	/*将任务加入队列*/
	pthread_mutex_lock (&self->task_deque->mutex);
	deque_enter(self->task_deque, task);
	
		/*唤醒一个工作线程*/
	pthread_mutex_lock(&self->task_mutex);
	self->v = 1;
	pthread_cond_signal(&self->task_cond);
	pthread_mutex_unlock(&self->task_mutex);
	
	pthread_mutex_unlock(&self->task_deque->mutex);
	


	//pthread_cond_signal(&self->threads_all_idle);
}

void pthread_pool_wait(thread_pool_t *self){
	
	pthread_mutex_lock(&self->threads_num_mutex);
	while(self->task_deque->deque_len || self->working_threads_num ){
		pthread_cond_wait(&self->threads_all_idle_cond, &self->threads_num_mutex);
	}
	pthread_mutex_unlock(&self->threads_num_mutex);
}
void *aa(void* args){

	printf("xxxxx\n");
	sleep(5);
}
int main(){
	thread_pool_t *pool;
	int ret ;
    pool = thread_pool_create(3);
	printf("ret=%d\n", ret);
	printf("shutdown=%d\n", pool->shutdown);
	thread_pool_add_task(pool, aa, NULL);
    //pthread_pool_wait(pool);
	ret = thread_pool_destroy(pool);

	printf("ret=%d\n", ret);

	return 0;
}
