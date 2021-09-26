#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include "threadspool.hpp"

void * threadTask(void * arg)
{   
    Pthreadspool * pool = (Pthreadspool *)arg;
loop:
//  若任务队列为空，则阻塞等待新增任务的信号，对任务队列的访问需互斥进行
    pthread_mutex_lock(&(pool->pool_mutex));
    while(pool->tasks_queue.empty() && !(pool->pool_close) && !(pool->pool_reduce))
    {   pthread_cond_wait(&(pool->pool_cond),&(pool->pool_mutex));  }

/*  1.若任务队列不为空，则应先去任务执行
    2.队列为空，且设置了close标志，则需要终止当前线程，并修改相关状态i信息
    3.队列为空，未设置close，但设置了reduce标志，仍需要终止当前线程，并修改相关状态信息 */
    if(!pool->tasks_queue.empty()) {   pool->execute();    }
    else if(pool->pool_close) 
    {
        --(pool->pool_size);
        --(pool->remains);
        --(pool->free_nums);
        pool->reduce();
    } 
    else if(pool->pool_reduce)
    {   
//  若剩余线程数为原线程数的一半，则停止缩减
        if(--(pool->remains) == (pool->pool_size) / 2)
        {
            pool->pool_reduce = false;
            pool->pool_size = pool->remains;
        }
        --(pool->free_nums);
        pool->reduce();
    }
    goto loop;

    return (void *) (0);

}

void * adminDaemon(void * arg)
{   
/*  
    管理线程有定时检查线程池的活跃度，由活跃度决定是否需要缩减或扩展线程池
    时间间隔由成员变量interval指定  */

    Pthreadspool * pool = (Pthreadspool *)arg;
loop:
    sleep(pool->interval);
//  定时从链表中删除已终止线程的描述符，避免链表积累过长
    pool->tryjoin();
//  在此过程中持有相关的互斥量，所以必须保证在释放锁之前线程不能响应cancel请求
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);
    pthread_mutex_lock(&(pool->pool_mutex)); 
    switch(pool->testbusy())
    {
    case 0:
        pthread_mutex_unlock(&(pool->pool_mutex));
        break;
    case 1:
        pool->pool_reduce = true;
        pthread_mutex_unlock(&(pool->pool_mutex));
        pthread_cond_broadcast(&pool->pool_cond);
        break;
    case 2:
        //pthread_mutex_unlock(&(pool->pool_mutex));
        pool->extend(pool->pool_size);
        break;
    default:
        pthread_mutex_unlock(&(pool->pool_mutex));
        break;
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);
    // pool->Print("Admin");
    goto loop;

    return (void *) (0);
}

Pthreadspool::Pthreadspool(int pool_size, int max_size, int interval)
:pool_size(pool_size), max_size(max_size), interval(interval),
pool_close(false), pool_reduce(false)
{
    normal = initialize();
}
Pthreadspool::~Pthreadspool()
{
    Close();
    pthread_mutex_destroy(&threads_mutex);
    pthread_mutex_destroy(&pool_mutex);
    pthread_mutex_destroy(&resource_mutex);
    pthread_cond_destroy(&pool_cond);
}
bool Pthreadspool::initialize()
{
/*
    线程池初始化，创建指定数量的线程并阻塞等待任务信号
    线程描述符加入链表中，用来进行管理线程
    创建管理线程，独立于执行线程组，进行线程池管理任务  */
    
    pthread_t thread_id;
    int i,numbers;

//  初始线程数不能大于线程池最大规模
    if(pool_size > max_size) pool_size = remains = max_size;
//  设置相关状态信息
    busy_nums = 0;
//  初始化条件变量及互斥量，任一变量初始化错误则停止初始化，并返回false表示失败
    if(pthread_cond_init(&pool_cond, nullptr)               \
        || pthread_mutex_init(&threads_mutex,nullptr)       \
        || pthread_mutex_init(&pool_mutex,nullptr)          \
        || pthread_mutex_init(&resource_mutex,nullptr))     \
        return false;

    for(i = 0, numbers = 0; i < pool_size; ++i)
    {
//  若线程创建失败，则继续创建其他后续线程
        if(pthread_create(&thread_id,nullptr,threadTask,this))
            continue;
//  若线程创建成功，则统计成功创建的线程数，并将线程描述符加入threads_list链表中
        threads_list.push_back(thread_id);
        ++numbers;
    }

//  尝试重新创建线程，达到用户需要的线程数
    for(i = numbers; i < pool_size; ++i)
    {
        if(pthread_create(&thread_id,nullptr,threadTask,this))
            continue;
        threads_list.push_back(thread_id);  
        ++numbers;
    }
//  线程池大小设置为成功创建的线程数
    free_nums = pool_size = remains = numbers;

//  while(pthread_create(&administrator,nullptr,adminDaemon,this));

    return true;
}
void Pthreadspool::execute()
{
//  取出队列头的任务
    void * parameter = tasks_queue.front().arg;
    void * (*function) (void *) = tasks_queue.front().run;
    tasks_queue.pop();

    --free_nums;
    ++busy_nums;
    
//  释放互斥量，执行任务
    pthread_mutex_unlock(&pool_mutex);
    function(parameter);

    pthread_mutex_lock(&pool_mutex);
    ++free_nums;
    --busy_nums;
    pthread_mutex_unlock(&pool_mutex);
   
}
void Pthreadspool::reduce()
{   
    pthread_mutex_unlock(&pool_mutex);
    pthread_exit(nullptr);

}
int Pthreadspool::testbusy()
{
    if(pool_reduce) return 0;
    if((busy_nums + tasks_queue.size()) >= pool_size) return 2;
    if((busy_nums + tasks_queue.size()) < (pool_size / 2)) return 1;
    else return 0;
}

