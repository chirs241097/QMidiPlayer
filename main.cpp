#include "qmpmainwindow.hpp"
#include <QApplication>
#include <QStyle>
#include <QDesktopWidget>

int main(int argc, char *argv[])
{
	QApplication a(argc,argv);
	qmpMainWindow w;
	w.setGeometry(QStyle::alignedRect(
		Qt::LeftToRight,Qt::AlignCenter,w.size(),
		qApp->desktop()->availableGeometry()));
	w.show();

	return a.exec();
}
