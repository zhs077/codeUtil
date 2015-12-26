#ifndef THREAD_POOL_H
#define THREAD_POOL_H


typedef  struct thread_pool{


    int max_thread_num;
    //任务队列
    d_list  task_head;
    int cur_task_num;
    pthread_mutex_t task_list_mutext;
    pthread_cond_t  task_read_cond;
    


}thread_pool;

#endif // THREAD_POOL_H
