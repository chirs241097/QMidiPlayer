#include <fluidsynth.h>
#include "qmphelpwindow.hpp"
#include "ui_qmphelpwindow.h"

static const char *months="JanFebMarAprMayJunJulAugSepOctNovDec";
std::string parseDate(const char *date)
{
	char ms[8];
	int y,d,m;sscanf(date,"%s %d %d",ms,&d,&y);
	m=(strstr(months,ms)-months)/3+1;
	char r[16];
	sprintf(r,"%04d-%02d-%02d",y,m,d);
	return std::string(r);
}

qmpHelpWindow::qmpHelpWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpHelpWindow)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
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
		s.replace("BUILD_DATE",parseDate(__DATE__).c_str());
		s.replace("BUILD_MACHINE",sss(BUILD_MACHINE));
		ui->textBrowser->setHtml(s);
	}
}
