#-------------------------------------------------
#
# Project created by QtCreator 2016-05-19T09:25:32
#
#-------------------------------------------------

QT       -= core gui

CONFIG += c++11

TARGET = sample-plugin
TEMPLATE = lib

DEFINES += SAMPLEPLUGIN_LIBRARY

SOURCES += sampleplugin.cpp

HEADERS += sampleplugin.hpp

unix {
	target.path = /usr/lib
	INSTALLS += target
}
