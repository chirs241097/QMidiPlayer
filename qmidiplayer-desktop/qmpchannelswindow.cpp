#include <cstdio>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include "qmpchannelswindow.hpp"
#include "ui_qmpchannelswindow.h"
#include "qmpmainwindow.hpp"

qmpChannelsWindow::qmpChannelsWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpChannelsWindow)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
	pselectw=new qmpPresetSelector(this);
	ceditw=new qmpChannelEditor(this);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	mapper=qmpMainWindow::getInstance()->getPlayer()->getMidiMapper();
	cha=new QPixmap(":/img/ledon.png");chi=new QPixmap(":/img/ledoff.png");
	cb=new qmpCWNoteOnCB();fused=callbacksc=cbcnt=0;
	qmpMainWindow::getInstance()->getPlayer()->setEventHandlerCB(cb,NULL);
	connect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
	int devc=mapper->enumDevices();
	//We setup default output here...
	//Pretty strange...
	for(int i=0;i<devc;++i)
	{
		qmpSettingsWindow::getDefaultOutWidget()->addItem(mapper->deviceName(i).c_str());
		if(!QString(mapper->deviceName(i).c_str()).compare(qmpSettingsWindow::getSettingsIntf()->
				value("Midi/DefaultOutput","Internal FluidSynth").toString()))
			qmpSettingsWindow::getDefaultOutWidget()->setCurrentIndex(i+1);
	}
	qmpSettingsWindow::getSettingsIntf()->setValue("Midi/DefaultOutput",
			qmpSettingsWindow::getDefaultOutWidget()->currentText());
	qmpSettingsWindow::getSettingsIntf();
	for(int i=0;i<16;++i)
	{
		ui->twChannels->setItem(i,0,new QTableWidgetItem());
		ui->twChannels->item(i,0)->setIcon(*chi);
		ui->twChannels->item(i,0)->setFlags(ui->twChannels->item(i,0)->flags()^Qt::ItemIsEditable);
		ui->twChannels->setCellWidget(i,1,new QCheckBox(""));
		connect(ui->twChannels->cellWidget(i,1),SIGNAL(stateChanged(int)),this,SLOT(channelMSChanged()));
		ui->twChannels->setCellWidget(i,2,new QCheckBox(""));
		connect(ui->twChannels->cellWidget(i,2),SIGNAL(stateChanged(int)),this,SLOT(channelMSChanged()));
		ui->twChannels->setCellWidget(i,3,new QDCComboBox());
		QDCComboBox *cb=(QDCComboBox*)ui->twChannels->cellWidget(i,3);
		cb->addItem("Internal FluidSynth");cb->setID(i);
		for(int j=0;j<devc;++j)
		{
			cb->addItem(mapper->deviceName(j).c_str());
			if(!qmpSettingsWindow::getSettingsIntf()->
					value("Midi/DefaultOutput","Internal FluidSynth").toString().compare(
					QString(mapper->deviceName(j).c_str())))
			{
				cb->setCurrentIndex(j+1);
				changeMidiMapping(i,j+1);
			}
		}
		if(qmpSettingsWindow::getSettingsIntf()->value("Midi/DisableMapping",0).toInt())
			cb->setEnabled(false);
		connect(cb,SIGNAL(onChange(int,int)),this,SLOT(changeMidiMapping(int,int)));
		ui->twChannels->setCellWidget(i,4,new QDCLabel(""));
		((QDCLabel*)ui->twChannels->cellWidget(i,4))->setID(i);
		connect(ui->twChannels->cellWidget(i,4),SIGNAL(onDoubleClick(int)),this,SLOT(showPresetWindow(int)));
		ui->twChannels->setCellWidget(i,5,new QDCPushButton("..."));
		((QDCLabel*)ui->twChannels->cellWidget(i,5))->setID(i);
		connect(ui->twChannels->cellWidget(i,5),SIGNAL(onClick(int)),this,SLOT(showChannelEditorWindow(int)));
	}
	ui->twChannels->setColumnWidth(0,24*(logicalDpiX()/96.));
	ui->twChannels->setColumnWidth(1,24*(logicalDpiX()/96.));
	ui->twChannels->setColumnWidth(2,24*(logicalDpiX()/96.));
	ui->twChannels->setColumnWidth(3,192*(logicalDpiX()/96.));
	ui->twChannels->setColumnWidth(4,208*(logicalDpiX()/96.));
	ui->twChannels->setColumnWidth(5,32*(logicalDpiX()/96.));
}

