TEMPLATE = app

QT += qml quick
unix:QT += widgets
win32:QT += widgets
CONFIG += c++11

SOURCES += main.cpp \
	../core/qmpmidiplay.cpp \
    ../core/qmpmidiread.cpp \
    ../core/qmpmidioutfluid.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
	../core/qmpmidiplay.hpp \
	qmpcorewrapper.hpp \
    ../core/qmpmidioutfluid.hpp \
	../include/qmpcorepublic.hpp
unix{
LIBS += -lfluidsynth -lrtmidi
exists("/usr/include/RtMidi.h") {
	DEFINES += RT_MIDI_H=\\\"/usr/include/RtMidi.h\\\"
}
exists("/usr/include/rtmidi/RtMidi.h") {
	DEFINES += RT_MIDI_H=\\\"/usr/include/rtmidi/RtMidi.h\\\"
}
}

win32{
	DEFINES += RT_MIDI_H=\\\"RtMidi.h\\\"
	#change these before building...
	LIBS += e:/libs/fluidsynth/fluidsynth.lib winmm.lib
	Release:LIBS += e:/libs/rtmidi/rtmidi.lib
	Debug:LIBS += e:/libs/rtmidi/rtmidid.lib
	INCLUDEPATH += e:/libs/fluidsynth/include
	INCLUDEPATH += e:/libs/rtmidi
}
