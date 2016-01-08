#include "thread_pool.h"
#include <pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
void *task_proc(void* args){
	int *task = (int*)args;
	printf("process task: %d\n", *task);
	
	
}

void *task_proc2(void* args){
	
	int *p  = NULL;
	assert(p);
	
	
}
/*任务数大于线程池个数*/
void test1()
{
	int i;
	int *task;
	printf("run test case :%s\n",__func__);
	thread_pool_t *pool = thread_pool_create(3);
	for (i=0; i <10; i++){
	    task = (int*)malloc(sizeof(int));
		*task = i;
		thread_pool_add_task(pool, task_proc, task);
	}
	thread_pool_wait(pool);
	thread_pool_destroy(pool);
	//assert(pool->working_threads_num == 0);

	printf("test case :%s finish\n",__func__);
	
}

/*任务数小于线程池个数*/
void test2()
{
	int i;
	int *task;
	printf("run test case :%s\n",__func__);
	thread_pool_t *pool = thread_pool_create(15);
	for (i=0; i <10; i++){
	    task = (int*)malloc(sizeof(int));
		*task = i;
		thread_pool_add_task(pool, task_proc2, task);
	}
	thread_pool_wait(pool);
	thread_pool_destroy(pool);
	printf("test case :%s finish\n",__func__);
	
}
/*执行任务发生崩溃*/
void test3()
{
	int i;
	int *task;
	printf("run test case :%s\n",__func__);
	thread_pool_t *pool = thread_pool_create(3);
	for (i=0; i <10; i++){
	    task = (int*)malloc(sizeof(int));
		*task = i;
		thread_pool_add_task(pool, task_proc, task);
	}
	thread_pool_wait(pool);
	thread_pool_destroy(pool);
	printf("test case :%s finish\n",__func__);
	
}
/*任务数大于队列的最大长度*/
void test4(){
	
	int i;
	int *task;
	int ret;
	printf("run test case :%s\n",__func__);
	thread_pool_t *pool = thread_pool_create(3);
	for (i=0; i <2048; i++){
	    task = (int*)malloc(sizeof(int));
		*task = i;
		while(1){
			ret= thread_pool_add_task(pool, task_proc, task);
			if (ret != 0){
				usleep(100000);
			}else{
				break;
			}
		}
		
	}
	thread_pool_wait(pool);
	thread_pool_destroy(pool);
	printf("test case :%s finish\n",__func__);
}

int main(){
	//test1();
	//test2();
	test4();
}
