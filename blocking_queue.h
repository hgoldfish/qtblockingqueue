#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <QtCore/qqueue.h>
#include <QtCore/qmutex.h>

class EventPrivate;
class Event
{
public:
    Event();
public:
    void set();
    void clear();
    bool wait(unsigned long time = ULONG_MAX);
    bool isSet() const;
    quint32 getting() const;
private:
    EventPrivate *d_ptr;
    Q_DECLARE_PRIVATE(Event)
};


template<typename T>
class BlockingQueue
{
public:
    BlockingQueue(quint32 capacity);
    BlockingQueue() : BlockingQueue(UINT_MAX) {}
public:
    void setCapacity(quint32 capacity);
    T get();
    inline T peek();
    void put(const T &t);            // insert e to the tail of queue. blocked until not full.
    void putForcedly(const T& t);    // insert e to the tail of queue ignoring capacity.
    void returns(const T &e);        // like put() but insert e to the head of queue.
    void returnsForcely(const T &t);

    void clear();
    bool contains(const T &e) const;
    bool remove(const T &e);

    inline bool isEmpty() const;
    inline bool isFull() const;
    quint32 capacity() const { return mCapacity; }
    unsigned int size() const;
private:
    Event notFull;
    Event notEmpty;
    QQueue<T> queue;
    QMutex lock;
    quint32 mCapacity;
};


template<typename T>
BlockingQueue<T>::BlockingQueue(quint32 capacity)
    : lock(QMutex::Recursive), mCapacity(capacity)
{
    notFull.set();
    notEmpty.clear();
}


template<typename T>
void BlockingQueue<T>::setCapacity(quint32 capacity)
{
    lock.lock();
    this->mCapacity = capacity;
    if (isFull()) {
        notFull.clear();
    } else {
        notFull.set();
    }
    lock.unlock();
}


template<typename T>
void BlockingQueue<T>::clear()
{
    lock.lock();
    this->queue.clear();
    notFull.set();
    notEmpty.clear();
    lock.unlock();
}


template<typename T>
bool BlockingQueue<T>::contains(const T &e) const
{
    return queue.contains(e);
}


template<typename T>
unsigned int BlockingQueue<T>::size() const
{
    return queue.size();
}


template<typename T>
bool BlockingQueue<T>::remove(const T &e)
{
    lock.lock();
    int n = this->queue.removeAll(e);
    if (n > 0) {
        if (isEmpty()) {
            notEmpty.clear();
        } else {
            notEmpty.set();
        }
        if (isFull()) {
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


template<typename T>
void BlockingQueue<T>::put(const T &e)
{
    notFull.wait();
    lock.lock();
    queue.enqueue(e);
    notEmpty.set();
    if (isFull()) {
        notFull.clear();
    }
    lock.unlock();
}


template<typename T>
void BlockingQueue<T>::putForcedly(const T& e)
{
    lock.lock();
    queue.enqueue(e);
    notEmpty.set();
    if (isFull()) {
        notFull.clear();
    }
    lock.unlock();
}


template<typename T>
void BlockingQueue<T>::returns(const T &e)
{
    notFull.wait();
    lock.lock();
    queue.prepend(e);
    notEmpty.set();
    if (isFull()) {
        notFull.clear();
    }
    lock.unlock();
}


template<typename T>
void BlockingQueue<T>::returnsForcely(const T& e)
{
    lock.lock();
    queue.prepend(e);
    notEmpty.set();
    if (isFull()) {
        notFull.clear();
    }
    lock.unlock();
}


template<typename T>
T BlockingQueue<T>::get()
{
    if (!notEmpty.wait())
        return T();
    lock.lock();
    const T &e = queue.dequeue();
    if (isEmpty()) {
        notEmpty.clear();
    }
    if (!isFull()) {
        notFull.set();
    }
    lock.unlock();
    return e;
}


template<typename T>
inline T BlockingQueue<T>::peek()
{
    lock.lock();
    if (isEmpty()) {
        return T();
    }
    T e = queue.head();    // not safe to returns reference, because clear() and get() can delete the element.
    lock.unlock();
    return e;
}


template<typename T>
inline bool BlockingQueue<T>::isEmpty() const
{
    return queue.isEmpty();
}


template<typename T>
inline bool BlockingQueue<T>::isFull() const
{
    return static_cast<quint32>(queue.size()) >= mCapacity;
}


#endif // BLOCKING_QUEUE_H
