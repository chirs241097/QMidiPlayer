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
	qmpplugin.cpp \
    qmpcustomizewindow.cpp \
    ../core/qmpmidioutrtmidi.cpp \
    ../core/qmpmidioutfluid.cpp \
    qmpdevpropdialog.cpp

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
	../include/qmpcorepublic.hpp \
	qmpplugin.hpp \
    qmpcustomizewindow.hpp \
    ../core/qmpmidioutrtmidi.hpp \
    ../core/qmpmidioutfluid.hpp \
    qmpdevpropdialog.hpp

FORMS    += qmpmainwindow.ui \
	qmpplistwindow.ui \
	qmpchannelswindow.ui \
	qmppresetselect.ui \
	qmpchanneleditor.ui \
	qmpefxwindow.ui \
	qmpinfowindow.ui \
	qmpsettingswindow.ui \
	qmphelpwindow.ui \
    qmpcustomizewindow.ui \
    qmpdevpropdialog.ui

TRANSLATIONS += translations/qmp_zh_CN.ts
DEFINES += BUILD_MACHINE=$${QMAKE_HOST.name}

unix{
	isEmpty(PREFIX) {
		PREFIX = /usr/local
	}
	BUILDM = $$(QMP_BUILD_MODE)
	isEmpty(BUILDM) {
		message(Building in normal(debugging) mode...)
	}
	else {
		message(Building in packaging mode...)
		DEFINES += QMP_BUILD_UNIX_PACKAGE
	}
	exists("/usr/include/RtMidi.h") {
		DEFINES += RT_MIDI_H=\\\"/usr/include/RtMidi.h\\\"
	}
	exists("/usr/include/rtmidi/RtMidi.h") {
		DEFINES += RT_MIDI_H=\\\"/usr/include/rtmidi/RtMidi.h\\\"
	}
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3
	QMAKE_LFLAGS_RELEASE -= -O1
	QMAKE_LFLAGS_RELEASE += -O3
	BINDIR = $$PREFIX/bin
	target.path = $$BINDIR
	DATADIR = $$PREFIX/share
	INSTALLS += target desktop iconbmp iconsvg iconxpm doc docimg appdata menu mimetype
	desktop.path = $$DATADIR/applications
	desktop.files += $${TARGET}.desktop
	iconbmp.path = $$DATADIR/icons/hicolor/64x64/apps
	iconbmp.files += ../img/qmidiplyr.png
	iconsvg.path = $$DATADIR/icons/hicolor/scalable/apps
	iconsvg.files += ../img/qmidiplyr.svg
	iconxpm.path = $$DATADIR/pixmaps
	iconxpm.files += ../img/qmidiplyr.xpm
	doc.path = $$DATADIR/qmidiplayer/doc
	doc.files += ../doc/*
	docimg.path = $$DATADIR/qmidiplayer/img
	docimg.files += ../img/mainw.png ../img/chanw.png ../img/chparaw.png ../img/qmidiplyr.png
	appdata.path = $${DATADIR}/appdata
	appdata.files += $${TARGET}.appdata.xml
	menu.path = $${DATADIR}/menu
	menu.files += ./menu/$${TARGET}
	mimetype.path = $$DATADIR/mime/packages
	mimetype.files += $${TARGET}.mime
	QMAKE_CXXFLAGS += -Wall
	LIBS += -lfluidsynth -lrtmidi -ldl
}
win32{
	DEFINES += RT_MIDI_H=\\\"RtMidi.h\\\"
	#change these before building...
	LIBS += -lfluidsynth -lwinmm -lRtMidi
	RC_FILE = qmidiplayer.rc
}
RESOURCES     = resources.qrc
