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

SOURCES += qmpvisualization.cpp \
	extrasmeltutils.cpp \
	qmpvirtualpiano3d.cpp

HEADERS += qmpvisualization.hpp \
	extrasmeltutils.hpp \
	qmpvirtualpiano3d.hpp

unix {
	target.path = $$PREFIX/lib/qmidiplayer
	DATADIR = $$PREFIX/share
	INSTALLS += target res
	QMAKE_CXXFLAGS += -pthread -fPIC
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3
	QMAKE_LFLAGS_RELEASE -= -O1
	QMAKE_LFLAGS_RELEASE += -O3
	res.path = $$DATADIR/qmidiplayer/img
	res.files += ../img/chequerboard.png ../img/particle.png ../img/kb_128.png
	isEmpty(SMELTDIR){
		INCLUDEPATH += /home/chrisoft/devel/SMELT/include/ /usr/include/freetype2
		LIBS += -L/home/chrisoft/devel/SMELT/smelt/glfw/
		LIBS += -L/home/chrisoft/devel/SMELT/extensions/
		LIBS += -lstdc++ -lfreetype -lz -lsmeltext -lsmelt-dumb -lCxImage -ljpeg -lpng -lglfw -lGLEW -lGL
	}else{
		INCLUDEPATH += $$(SMELT_DIR)/include/ /usr/include/freetype2
		LIBS += -L$$(SMELT_DIR)/smelt/glfw/
		LIBS += -L$$(SMELT_DIR)/extensions/
		LIBS += -lstdc++ -lfreetype -lz -lsmeltext -lsmelt-dumb -lCxImage -ljpeg -lpng -lglfw -lGLEW -lGL
	}
}
win32 {
		#Change these before producing your own build!
		INCLUDEPATH += $$(SMELT_DIR)/include/
		INCLUDEPATH += /home/chrisoft/devel/mingwlibs/mxe/usr/x86_64-w64-mingw32.shared/include/freetype2/
		LIBS += -L$$(SMELT_DIR)/smelt/glfw/
		LIBS += -L$$(SMELT_DIR)/extensions/
		LIBS += -lz -lsmeltext -lfreetype -lsmelt -lCxImage -ljpeg -lpng16 -lglfw3 -lglew32s -lopengl32 -luser32 -lgdi32 -lshell32
}
