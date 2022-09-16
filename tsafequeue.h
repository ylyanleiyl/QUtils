#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <QDebug>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

//static quint32 Max_Size=(1<<31);

/*读写互斥、写写也要互斥 */
template <typename T>
class TSafeQueue
{
public:
    TSafeQueue(){
    }

    void enqueue(const T &t)
    {
        m_oMutex.lock();
//        if( m_queue.size() >= Max_Size )
//            m_bufferIsNotFull.wait(&m_oMutex);//不阻塞生产者线程

        m_queue.enqueue(t);
        m_oMutex.unlock();
        m_bufferIsNotEmpty.wakeAll();
    }

    T dequeue()
    {
        m_oMutex.lock();
        if (m_queue.empty())
        {
            //qDebug() << "wait begin";
            m_bufferIsNotEmpty.wait(&m_oMutex);//wait的过程：解锁－>阻塞当前线程－>满足条件－>锁定－>返回
            //qDebug() << "wait end";
        }
        T t = m_queue.dequeue();
        m_oMutex.unlock();
        //m_bufferIsNotFull.wakeAll();
        return t;
    }

    //no lock
    bool dequeue(T &t)
    {
        m_oMutex.lock();
        if (m_queue.empty())
        {
            m_oMutex.unlock();
            return false;
        }
        t = m_queue.dequeue();
        m_oMutex.unlock();
        return true;
    }

    bool dequeue(QList<T> &lt)
    {
        m_oMutex.lock();
        if (m_queue.empty())
        {
            m_oMutex.unlock();
            return false;
        }
        while (!m_queue.isEmpty()) {
            lt.append(m_queue.dequeue());
        }
        m_oMutex.unlock();
        return true;
    }

    quint32 size(){quint32 size=0;m_oMutex.lock();size=m_queue.size();m_oMutex.unlock();return size;}
private:
    QQueue<T> m_queue;
    QMutex m_oMutex;
    QWaitCondition m_bufferIsNotEmpty;
    //QWaitCondition m_bufferIsNotFull;
};

#endif // SAFEQUEUE_H