void qmpChannelsWindow::showEvent(QShowEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",1);
	}
	event->accept();
}

void qmpChannelsWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	if(!qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",0);
	}
	emit dialogClosing();
	event->accept();
}

void qmpChannelsWindow::moveEvent(QMoveEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlW",event->pos());
	}
}

void qmpChannelsWindow::resetAcitivity()
{
	for(int i=0;i<16;++i)ui->twChannels->item(i,0)->setIcon(*chi);
}

void qmpChannelsWindow::updateChannelActivity()
{
	++callbacksc;
	for(int i=0;i<16;++i)
	ui->twChannels->item(i,0)->setIcon(
	qmpMainWindow::getInstance()->getPlayer()->getChstates()[i]?*cha:*chi);
}

void qmpChannelsWindow::channelWindowsUpdate()
{
	if(qmpMainWindow::getInstance()->getPlayer()->isFinished())
	{
		for(int i=0;i<16;++i)
			((QLabel*)ui->twChannels->cellWidget(i,4))->setText("");
		connect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
		fused=0;return;
	}
	++cbcnt;
	if(cbcnt>15)
	{
		if(callbacksc>8192)
		{
			disconnect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
			fprintf(stderr,"Fuse!\n");fused=1;
		}
		cbcnt=0;
		callbacksc=0;
	}
	for(int i=0;i<16;++i)
	{
		char data[128],nm[25];
		int b,p;
		qmpMainWindow::getInstance()->getPlayer()->getChannelPreset(i,&b,&p,nm);
		sprintf(data,"%03d:%03d %s",b,p,nm);
		if(fused)
		{
			if(strcmp(((QLabel*)ui->twChannels->cellWidget(i,4))->
					  text().toStdString().c_str(),data))
			{
				connect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
				fused=0;
			}
		}
		((QLabel*)ui->twChannels->cellWidget(i,4))->setText(data);
		ui->twChannels->item(i,0)->setIcon(
		qmpMainWindow::getInstance()->getPlayer()->getChstates()[i]?*cha:*chi);
		if(qmpMainWindow::getInstance()->getPlayer()->getChstates()[i])
			qmpMainWindow::getInstance()->getPlayer()->getChstates()[i]=0;
	}
}

void qmpChannelsWindow::channelMSChanged()
{
	for(int i=0;i<16;++i)
	{
		QCheckBox *m,*s;
		m=(QCheckBox*)ui->twChannels->cellWidget(i,1);
		s=(QCheckBox*)ui->twChannels->cellWidget(i,2);
		if(m->isChecked()&&s->isChecked())s->setChecked(false);
		qmpMainWindow::getInstance()->getPlayer()->setMute(i,m->isChecked());
		qmpMainWindow::getInstance()->getPlayer()->setSolo(i,s->isChecked());
	}
}

qmpChannelsWindow::~qmpChannelsWindow()
{
	delete chi;delete cha;
	delete cb;delete ui;
}

void qmpChannelsWindow::on_pbUnmute_clicked()
{
	for(int i=0;i<16;++i)
	{
		((QCheckBox*)ui->twChannels->cellWidget(i,1))->setChecked(false);
		qmpMainWindow::getInstance()->getPlayer()->setMute(i,false);
	}
}

void qmpChannelsWindow::on_pbUnsolo_clicked()
{
	for(int i=0;i<16;++i)
	{
		((QCheckBox*)ui->twChannels->cellWidget(i,2))->setChecked(false);
		qmpMainWindow::getInstance()->getPlayer()->setSolo(i,false);
	}
}

void qmpChannelsWindow::showPresetWindow(int chid)
{
	pselectw->show();
	pselectw->setupWindow(chid);
}

void qmpChannelsWindow::showChannelEditorWindow(int chid)
{
	ceditw->show();
	ceditw->setupWindow(chid);
}

void qmpChannelsWindow::changeMidiMapping(int chid,int idx)
{
	qmpMainWindow::getInstance()->getPlayer()->setChannelOutput(chid,idx);
}
