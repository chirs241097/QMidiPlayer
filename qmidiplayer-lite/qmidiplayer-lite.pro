TEMPLATE = app

QT += qml quick
unix:QT += widgets
win32:QT += widgets
CONFIG += c++11

SOURCES += main.cpp \
	../core/qmpmidiplay.cpp \
	../core/qmpmidiread.cpp \
	../core/qmpmidimapperrtmidi.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	../core/qmpmidiplay.hpp \
	qmpcorewrapper.hpp \
	../core/qmpmidimappers.hpp
unix{
LIBS += -lfluidsynth -lrtmidi
}

win32{
	#change these before building...
	LIBS += e:/libs/fluidsynth/fluidsynth.lib winmm.lib
	Release:LIBS += e:/libs/rtmidi/rtmidi.lib
	Debug:LIBS += e:/libs/rtmidi/rtmidid.lib
	INCLUDEPATH += e:/libs/fluidsynth/include
	INCLUDEPATH += e:/libs/rtmidi
}
