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
	target.path = /usr/lib/qmidiplayer
	DATADIR = $$PREFIX/share
	INSTALLS += target res
	QMAKE_CXXFLAGS += -pthread -fPIC
	QMAKE_CXXFLAGS_RELEASE -= -O2
	QMAKE_CXXFLAGS_RELEASE += -O3
	QMAKE_LFLAGS_RELEASE -= -O1
	QMAKE_LFLAGS_RELEASE += -O3
	res.path = $$DATADIR/qmidiplayer/img
	res.files += ../img/chequerboard.png ../img/particle.png ../img/kb_128.png
	#well...
	INCLUDEPATH += /home/chrisoft/devel/SMELT/include/ /usr/include/freetype2
	LIBS += -L/home/chrisoft/devel/SMELT/smelt/glfw/
	LIBS += -L/home/chrisoft/devel/SMELT/extensions/
	LIBS += -lstdc++ -lfreetype -lz -lsmeltext -lsmelt-dumb -lCxImage -ljpeg -lpng -lglfw -lGLEW -lGL
}
win32 {
		#Change these before producing your own build!
		INCLUDEPATH += "C:\Users\Chris Xiong\Documents\devel\SMELT\include"
		INCLUDEPATH += "C:\Users\Chris Xiong\Documents\devel\freetype-2.7\include"
		INCLUDEPATH += "C:\Users\Chris Xiong\Documents\devel\zlib-1.2.8"
		CONFIG(release, debug|release){
			LIBS += -L"C:\Users\Chris Xiong\Documents\devel\SMELT\msvc\libdeps"
			LIBS += -lfreetype27MT -lzlib -lsmeltext -lsmelt -ljpeg-static -llibpng16 -lglfw3 -lglew32s -lopengl32 -luser32 -lgdi32 -lshell32
		}else{
			LIBS += -L"C:\Users\Chris Xiong\Documents\devel\SMELT\msvc\libdepsd"
			LIBS += -lfreetype27MTd -lzlib -lsmeltextd -lsmeltd -ljpeg-static -llibpng16 -lglfw3 -lglew32sd -lopengl32 -luser32 -lgdi32 -lshell32
		}
}
