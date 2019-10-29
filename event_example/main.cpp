#include <QApplication>
#include <QPushButton>
#include <QThread>
#include "../blocking_queue.h"


class MyThread: public QThread
{
public:
    MyThread()
        :exiting(false) {}
    virtual void run() override;
    void stop();
public:
    Event event;
    bool exiting;
};


void MyThread::run()
{
    while (event.wait()) {
        if (exiting) {
            return;
        }
        qDebug("button clicked.");
        event.clear();
    }
}

void MyThread::stop()
{
    exiting = true;
    event.set();
    wait();
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
    
    app.exec();
    thread.stop();
    return 0;
}
