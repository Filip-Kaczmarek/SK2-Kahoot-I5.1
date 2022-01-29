QT += core network gui widgets

TARGET = Tworca
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += debug_and_release

SOURCES += \
    clientcreator.cpp \
    clientmain.cpp \
    creatorwindow.cpp \
    dataencoder.cpp \
    dataparser.cpp

FORMS += \
    creatorwindow.ui

HEADERS += \
    clientcreator.h \
    creatorwindow.h \
    dataencoder.h \
    dataparser.h \
    protocolmsg.h
