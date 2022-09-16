#ifndef SAFEQUEUE_H
#define SAFEQUEUE_H

#include <QDebug>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>


/*使用QWaitCondition控制 一个生成者多个消费者的问题,写入不控制，读取控制 */
//template <typename T>
class CSafeQueue : public QObject
{
    Q_OBJECT
signals:
    void sizeChanged(int);
public:
    CSafeQueue(): m_nSize(0) {
    }

    int size(){return m_nSize;}

    void enqueue(const qintptr &t);

    qintptr dequeue();

private:
    QQueue<qintptr> m_queue;
    QMutex m_mutex;
    QWaitCondition m_bufferIsNotEmpty;
    QWaitCondition m_bufferIsNotFull;
    int m_nSize;
};

#endif // SAFEQUEUE_H
