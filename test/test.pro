TEMPLATE = app
QT =
QT	   -= gui
CONFIG   += console
CONFIG   -= app_bundle

TARGET = tst_ezlog
include(../src/libezlog.pri)

#win32:LIBS += -lUser32

SOURCES += \
    main.cpp
HEADERS +=
unix: LIBS += -lpthread
