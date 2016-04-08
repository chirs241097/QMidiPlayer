#-------------------------------------------------
#
# Project created by QtCreator 2015-12-25T20:24:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qmidiplayer
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
	qmpmainwindow.cpp \
	../core/qmpmidiplay.cpp \
	../core/qmpmidiread.cpp \
	qmpplistwindow.cpp \
	qmpchannelswindow.cpp \
	qmppresetselect.cpp \
	qmpchanneleditor.cpp \
	qmpefxwindow.cpp \
	qmpinfowindow.cpp \
	qmpsettingswindow.cpp \
	qmphelpwindow.cpp \
	qdialskulpturestyle.cpp \
	../core/qmpmidimapperrtmidi.cpp

HEADERS  += qmpmainwindow.hpp \
	../core/qmpmidiplay.hpp \
	qmpplistwindow.hpp \
	qmpchannelswindow.hpp \
	qmppresetselect.hpp \
	qmpchanneleditor.hpp \
	qmpefxwindow.hpp \
	qmpinfowindow.hpp \
	qmpsettingswindow.hpp \
	qmphelpwindow.hpp \
	qdialskulpturestyle.hpp \
	../core/qmpmidimappers.hpp

FORMS    += qmpmainwindow.ui \
	qmpplistwindow.ui \
	qmpchannelswindow.ui \
	qmppresetselect.ui \
	qmpchanneleditor.ui \
	qmpefxwindow.ui \
	qmpinfowindow.ui \
	qmpsettingswindow.ui \
	qmphelpwindow.ui

unix{
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	BINDIR = $$PREFIX/bin
	target.path = $$BINDIR
	INSTALLS += target
	QMAKE_CXXFLAGS += -Wall
	LIBS += -lfluidsynth -lrtmidi
}
win32{
	#change these before building...
	LIBS += e:/libs/fluidsynth/fluidsynth.lib winmm.lib
	LIBS += e:/libs/rtmidi/rtmidi.lib
	INCLUDEPATH += e:/libs/fluidsynth/include
	INCLUDEPATH += e:/libs/rtmidi
}
RESOURCES     = resources.qrc
