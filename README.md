Qt Blocking Queue
=================

Passing data through threads is a common task in multi-thread programming. The Qt toolkit does provide a `QQueue` class, and calling slots via `QMetaObject::invokdeMethod(Qt::BlockingQueuedConnection)`. But you might miss the more convenient `BlockingQueue` in Java/Python.

This project implements the `Event` and `BlockingQueue` in two files. You can copy these two files into your projects.

An example using `Event` (The complete code is in the `event_example` directory). In this example, the child thread waits for user clicking, then prints a message.

    #include <QApplication>
    #include <QPushButton>
    #include <QThread>
    #include "blocking_queue.h"

    class MyThread: public QThread
    {
    public:
        virtual void run() override;
    public:
        Event event;
    };


    void MyThread::run()
    {
        while (event.wait()) {
            qDebug("button clicked.");
            event.clear();
        }
    }

    int main(int argc, char **argv)
    {
        QApplication app(argc, argv);

        MyThread thread;
        thread.start();

        QPushButton button("click here!");
        button.show();
        QObject::connect(&button, &QPushButton::clicked, [&thread] {
            thread.event.set();
        });
        
        return app.exec();
    }
    
Another example using `BlockingQueue` to implement the "Product/Consume" design pattern. In this example, we start 4 consumer threads to consume the message producted by the GUI thread.

    #include <QCoreApplication>
    #include <QThread>
    #include <QSharedPointer>
    #include "../blocking_queue.h"

    typedef BlockingQueue<QByteArray> MessageQueue;

    class ConsumeThread: public QThread
    {
    public:
        ConsumeThread(QSharedPointer<MessageQueue> messages)
            :messages(messages) {}
        virtual void run() override;
    public:
        QSharedPointer<MessageQueue> messages;
        int index;
    };

    void ConsumeThread::run()
    {
        while(true) {
            const QByteArray &message = messages->get();
            if (message.isEmpty()) {
                break;
            }
            qDebug("consumer %d consume message: %s", index, qPrintable(message));
        }
    }


    int main(int argc, char **argv)
    {
        QCoreApplication app(argc, argv);
        QList<ConsumeThread*> threads;
        QSharedPointer<MessageQueue> messages = QSharedPointer<MessageQueue>::create();
        for (int i = 0; i < 4; ++i) {
            ConsumeThread *thread = new ConsumeThread(messages);
            thread->index = i;
            thread->start();
            threads.append(thread);
        }
        for (int i = 0; i < 1e8; ++i) {
            messages->put("New Message");
        }
        return 0;
    }
    

Reference
=========

class Event
-----------

* bool wait(unsigned long time = ULONG_MAX);

    Wait for the event. Block current thread until the `time` in milliseconds elasped. If it times out, this function returns false.
    
* void set();

    Set up the event, and wake up all waiters.
    
* void clear();

    Clear the event. All waiters which have previous awake are unaffected, and the new waiters will be blocked.

* bool isSet() const;

    Returns true if this event is set.
    
* quint32 getting() const;

    Returns the number of blocked waiters.

class BlockQueue
----------------

* BlockQueue()

    Construct a blocking queue, with an unlimit (UINT_MAX) capacity.
    
* BlockingQueue(quint32 capacity);

    Construct a blocking queue, with the `capacity` limit. If the queue is full, the `put()` function blocks the calling thread. The `get()` function always blocks the calling thread when the queue is empty.

* void put(const T &t);

    Insert an element at the end of the queue. If the queue is full, the current thread will be blocked until another thread pops an element from it.
    
* T get();

    Gets an element from the begining of the queue and returns the element. If the queue is empty, the current thread will be blocked until another thread puts an element into it.
    
* T peek();

    Gets an elelemt from queue head, without removing from queue. If the queue is empty, returns an empty value. This function never block the current thread.
    
    
* void returns(const T &e);

    Insert an element at the begining of the queue and return the element. If the queue is full, the current thread will be blocked until another thread pops an elment from it. The function is similar to the `put()` function, but different at the position of the queue.
    
* void putForcedly(const T& t);

    Insert an element at the end of the queue, regardless of whether the queue is full. This function never block the current thread.
    
* void returnsForcely(const T &t);

    Insert an element at the begining of the queue, regardless of wheter the queue is full. This function never block the current thread.
    
* void clear();

    Remove all elements from the queue. This function never block the current thread.

* bool contains(const T &e) const;

    Returns true if the queue contains the element `e`.
    
* bool remove(const T &e);

    Remove the elelemt `e` from the queue. This function never block the current thread.
    
* bool isEmpty() const;

    Return true if the queue is empty.
    
* bool isFull() const;

    Return true if the queue is full.
    
* unsigned int size() const;

    Return the queue size.
    
* quint32 capacity() const

    Return the queue capacity. The queue size might be larger than the capacity because the `putForcedly()` always ignores this capacity limit.
    
    
License
=======

This project is licensed with LGPL v3.0 with static link exception.



