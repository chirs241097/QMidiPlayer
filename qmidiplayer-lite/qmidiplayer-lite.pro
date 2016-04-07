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

LIBS += -lfluidsynth -lrtmidi
