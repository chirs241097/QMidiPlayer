#include "qmphelpwindow.hpp"
#include "ui_qmphelpwindow.h"

qmpHelpWindow::qmpHelpWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpHelpWindow)
{
	ui->setupUi(this);
	ui->textBrowser->setSearchPaths(QStringList(QString(":/doc"))+QStringList(QString(":/img")));
	ui->textBrowser->setSource(QUrl("qrc:///doc/index.html"));
}

qmpHelpWindow::~qmpHelpWindow()
{
	delete ui;
}

void qmpHelpWindow::on_textBrowser_sourceChanged(const QUrl &src)
{
	if(src.fileName()==QString("version.html"))
	{
		QString s=ui->textBrowser->toHtml();
		s.replace("QT_VERSION_STR",QT_VERSION_STR);
		s.replace("APP_VERSION",APP_VERSION);
		ui->textBrowser->setHtml(s);
	}
}
