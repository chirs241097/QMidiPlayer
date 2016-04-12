#include <fluidsynth.h>
#include "qmphelpwindow.hpp"
#include "ui_qmphelpwindow.h"

qmpHelpWindow::qmpHelpWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpHelpWindow)
{
	ui->setupUi(this);
	ui->textBrowser->setSearchPaths(QStringList(QString(":/doc"))+QStringList(QString(":/img")));
	ui->textBrowser->setSource(QUrl("qrc:///doc/index_internal.html"));
}

qmpHelpWindow::~qmpHelpWindow()
{
	delete ui;
}

void qmpHelpWindow::on_textBrowser_sourceChanged(const QUrl &src)
{
	if(src.fileName()==QString("version_internal.html"))
	{
		QString s=ui->textBrowser->toHtml();
		s.replace("CT_QT_VERSION_STR",QT_VERSION_STR);
		s.replace("RT_QT_VERSION_STR",qVersion());
		s.replace("CT_FLUIDSYNTH_VERSION",FLUIDSYNTH_VERSION);
		s.replace("RT_FLUIDSYNTH_VERSION",fluid_version_str());
		s.replace("APP_VERSION",APP_VERSION);
		ui->textBrowser->setHtml(s);
	}
}
