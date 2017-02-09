#-------------------------------------------------
#
# Project created by QtCreator 2017-02-09T09:24:43
#
#-------------------------------------------------

QT       -= core gui

CONFIG += c++11

TARGET = midifmt-plugin
TEMPLATE = lib

DEFINES += MIDIFMTPLUGIN_LIBRARY

SOURCES += midifmtplugin.cpp

HEADERS += midifmtplugin.hpp

unix {
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3
	QMAKE_LFLAGS_RELEASE -= -O1
	QMAKE_LFLAGS_RELEASE += -O3
	target.path = $$PREFIX/lib/qmidiplayer
	INSTALLS += target
}
