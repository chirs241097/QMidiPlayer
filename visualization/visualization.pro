#-------------------------------------------------
#
# Project created by QtCreator 2016-04-24T22:04:32
#
#-------------------------------------------------

QT       -= core gui

CONFIG += c++11

TARGET = visualization
TEMPLATE = lib

DEFINES += VISUALIZATION_LIBRARY

SOURCES += qmpvisualization.cpp

HEADERS += qmpvisualization.hpp

unix {
	target.path = /usr/lib/qmidiplayer
	INSTALLS += target
}
