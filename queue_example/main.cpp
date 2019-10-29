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
