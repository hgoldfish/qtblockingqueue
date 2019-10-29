TEMPLATE = app
TARGET = queue_example
INCLUDEPATH += .
QT += widgets
DEFINES += QT_DEPRECATED_WARNINGS
SOURCES += main.cpp ../blocking_queue.cpp
