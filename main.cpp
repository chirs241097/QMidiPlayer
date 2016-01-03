#include "qmpmainwindow.hpp"
#include <QApplication>
#include <QStyle>

int main(int argc, char *argv[])
{
	QApplication a(argc,argv);
	qmpMainWindow w;
	w.show();

	return a.exec();
}
