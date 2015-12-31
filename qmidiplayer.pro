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
	qmpChannelsWindow.cpp \
	qmppresetselect.cpp \
	qmpchanneleditor.cpp \
	qmpefxwindow.cpp \
    qmpinfowindow.cpp

HEADERS  += qmpmainwindow.hpp \
	qmpmidiplay.hpp \
	qmpplistwindow.hpp \
	qmpChannelsWindow.hpp \
	qmppresetselect.hpp \
	qmpchanneleditor.hpp \
	qmpefxwindow.hpp \
    qmpinfowindow.hpp

FORMS    += qmpmainwindow.ui \
	qmpplistwindow.ui \
	qmpChannelsWindow.ui \
	qmppresetselect.ui \
	qmpchanneleditor.ui \
	qmpefxwindow.ui \
    qmpinfowindow.ui

QMAKE_CXXFLAGS += -std=c++11 -Wall
LIBS += -lfluidsynth
RESOURCES     = resources.qrc
