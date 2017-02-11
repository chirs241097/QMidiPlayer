#include <map>
#include <set>
#include <vector>
#include <QSettings>
#include <QList>
#include <QVariant>
#include "qmpmainwindow.hpp"
#include "qmpcustomizewindow.hpp"
#include "ui_qmpcustomizewindow.h"

qmpCustomizeWindow::qmpCustomizeWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpCustomizeWindow)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMinimumWidth(w);setMinimumHeight(h);
	QSettings *s=qmpMainWindow::getInstance()->getSettingsWindow()->getSettingsIntf();
	QList<QVariant> defa={"FileInfo","Render","Panic","ReloadSynth"};
	QList<QVariant> defb={"Channel","Playlist","Effects","Visualization"};
	QList<QVariant> a=s->value("Behavior/Actions",QVariant(defa)).toList();
	QList<QVariant> b=s->value("Behavior/Toolbar",QVariant(defb)).toList();
	std::vector<std::string>& v=qmpMainWindow::getInstance()->getWidgets(1);
	v.clear();
	for(int i=0;i<a.size();++i)
	v.push_back(a[i].toString().toStdString());
	std::vector<std::string>& vv=qmpMainWindow::getInstance()->getWidgets(0);
	vv.clear();
	for(int i=0;i<b.size();++i)
	vv.push_back(b[i].toString().toStdString());
}

qmpCustomizeWindow::~qmpCustomizeWindow()
{
	delete ui;
}

void qmpCustomizeWindow::launch(int w)
{
	show();
	ui->lwAvail->clear();
	ui->lwEnabled->clear();
	ow=w;
	std::vector<std::string>& v=qmpMainWindow::getInstance()->getWidgets(w);
	std::map<std::string,qmpFuncPrivate>& m=qmpMainWindow::getInstance()->getFunc();
	std::set<std::string> s;
	for(auto i=v.begin();i!=v.end();++i)
	{
		s.insert(*i);
		QListWidgetItem* it=new QListWidgetItem(
			m[*i].icon(),
			QString(m[*i].desc().c_str())
		);
		it->setToolTip(i->c_str());
		ui->lwEnabled->addItem(it);
	}
	for(auto i=m.begin();i!=m.end();++i)
	{
		if(s.find(i->first)==s.end())
		{
			QListWidgetItem* it=new QListWidgetItem(
				i->second.icon(),
				QString(i->second.desc().c_str())
			);
			it->setToolTip(i->first.c_str());
			ui->lwAvail->addItem(it);
		}
	}
}

void qmpCustomizeWindow::on_tbAdd_clicked()
{
	if(!ui->lwAvail->currentItem())return;
	ui->lwEnabled->addItem(ui->lwAvail->takeItem(ui->lwAvail->currentRow()));
	ui->lwAvail->removeItemWidget(ui->lwAvail->currentItem());
}

void qmpCustomizeWindow::on_tbRemove_clicked()
{
	if(!ui->lwEnabled->currentItem())return;
	ui->lwAvail->addItem(ui->lwEnabled->takeItem(ui->lwEnabled->currentRow()));
}

void qmpCustomizeWindow::on_buttonBox_accepted()
{
	std::vector<std::string>& v=qmpMainWindow::getInstance()->getWidgets(ow);
	v.clear();
	QList<QVariant> ql;
	for(int i=0;i<ui->lwEnabled->count();++i)
	{
		v.push_back(ui->lwEnabled->item(i)->toolTip().toStdString());
		ql.push_back(QVariant(ui->lwEnabled->item(i)->toolTip()));
	}
	QSettings *s=qmpMainWindow::getInstance()->getSettingsWindow()->getSettingsIntf();
	s->setValue(ow?"Behavior/Actions":"Behavior/Toolbar",ql);
	qmpMainWindow::getInstance()->setupWidget();
	close();
}

void qmpCustomizeWindow::on_buttonBox_rejected()
{
	close();
}
