#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include "qmpchannelswindow.hpp"
#include "ui_qmpchannelswindow.h"
#include "qmpmainwindow.hpp"

qmpchannelswindow::qmpchannelswindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpchannelswindow)
{
	ui->setupUi(this);
	pselectw=new qmppresetselect(this);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	for(int i=0;i<16;++i)
	{
		ui->twChannels->setCellWidget(i,0,new QCheckBox(""));
		connect(ui->twChannels->cellWidget(i,0),SIGNAL(stateChanged(int)),this,SLOT(channelMSChanged()));
		ui->twChannels->setCellWidget(i,1,new QCheckBox(""));
		connect(ui->twChannels->cellWidget(i,1),SIGNAL(stateChanged(int)),this,SLOT(channelMSChanged()));
		ui->twChannels->setCellWidget(i,2,new QComboBox());
		QComboBox *cb=(QComboBox*)ui->twChannels->cellWidget(i,2);
		//stub
		cb->addItem("Internal fluidsynth");
		ui->twChannels->setCellWidget(i,3,new QDCLabel(""));
		((QDCLabel*)ui->twChannels->cellWidget(i,3))->setID(i);
		connect(ui->twChannels->cellWidget(i,3),SIGNAL(onDoubleClick(int)),this,SLOT(showPresetWindow(int)));
		ui->twChannels->setCellWidget(i,4,new QPushButton("..."));
	}
	ui->twChannels->setColumnWidth(0,32);
	ui->twChannels->setColumnWidth(1,32);
	ui->twChannels->setColumnWidth(2,192);
	ui->twChannels->setColumnWidth(3,192);
	ui->twChannels->setColumnWidth(4,32);
}

void qmpchannelswindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	emit dialogClosing();
	event->accept();
}

void qmpchannelswindow::channelWindowsUpdate()
{
	for(int i=0;i<16;++i)
	{
		char data[128],nm[24];
		int b,p;
		((qmpMainWindow*)this->parent())->getPlayer()->getChannelPreset(i,&b,&p,nm);
		sprintf(data,"%d:%d %s",b,p,nm);
		((QLabel*)ui->twChannels->cellWidget(i,3))->setText(data);
	}
}

void qmpchannelswindow::channelMSChanged()
{
	for(int i=0;i<16;++i)
	{
		QCheckBox *m,*s;
		m=(QCheckBox*)ui->twChannels->cellWidget(i,0);
		s=(QCheckBox*)ui->twChannels->cellWidget(i,1);
		if(m->isChecked()&&s->isChecked())s->setChecked(false);
		((qmpMainWindow*)this->parent())->getPlayer()->setMute(i,m->isChecked());
		((qmpMainWindow*)this->parent())->getPlayer()->setSolo(i,s->isChecked());
	}
}

qmpchannelswindow::~qmpchannelswindow()
{
	delete ui;
}

void qmpchannelswindow::on_pbUnmute_clicked()
{
	for(int i=0;i<16;++i)
	{
		((QCheckBox*)ui->twChannels->cellWidget(i,0))->setChecked(false);
		((qmpMainWindow*)this->parent())->getPlayer()->setMute(i,false);
	}
}

void qmpchannelswindow::on_pbUnsolo_clicked()
{
	for(int i=0;i<16;++i)
	{
		((QCheckBox*)ui->twChannels->cellWidget(i,1))->setChecked(false);
		((qmpMainWindow*)this->parent())->getPlayer()->setSolo(i,false);
	}
}

void qmpchannelswindow::showPresetWindow(int chid)
{
	pselectw->show();
	pselectw->setupWindow(chid);
}
