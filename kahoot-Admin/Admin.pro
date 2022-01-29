QT += core network gui widgets

TARGET = Admin
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += debug_and_release

SOURCES += \
    adminwindow.cpp \
    clientadmin.cpp \
    clientmain.cpp \
    dataencoder.cpp \
    dataparser.cpp

FORMS += \
    adminwindow.ui

HEADERS += \
    adminwindow.h \
    clientadmin.h \
    dataencoder.h \
    protocolmsg.h \
    dataparser.h
