TEMPLATE = app

QT += qml quick
unix:QT += widgets
win32:QT += widgets
CONFIG += c++11

SOURCES += ./qmidiplayerlite.src.d/main.cpp \
	./common/qmpmidiplay.cpp \
	./common/qmpmidiread.cpp

RESOURCES += ./qmidiplayerlite.src.d/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(./qmidiplayerlite.src.d/deployment.pri)

HEADERS += \
	./common/qmpmidiplay.hpp \
	./qmidiplayerlite.src.d/qmpcorewrapper.hpp

LIBS += -lfluidsynth
