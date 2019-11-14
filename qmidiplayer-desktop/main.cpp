/*
 *QMidiPlayer, a cross-platform player for midi files.
 *Copyright (C) 2015  Chris Xiong
 *
 *This program is free software: you can redistribute it and/or modify
 *it under the terms of the GNU General Public License as published by
 *the Free Software Foundation, either version 3 of the License, or
 *(at your option) any later version.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "qmpmainwindow.hpp"
#include <QApplication>
#include <QStyle>
#include <QTranslator>
#include <QLibraryInfo>
#include <QCommandLineParser>
#ifdef _WIN32
#include <windows.h>
#endif

int main(int argc,char **argv)
{
#ifdef _WIN32
	FreeConsole();
#endif
	QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QCoreApplication::setApplicationName("qmidiplayer");
	QCoreApplication::setApplicationVersion(APP_VERSION);
	if(!qgetenv("QT_SCALE_FACTOR").length()&&!qgetenv("QT_SCREEN_SCALE_FACTORS").length())
	QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication a(argc,argv);

	QTranslator qtTranslator;
	qtTranslator.load("qt_"+QLocale::system().name(),
					  QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	a.installTranslator(&qtTranslator);
	QTranslator qmpTranslator;
	qmpTranslator.load("qmp_"+QLocale::system().name());
	a.installTranslator(&qmpTranslator);

	QCommandLineParser clp;
	clp.setApplicationDescription("A cross-platform MIDI player.");
	clp.addHelpOption();
	clp.addVersionOption();
	clp.addPositionalArgument("file",QCoreApplication::translate("main","midi files to play (optional)."),"[files...]");
	clp.addOption(QCommandLineOption("plugin",QCoreApplication::translate("main","Load a plugin from <plugin library>."),"plugin library"));
	clp.addOption(QCommandLineOption({"l","load-all-files"},QCoreApplication::translate("main","Load all files from the same folder.")));
	clp.process(a);

	qmpMainWindow w(&clp);
	if(w.parseArgs()==1)return 0;
	w.init();

	return a.exec();
}
