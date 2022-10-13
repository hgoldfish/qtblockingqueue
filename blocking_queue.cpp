#include <QtCore/qwaitcondition.h>
#include <QtCore/qmutex.h>
#include "blocking_queue.h"


class EventPrivate
{
public:
    EventPrivate();
    inline void incref();
    inline bool decref();
    bool wait(unsigned long time);
public:
    QWaitCondition condition;
    QMutex mutex;
    QAtomicInteger<bool> flag;
    QAtomicInteger<int> ref;
    QAtomicInteger<quint32> waiters;
};


EventPrivate::EventPrivate()
    : flag(false)
    , ref(1)
    , waiters(0)
{}


void EventPrivate::incref()
{
    ref.ref();
}


bool EventPrivate::decref()
{
    if (!ref.deref()) {
        delete this;
        return false;
    }
    return true;
}


bool EventPrivate::wait(unsigned long time)
{
    bool f = flag.loadAcquire();
    if (time == 0 || f) {
        return f;
    }

    incref();
    mutex.lock();
    Q_ASSERT(!f);
    ++waiters;
    while (!(f = flag.loadAcquire()) && ref.loadAcquire() > 1) {
        condition.wait(&mutex);
    }
    --waiters;
    mutex.unlock();
    decref();
    return f;
}


Event::Event()
    :d(new EventPrivate()) {}


Event::~Event()
{
    if (d->decref()) {
        d->condition.wakeAll();
    }
    d = nullptr;
}


void Event::set()
{
    if (d) {
        if (d->flag.fetchAndStoreAcquire(true)) {
            return;
        }
        d->incref();
        if (d->waiters.loadAcquire() > 0) {
            d->condition.wakeAll();
        }
        d->decref();
    }
}


void Event::clear()
{
    if (d) {
        d->flag.storeRelease(false);
    }
}


bool Event::wait(unsigned long time)
{
    if (d) {
        return d->wait(time);
    } else {
        return false;
    }
}


bool Event::isSet() const
{
    if (d) {
        return d->flag.loadAcquire();
    } else {
        return false;
    }
}


quint32 Event::getting() const
{
    if (d) {
        return d->waiters.loadAcquire();
    } else {
        return 0;
    }
}
