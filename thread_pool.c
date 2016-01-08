#include "thread_pool.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <pthread.h>

#define THREAD_POOL_LOG(format, ...) do { \
	fprintf(stderr, format, ##__VA_ARGS__); \
} while(0)


#define MAX_THREAD_NUM 64
#define MAX_TASK_NUM 1024

typedef struct task_node_s{
	thread_func_t 			thread_func;
    void 			    	*arg;
    struct task_node_s 	    *next;
}task_node_t;

typedef struct task_deque_s{
    volatile int			deque_len;
	task_node_t             *head;
	pthread_mutex_t 		mutex;	
}task_deque_t;

typedef struct task_sem_s{
	pthread_mutex_t        	task_mutex;
	int 					v;
    pthread_cond_t  		task_cond;
}task_sem_t;

struct thread_pool_s{
    int 						max_thread_num;
	volatile int	            working_threads_num; 
	volatile int	            alive_threads_num; 
	volatile int			    keep_alive;
    task_deque_t 				*task_deque;
    pthread_mutex_t 			threads_num_mutex;
	pthread_cond_t  			threads_all_idle_cond;
	/*用于线程池调度*/
	task_sem_t                  *task_sem;
    pthread_t 					*thread_id_array;
 
};


static void deque_init(task_deque_t *self);
static void deque_push(task_deque_t *self,task_node_t *task_node);
static task_node_t* deque_pull(task_deque_t *self);
static void deque_clear(task_deque_t *self);
static void deque_destory(task_deque_t *self);

static void task_sem_init(task_sem_t *self);
static void task_sem_signal(task_sem_t *self);
static void task_sem_broadcast(task_sem_t *self);
static void task_sem_wait(task_sem_t *self);
static void task_sem_destory(task_sem_t *self);

void* thread_pool_thread_do(void *args);

/* ========================== semaphore ============================ */
static void task_sem_init(task_sem_t *self){
	pthread_mutex_init(&self->task_mutex, NULL);
	self->v = 0;
	pthread_cond_init(&self->task_cond, NULL);
}

static void task_sem_destory(task_sem_t *self){
	pthread_mutex_destroy(&self->task_mutex);
	pthread_cond_destroy(&self->task_cond);
}

static void task_sem_broadcast(task_sem_t *self){
	pthread_mutex_lock(&self->task_mutex);
	self->v = 1;
	pthread_cond_broadcast(&self->task_cond);
	pthread_mutex_unlock(&self->task_mutex);
}

static void task_sem_wait(task_sem_t *self){
	pthread_mutex_lock(&self->task_mutex);
	while (self->v != 1){
		pthread_cond_wait(&self->task_cond, &self->task_mutex);
	}
	self->v = 0;
	pthread_mutex_unlock(&self->task_mutex);	
}

static void task_sem_signal(task_sem_t *self){
	pthread_mutex_lock(&self->task_mutex);
	self->v = 1;
	pthread_cond_signal(&self->task_cond);
	pthread_mutex_unlock(&self->task_mutex);
}
/* ========================== deque ============================ */
static void deque_init(task_deque_t *self){
	self->deque_len = 0;
	self->head = NULL;
    pthread_mutex_init(&self->mutex, NULL);
}
/*外部需要加锁*/
static void deque_push(task_deque_t *self,task_node_t *task_node){
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
/*外部需要加锁*/
static task_node_t* deque_pull(task_deque_t *self){
	
	task_node_t *task_node = self->head;
	if (self->deque_len == 0){
		
	}else{
		self->head = self->head->next;
		self->deque_len--;
	}
	return task_node;
}
static void deque_clear(task_deque_t *self){
	
	while(self->deque_len){
		free(deque_pull(self));
	}
	self->deque_len = 0;
	self->head = NULL;
}
static void deque_destory(task_deque_t *self){
	
	deque_clear(self);
	pthread_mutex_destroy(&self->mutex);
}

/* ========================== thread_pool ============================ */
void* thread_pool_thread_do(void *args){
	thread_pool_t *self = (thread_pool_t*)args;
	task_node_t  *task = NULL;
	pthread_mutex_lock(&self->threads_num_mutex);
	self->alive_threads_num++;
	pthread_mutex_unlock(&self->threads_num_mutex);

	while(self->keep_alive){
		
		task_sem_wait(self->task_sem);
		if (self->keep_alive){
			
			pthread_mutex_lock(&self->threads_num_mutex);
			self->working_threads_num++;
			pthread_mutex_unlock(&self->threads_num_mutex);
			
			pthread_mutex_lock(&self->task_deque->mutex);
			task = deque_pull(self->task_deque);
			
			/*如果队列还有任务则唤醒一个线程*/
			if (self->task_deque->deque_len > 0){
				task_sem_signal(self->task_sem);
			}
			pthread_mutex_unlock(&self->task_deque->mutex);
			if (task){
				task->thread_func(task->arg);
				free(task);
			}
			pthread_mutex_lock(&self->threads_num_mutex);
			self->working_threads_num--;
			if (self->working_threads_num == 0) {
				pthread_cond_signal(&self->threads_all_idle_cond);
			}
			pthread_mutex_unlock(&self->threads_num_mutex);
		}
	}

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
	self->keep_alive = 1;
	self->max_thread_num = max_thread_num;
	self->thread_id_array = (pthread_t*)malloc(sizeof(pthread_t) *self->max_thread_num);
	self->task_deque = (task_deque_t*)malloc(sizeof(task_deque_t));
	deque_init(self->task_deque);
	pthread_mutex_init(&self->threads_num_mutex, NULL);
	pthread_cond_init(&self->threads_all_idle_cond, NULL);
	
	self->task_sem = (task_sem_t*)malloc(sizeof(task_sem_t));
	task_sem_init(self->task_sem);

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
		
		task_sem_broadcast(self->task_sem);
		usleep(100000);
	}
	free(self->thread_id_array);
	pthread_mutex_destroy(&self->threads_num_mutex);
	pthread_cond_destroy(&self->threads_all_idle_cond);
	deque_destory(self->task_deque);
	task_sem_destory(self->task_sem);
	free(self->task_sem);
	free(self->task_deque);
	
	free(self);
	self = NULL;
	return 0;

}

int thread_pool_add_task(thread_pool_t *self, thread_func_t thread_func, void *arg){
	task_node_t  *task = NULL;
	
	if (self->task_deque->deque_len > MAX_TASK_NUM){
		return -1;
	}
	task = (task_node_t*)malloc(sizeof(task_node_t));
	task->arg = arg;
	task->thread_func = thread_func;
	task->next = NULL;
	/*将任务加入队列*/
	pthread_mutex_lock (&self->task_deque->mutex);
	deque_push(self->task_deque, task);
	/*唤醒一个工作线程*/
	task_sem_signal(self->task_sem);
	pthread_mutex_unlock(&self->task_deque->mutex);
	return 0;
}

void thread_pool_wait(thread_pool_t *self){
	
	pthread_mutex_lock(&self->threads_num_mutex);
	while(self->task_deque->deque_len || self->working_threads_num ){
		pthread_cond_wait(&self->threads_all_idle_cond, &self->threads_num_mutex);
	}
	pthread_mutex_unlock(&self->threads_num_mutex);
}
