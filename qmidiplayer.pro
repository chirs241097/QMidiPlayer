#-------------------------------------------------
#
# Project created by QtCreator 2015-12-25T20:24:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qmidiplayer
TEMPLATE = app


SOURCES += ./qmidiplayer.src.d/main.cpp\
	./qmidiplayer.src.d/qmpmainwindow.cpp \
	./common/qmpmidiplay.cpp \
	./common/qmpmidiread.cpp \
	./qmidiplayer.src.d/qmpplistwindow.cpp \
	./qmidiplayer.src.d/qmpchannelswindow.cpp \
	./qmidiplayer.src.d/qmppresetselect.cpp \
	./qmidiplayer.src.d/qmpchanneleditor.cpp \
	./qmidiplayer.src.d/qmpefxwindow.cpp \
	./qmidiplayer.src.d/qmpinfowindow.cpp \
	./qmidiplayer.src.d/qmpsettingswindow.cpp \
	./qmidiplayer.src.d/qmphelpwindow.cpp \
	./qmidiplayer.src.d/qdialskulpturestyle.cpp

HEADERS  += ./qmidiplayer.src.d/qmpmainwindow.hpp \
	./common/qmpmidiplay.hpp \
	./qmidiplayer.src.d/qmpplistwindow.hpp \
	./qmidiplayer.src.d/qmpchannelswindow.hpp \
	./qmidiplayer.src.d/qmppresetselect.hpp \
	./qmidiplayer.src.d/qmpchanneleditor.hpp \
	./qmidiplayer.src.d/qmpefxwindow.hpp \
	./qmidiplayer.src.d/qmpinfowindow.hpp \
	./qmidiplayer.src.d/qmpsettingswindow.hpp \
	./qmidiplayer.src.d/qmphelpwindow.hpp \
	./qmidiplayer.src.d/qdialskulpturestyle.hpp \
	./qmidiplayer.src.d/qmpimidimapper.hpp

FORMS    += ./qmidiplayer.src.d/qmpmainwindow.ui \
	./qmidiplayer.src.d/qmpplistwindow.ui \
	./qmidiplayer.src.d/qmpchannelswindow.ui \
	./qmidiplayer.src.d/qmppresetselect.ui \
	./qmidiplayer.src.d/qmpchanneleditor.ui \
	./qmidiplayer.src.d/qmpefxwindow.ui \
	./qmidiplayer.src.d/qmpinfowindow.ui \
	./qmidiplayer.src.d/qmpsettingswindow.ui \
	./qmidiplayer.src.d/qmphelpwindow.ui

unix{
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	BINDIR = $$PREFIX/bin
	target.path = $$BINDIR
	INSTALLS += target
	QMAKE_CXXFLAGS += -std=c++11 -Wall
	LIBS += -lfluidsynth
}
win32:LIBS += e:/libs/fluidsynth/fluidsynth.lib winmm.lib #You have to change these
win32:INCLUDEPATH += e:/libs/fluidsynth/include           #before building...
RESOURCES     = ./qmidiplayer.src.d/resources.qrc
