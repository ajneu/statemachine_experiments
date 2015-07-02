TEMPLATE = app
CONFIG += c++14

TARGET = ping_pong

QT += core

INCLUDEPATH +=

HEADERS += interfacethread.h usereventtransition.h statemachine.h

HEADERS += interlayer.h   interlayer_connections.h   userevents.h   tptimer.h
SOURCES += interlayer.cpp interlayer_connections.cpp userevents.cpp tptimer.cpp

SOURCES += main.cpp
