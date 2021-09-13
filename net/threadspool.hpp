#ifndef THREADSPOOL_HPP
#define THREADSPOOL_HPP

#include <pthread.h>
#include <list>
#include <queue>

struct task
{   
    //  任务参数
    void * arg;
    //  任务函数
    void * (*run) (void *);
};

class Pthreadspool
{
    public:
    //  默认构造包含100个线程的线程池，线程池最大规模200，调度时间10s
    Pthreadspool() : Pthreadspool(100, 200, 10) {}
    //  构造线程池，初始线程数量由参数pool_size指定，最大规模200
    Pthreadspool(int pool_size, int interval) 
        : Pthreadspool(pool_size, 200, interval) {}
    //  构造线程池，指定初始线程数，线程池最大规模以及调度时间间隔
    Pthreadspool(int pool_size, int max_size, int interval);
    //  不支持线程池的复制构造
    Pthreadspool(const Pthreadspool & pool) = delete;
    //
    ~Pthreadspool();

    //  向线程池中添加新任务
    void addTask(struct task & new_task);
    void addTask(std::list<struct task> & t_list);

    /*  线程池的预拓展，当将要执行较多一批任务时，可预先扩展线程池，
        从而提供更好的性能支持  */
    void preExtend(int size) {  extend(size);   }

    //  对临界区进行加锁和解锁，线程池提供的一个互斥访问功能
    int resourceLock()
    {
        return pthread_mutex_lock(&resource_mutex);
    }
    int resourceUnlock()
    {
        return pthread_mutex_unlock(&resource_mutex);
    }
    //  提出关闭线程池的请求，当任务队列执行完毕后会中止所有线程
    void Close();

    private:
    //  私有扩展函数，提供线程池规模扩展核心逻辑
    void extend(int nums);
    //  私有缩减函数，提供线程池规模缩减核心逻辑
    void reduce();
    //  回收reduce过程中终止的线程资源
    void tryjoin();
    //  取出任务队列第一个任务执行
    void execute();
    //  测试线程池的活跃程度，用以判断是否需要扩展或缩减
    int testbusy();
    //  私有初始化函数，执行线程池创建的一系列初始化工作
    bool initialize();
    //  管理者线程的执行逻辑
    friend void * adminDaemon(void * arg);
    //  每个线程的执行函数
    friend void * threadTask(void * arg);
    //  打印相关状态信息，用于调试输出
    void Print(char * str);

    private:

    /*  pool_close :    是否关闭线程池
        pool_reduce :   是否减小线程池规模  */
    volatile bool pool_close, pool_reduce;

    //  标记线程池是否存在错误
        bool normal;

    /*  pool_size :     线程池中包含的线程数量
        busy_nums :     正在执行任务的线程数量
        free_nums :     空闲的线程数量
        max_size  :     线程池扩展时可达到的最大规模    
        remains   :     缩减过程中，保存当前的线程数    */
    volatile int \
            pool_size, busy_nums, free_nums, max_size, remains;

    //  管理线程执行管理的时间间隔，需在不同使用情景下设置合适的值
        int interval;

    //  管理线程的线程描述符
    pthread_t administrator;

    /*  
        pool_cond : 必要的条件变量，用来阻塞和激活线程  */
    pthread_cond_t pool_cond;

    /*  threads_mutex : 用于锁threads_list的互斥量
        pool_mutex :    用于加锁其他状态信息的互斥量
        resource_mutex: 提供一个控制资源互斥的互斥量   */
    pthread_mutex_t threads_mutex, pool_mutex, resource_mutex;

    //  线程描述符链表，用以对线程池中的线程进行管理
    std::list<pthread_t> threads_list;

    //  任务队列，保存用户添加的任务形成队列，以FIFO的方式执行任务
    std::queue<struct task> tasks_queue;    
};

#endif
