#-------------------------------------------------
#
# Project created by QtCreator 2015-12-25T20:24:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qmidiplayer
TEMPLATE = app


SOURCES += main.cpp\
		qmpmainwindow.cpp \
	qmpmidiplay.cpp \
	qmpmidiread.cpp \
    qmpplistwindow.cpp \
    qmpchannelswindow.cpp

HEADERS  += qmpmainwindow.hpp \
	qmpmidiplay.hpp \
    qmpplistwindow.hpp \
    qmpchannelswindow.hpp

FORMS    += qmpmainwindow.ui \
    qmpplistwindow.ui \
    qmpchannelswindow.ui

QMAKE_CXXFLAGS += -std=c++11 -Wall
LIBS += -lfluidsynth
RESOURCES     = resources.qrc
