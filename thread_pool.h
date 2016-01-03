#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <pthread.h>



typedef void * (*thread_func_t)(void *);

typedef struct task_node_s{
	thread_func_t 			thread_func;
    void 			    	*arg;
    struct task_node_s 	    *next;
}task_node_t;

typedef struct task_deque_s{
    int 			    	deque_len;
	task_node_t             *head;
	pthread_mutex_t 		mutex;	
}task_deque_t;

typedef struct thread_pool_s{
    int 						max_thread_num;
    int 						cur_task_num;
	volatile int	            working_threads_num; /*工作任务数*/
	volatile int	            alive_threads_num; /*工作任务数*/
    int 						shutdown;
	volatile int						keep_alive;
    task_deque_t 				*task_deque;
    pthread_mutex_t 		threads_num_mutex;
	pthread_cond_t  		threads_all_idle_cond;
	
	/*用于线程池调度*/
	pthread_mutex_t         task_mutex;
	int v;
    pthread_cond_t  		task_cond;

    pthread_t 				*thread_id_array;
 
}thread_pool_t;


thread_pool_t* thread_pool_create(int max_thread_num);
void thread_pool_add_task(thread_pool_t *self, thread_func_t thread_func, void *arg);
int  thread_pool_destroy(thread_pool_t *self);
void thread_pool_wait(thread_pool_t *self);

#endif // THREAD_POOL_H