void Pthreadspool::extend(int numbers)
{
    pthread_t thread_id;

    numbers = (pool_size + numbers) > max_size ? (max_size - pool_size) : numbers;
    
    pool_size += numbers;
    free_nums += numbers;
    remains += numbers;

    printf("reduce ? %d\n",pool_reduce);
    // Print("extend");

    while(numbers)
    {
        if(pthread_create(&thread_id,nullptr,threadTask,nullptr))
            continue;
        --numbers;

        printf("numbers %d\n",numbers);
    //  由于在此情景下只有此线程会访问线程描述符链表，所以此处不需要加锁
        threads_list.push_back(thread_id);
    }
    pthread_mutex_unlock(&(pool_mutex));
}

void Pthreadspool::tryjoin()
{
    pthread_mutex_lock(&threads_mutex);
    decltype(threads_list.begin()) thread = threads_list.begin(), pointer;
    while(thread != threads_list.end())
    {
    //  线程存在则不需要回收资源
        if(pthread_kill(*thread,0) == 0) 
        {   ++thread;
            continue;
        }
    //  线程已终止，回收资源并将描述符号从链表中删除
        pointer = thread++;
        pthread_join(*pointer,nullptr);
        threads_list.erase(pointer);
    }
    pthread_mutex_unlock(&threads_mutex);
}

void Pthreadspool::Close()
{
    decltype(threads_list.begin()) thread_id_p = threads_list.begin(), p;
//  设置close标志，并唤醒所有阻塞线程，
    pool_close = true;
    pthread_cond_broadcast(&pool_cond);
    while(!tasks_queue.empty());
    pthread_mutex_lock(&threads_mutex);

//  等待所有线程终止，回收资源，删除描述符号
    while(thread_id_p != threads_list.end())
    {
        p = thread_id_p++;
        pthread_join(*p,nullptr);
        threads_list.erase(p);
    }
/*
    pthread_mutex_unlock(&threads_mutex);
    pthread_cancel(administrator);
    pthread_join(administrator,nullptr);
*/
}

void Pthreadspool::Print(char * str)
{
    // printf("after %s\t[");
    printf("normal:%d "
            "pool_size:%d "
            "max_size :%d "
            "busy_nums:%d "
            "free_nums: %d "
            "remains:%d "
            "task_nums:%d]\n",
    normal, pool_size, max_size, busy_nums, free_nums, remains, (int)tasks_queue.size());
}

void Pthreadspool::addTask(struct task & new_task)
{
    pthread_mutex_lock(&pool_mutex);
    tasks_queue.push(new_task);
    pthread_mutex_unlock(&pool_mutex);
    pthread_cond_broadcast(&pool_cond);
}

void Pthreadspool::addTask(std::list<struct task> & t_list)
{
    pthread_mutex_lock(&pool_mutex);
    for(struct task t : t_list) tasks_queue.push(t);
    pthread_mutex_unlock(&pool_mutex);
    pthread_cond_broadcast(&pool_cond);
}
