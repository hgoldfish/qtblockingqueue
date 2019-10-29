#include <QtCore/qwaitcondition.h>
#include <QtCore/qmutex.h>
#include "blocking_queue.h"


class EventPrivate
{
public:
    EventPrivate();
    QWaitCondition condition;
    QMutex mutex;
    QAtomicInteger<bool> flag;
    QAtomicInteger<quint32> waiters;
};


EventPrivate::EventPrivate()
    : flag(false)
    , waiters(0)
{}


Event::Event()
    :d_ptr(new EventPrivate())
{
}


void Event::set()
{
    Q_D(Event);
    d->mutex.lock();
    if (!d->flag.load()) {
        d->flag.store(true);
        d->condition.wakeAll();
    }
    d->mutex.unlock();
}


void Event::clear()
{
    Q_D(Event);
    d->flag.store(false);
}


bool Event::wait(unsigned long time)
{
    Q_D(Event);
    if (!d->flag.load()) {
        d->mutex.lock();
        if (!d->flag.load()) {
            ++d->waiters;
            d->condition.wait(&d->mutex, time);
            --d->waiters;
        }
        d->mutex.unlock();
    }
    return d->flag.load();
}


bool Event::isSet() const
{
    Q_D(const Event);
    return d->flag.load();
}


quint32 Event::getting() const
{
    Q_D(const Event);
    return d->waiters.load();
}
