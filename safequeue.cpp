#include "SafeQueue.h"

static int Max_Size=1<<31-1;

//使用QWaitCondition和QMutex可以实现比信号量更精确的控制
void CSafeQueue::enqueue(const qintptr &t)
{
    m_mutex.lock();
    m_nSize = m_queue.size();
    if( m_nSize == Max_Size )
    {
        m_bufferIsNotFull.wait(&m_mutex);//wait的过程：解锁－>阻塞当前线程－>满足条件－>锁定－>返回
    }
    m_queue.enqueue(t);
    qWarning() << "CSafeQueue::enqueue"<< m_queue.size();
    m_nSize = m_queue.size();
    emit(sizeChanged(m_nSize));
    m_mutex.unlock();
    m_bufferIsNotEmpty.wakeOne();
}

qintptr CSafeQueue::dequeue()
{
    m_mutex.lock();
    if (m_queue.empty())
    {
        m_mutex.unlock();
        return -1;
        m_bufferIsNotEmpty.wait(&m_mutex);//wait的过程：解锁－>阻塞当前线程－>满足条件－>锁定－>返回
    }
    qWarning() << "CSafeQueue::dequeue"<< m_queue.size();
    qintptr t = m_queue.dequeue();
    m_nSize = m_queue.size();
    emit(sizeChanged(m_nSize));
    m_mutex.unlock();
    m_bufferIsNotFull.wakeAll();
    return t;
}


/*
 * QMutex

QMutex类提供的是线程之间的访问顺序化。
QMutex的目的是保护一段代码，使得同一时间只有一个线程可以访问它。但在一个线程中调用lock()，其它线程将会在同一地点试图调用lock()时会被阻塞，直到这个线程调用unlock()之后其它线程才会获得这个锁。



QSemaphore
信号量QSemaphore可以理解为对互斥量QMutex功能的扩展，互斥量只能锁定一次而信号量可以获取多次，它可以用来保护一定数量的同种资源。acquire()函数用于获取n个资源，当没有足够的资源时调用者将被阻塞直到有足够的可用资源。release(n)函数用于释放n个资源。

#include <QtCore/QCoreApplication>
#include <QSemaphore>
#include <QThread>
#include <QMutex>
#include <iostream>
using namespace std;
QSemaphore vacancy(5);       //空位资源
QSemaphore produce(0);       //产品数量
QMutex mutex;                //互斥锁
int buffer[5];               //缓冲区可以放5个产品
int numtaken=60;
int takei=0;
class Producer:public QThread
{
    public:
    void run();
};
void Producer::run()         //生产者线程
{
    for(int i=0;i<60;i++)    //生产60次产品
    {
        vacancy.acquire();   //P(s1)操作原语
        buffer[i%5]=i;
        printf("produced %d\n",i);
        produce.release();   //V(s2)操作原语
    }
}
class Consumer:public QThread
{
    public:
    void run();
};
void Consumer::run()         //消费者线程
{
    while(numtaken>1)
    {
        produce.acquire();   //P(s2)操作原语
        mutex.lock();        //从缓冲区取出一个产品...多个消费者,不能同时取出,故用了互斥锁
        printf("thread %ul take %d from buffer[%d] \n",currentThreadId(),buffer[takei%5],takei%5);
        takei++;
        numtaken--;
        mutex.unlock();
        vacancy.release();   //V(s1)操作原语
    }
}
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Producer producer;
    Consumer consumerA;
    Consumer consumerB;
    producer.start();
    consumerA.start();
    consumerB.start();
    producer.wait();
    consumerA.wait();
    consumerB.wait();
    return 0;
}


*/
