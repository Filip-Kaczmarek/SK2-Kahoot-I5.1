QT += core network gui widgets

TARGET = Gracz
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += debug_and_release

SOURCES += \
    clientmain.cpp \
    clientplayer.cpp \
    dataencoder.cpp \
    dataparser.cpp \
    playerwindow.cpp

FORMS += \
    playerwindow.ui

HEADERS += \
    clientplayer.h \
    dataencoder.h \
    playerwindow.h \
    protocolmsg.h \
    dataparser.h
