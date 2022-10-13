#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <QtCore/qqueue.h>
#include <QtCore/qreadwritelock.h>


class EventPrivate;
class Event
{
public:
    Event();
    ~Event();
public:
    void set();
    void clear();
    bool wait(unsigned long time = ULONG_MAX);
    bool isSet() const;
    quint32 getting() const;
private:
    EventPrivate *d;
};


template <typename T>
class BlockingQueue
{
public:
    explicit BlockingQueue(quint32 capacity);
    BlockingQueue() : BlockingQueue(UINT_MAX) {}
    ~BlockingQueue();
public:
    void setCapacity(quint32 capacity);
    bool put(const T &e);             // insert e to the tail of queue. blocked until not full.
    bool putForcedly(const T& e);     // insert e to the tail of queue ignoring capacity.
    bool returns(const T &e);         // like put() but insert e to the head of queue.
    bool returnsForcely(const T& e);  // like putForcedly() but insert e to the head of queue.
    T get();
    T peek();
    void clear();
    bool remove(const T &e);
public:
    inline bool isEmpty();
    inline bool isFull();
    inline quint32 capacity() const;
    inline quint32 size() const;
    inline quint32 getting() const;
    inline bool contains(const T &e);
private:
    QQueue<T> queue;
    Event notEmpty;
    Event notFull;
    QReadWriteLock lock;
    quint32 mCapacity;
    Q_DISABLE_COPY(BlockingQueue)
};


template <typename T>
BlockingQueue<T>::BlockingQueue(quint32 capacity)
    : mCapacity(capacity)
{
    notEmpty.clear();
    notFull.set();
}


template <typename T>
BlockingQueue<T>::~BlockingQueue()
{
//    if (queue.size() > 0) {
//        qtng_debug << "queue is free with element left.";
//    }
}


template <typename T>
void BlockingQueue<T>::setCapacity(quint32 capacity)
{
    lock.lockForWrite();
    this->mCapacity = capacity;
    if (static_cast<quint32>(queue.size()) >= mCapacity) {
        notFull.clear();
    } else {
        notFull.set();
    }
    lock.unlock();
}


template <typename T>
void BlockingQueue<T>::clear()
{
    lock.lockForWrite();
    this->queue.clear();
    notFull.set();
    notEmpty.clear();
    lock.unlock();
}


template <typename T>
bool BlockingQueue<T>::remove(const T &e)
{
    lock.lockForWrite();
    int n = this->queue.removeAll(e);
    if (n > 0) {
        if (this->queue.isEmpty()) {
            notEmpty.clear();
        } else {
            notEmpty.set();
        }
        if (static_cast<quint32>(queue.size()) >= mCapacity) {
            notFull.clear();
        } else {
            notFull.set();
        }
        lock.unlock();
        return true;
    } else {
        lock.unlock();
        return false;
    }
}


template <typename T>
bool BlockingQueue<T>::put(const T &e)
{
    if (!notFull.wait()) {
        return false;
    }
    lock.lockForWrite();
    queue.enqueue(e);
    notEmpty.set();
    if (static_cast<quint32>(queue.size()) >= mCapacity) {
        notFull.clear();
    }
    lock.unlock();
    return true;
}


template <typename T>
bool BlockingQueue<T>::putForcedly(const T& e)
{
    lock.lockForWrite();
    queue.enqueue(e);
    notEmpty.set();
    if (static_cast<quint32>(queue.size()) >= mCapacity) {
        notFull.clear();
    }
    lock.unlock();
    return true;
}


template <typename T>
bool BlockingQueue<T>::returns(const T &e)
{
    if (!notFull.wait()) {
        return false;
    }
    lock.lockForWrite();
    queue.prepend(e);
    notEmpty.set();
    if (static_cast<quint32>(queue.size()) >= mCapacity) {
        notFull.clear();
    }
    lock.unlock();
    return true;
}


template <typename T>
bool BlockingQueue<T>::returnsForcely(const T& e)
{
    lock.lockForWrite();
    queue.prepend(e);
    notEmpty.set();
    if (static_cast<quint32>(queue.size()) >= mCapacity) {
        notFull.clear();
    }
    lock.unlock();
    return true;
}


template <typename T>
T BlockingQueue<T>::get()
{
    if (!notEmpty.wait())
        return T();
    lock.lockForWrite();
    const T &e = queue.dequeue();
    if (this->queue.isEmpty()) {
        notEmpty.clear();
    }
    if (static_cast<quint32>(queue.size()) < mCapacity) {
        notFull.set();
    }
    lock.unlock();
    return e;
}


template <typename T>
T BlockingQueue<T>::peek()
{
    lock.lockForRead();
    if (this->queue.isEmpty()) {
        lock.unlock();
        return T();
    }
    const T &t = queue.head();
    lock.unlock();
    return t;
}


template <typename T>
inline bool BlockingQueue<T>::isEmpty()
{
    lock.lockForRead();
    bool t = queue.isEmpty();
    lock.unlock();
    return t;
}


template <typename T>
inline bool BlockingQueue<T>::isFull()
{
    lock.lockForRead();
    bool t = static_cast<quint32>(queue.size()) >= mCapacity;
    lock.unlock();
    return t;
}


template <typename T>
inline quint32 BlockingQueue<T>::capacity() const
{
    const_cast<BlockingQueue<T> *>(this)->lock.lockForRead();
    quint32 c = mCapacity;
    const_cast<BlockingQueue<T> *>(this)->lock.unlock();
    return c;
}


template <typename T>
inline quint32 BlockingQueue<T>::size() const
{
    const_cast<BlockingQueue<T> *>(this)->lock.lockForRead();
    int s = queue.size();
    const_cast<BlockingQueue<T> *>(this)->lock.unlock();
    return s;
}


template <typename T>
inline quint32 BlockingQueue<T>::getting() const
{
    const_cast<BlockingQueue<T> *>(this)->lock.lockForRead();
    int g = notEmpty.getting();
    const_cast<BlockingQueue<T> *>(this)->lock.unlock();
    return g;
}


template <typename T>
inline bool BlockingQueue<T>::contains(const T &e)
{
    const_cast<BlockingQueue<T> *>(this)->lock.lockForRead();
    bool t = queue.contains(e);
    const_cast<BlockingQueue<T> *>(this)->lock.unlock();
    return t;
}


#endif // BLOCKING_QUEUE_H
