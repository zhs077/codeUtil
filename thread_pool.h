#ifndef THREAD_POOL_H
#define THREAD_POOL_H


typedef struct thread_pool_s thread_pool_t;
typedef void * (*thread_func_t)(void *);

/*
创建线程池
参数:
int max_thread_num 线程个数
返回值：
成功返回线程池对象，失败返回NULL
*/
thread_pool_t* thread_pool_create(int max_thread_num);

/*
往线程池放入一个任务
参数:
thread_pool_t *self 线程池对象
thread_func_t thread_func 执行任务函数
void *arg					参数
返回值：
0代表成功，-1代表失败
失败原因是当前任务数大于最大任务数
*/
int  thread_pool_add_task(thread_pool_t *self, thread_func_t thread_func, void *arg);

/*
等待所有任务执行完
参数:
thread_pool_t *self 线程池对象
*/
void thread_pool_wait(thread_pool_t *self);

/*
销毁线程池
参数:
thread_pool_t *self 线程池对象
返回值：
0代表成功，-1代表失败
*/
int thread_pool_destroy(thread_pool_t *self);


#endif // THREAD_POOL_H
