#include <cstdio>
#include <QCheckBox>
#include <QPushButton>
#include <QComboBox>
#include "qmpchannelswindow.hpp"
#include "ui_qmpchannelswindow.h"
#include "qmpmainwindow.hpp"

qmpChannelsWindow::qmpChannelsWindow(QWidget *parent) :
	QWidget(parent,Qt::Dialog),
	ui(new Ui::qmpChannelsWindow)
{
	ui->setupUi(this);
	pselectw=new qmpPresetSelector(this);
	ceditw=new qmpChannelEditor(this);
	cha=new QIcon(":/img/ledon.svg");chi=new QIcon(":/img/ledoff.svg");
	cb=new qmpCWNoteOnCB();fused=callbacksc=cbcnt=0;
	qmpMainWindow::getInstance()->getPlayer()->setEventHandlerCB(cb,NULL);
	connect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
	std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
	size_t devc=devs.size();
	//We setup default output here...
	//Pretty strange...
	for(size_t i=0;i<devc;++i)
	{
		qmpSettingsWindow::getDefaultOutWidget()->addItem(devs[i].c_str());
		if(!QString(devs[i].c_str()).compare(qmpSettingsWindow::getSettingsIntf()->
				value("Midi/DefaultOutput","Internal FluidSynth").toString()))
			qmpSettingsWindow::getDefaultOutWidget()->setCurrentIndex(i);
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
		cb->setID(i);
		cb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
		for(size_t j=0;j<devc;++j)
		{
			cb->addItem(devs[j].c_str());
			if(!qmpSettingsWindow::getSettingsIntf()->
					value("Midi/DefaultOutput","Internal FluidSynth").toString().compare(
					QString(devs[j].c_str())))
			{
				cb->setCurrentIndex(j);
				changeMidiMapping(i,j);
			}
		}
		if(qmpSettingsWindow::getSettingsIntf()->value("Midi/DisableMapping",0).toInt())
			cb->setEnabled(false);
		connect(cb,SIGNAL(onChange(int,int)),this,SLOT(changeMidiMapping(int,int)));
		ui->twChannels->setItem(i,4,new QTableWidgetItem(""));
		ui->twChannels->item(i,4)->setFlags(Qt::ItemIsEnabled);
		ui->twChannels->setCellWidget(i,5,new QDCPushButton("..."));
		((QDCPushButton*)ui->twChannels->cellWidget(i,5))->setID(i);
		connect(ui->twChannels->cellWidget(i,5),SIGNAL(onClick(int)),this,SLOT(showChannelEditorWindow(int)));
	}
	connect(ui->twChannels,SIGNAL(cellDoubleClicked(int,int)),this,SLOT(showPresetWindow(int,int)));
	ui->twChannels->setColumnWidth(0,24);
	ui->twChannels->setColumnWidth(1,24);
	ui->twChannels->setColumnWidth(2,24);
	ui->twChannels->setColumnWidth(3,192);
	ui->twChannels->setColumnWidth(4,208);
	ui->twChannels->setColumnWidth(5,32);
	ui->twChannels->installEventFilter(this);
	qmpMainWindow::getInstance()->registerFunctionality(
		chnlf=new qmpChannelFunc(this),
		std::string("Channel"),
		tr("Channel").toStdString(),
		getThemedIconc(":/img/channel.svg"),
		0,
		true
	);
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect());
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlWShown",0).toInt())
	{show();qmpMainWindow::getInstance()->setFuncState("Channel",true);}
}

void qmpChannelsWindow::showEvent(QShowEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",1);
	}
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QRect(-999,-999,999,999)).toRect());
	event->accept();
}

void qmpChannelsWindow::closeEvent(QCloseEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlW",geometry());
	}
	setVisible(false);
	if(!qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/ChnlWShown",0);
	}
	qmpMainWindow::getInstance()->setFuncState("Channel",false);
	event->accept();
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
			ui->twChannels->item(i,4)->setText("");
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
		char data[128],nm[256];
		int b,p;
		qmpMainWindow::getInstance()->getPlayer()->getChannelPreset(i,&b,&p,nm);
		sprintf(data,"%03d:%03d %s",b,p,nm);
		if(fused)
		{
			if(strcmp((ui->twChannels->item(i,4))->
					  text().toStdString().c_str(),data))
			{
				connect(cb,SIGNAL(onNoteOn()),this,SLOT(updateChannelActivity()));
				fused=0;
			}
		}
		ui->twChannels->item(i,4)->setText(data);
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
	qmpMainWindow::getInstance()->unregisterFunctionality("Channel");
	delete chnlf;
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

void qmpChannelsWindow::showPresetWindow(int chid,int col)
{
	if(col!=4)return;
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

bool qmpChannelsWindow::eventFilter(QObject *o,QEvent *e)
{
	if(e->type()==QEvent::KeyPress&&ui->twChannels->currentColumn()==4)
	{
		QKeyEvent *ke=static_cast<QKeyEvent*>(e);
		if(ke->key()!=Qt::Key_Enter&&ke->key()!=Qt::Key_Return)return false;
		showPresetWindow(ui->twChannels->currentRow(),4);
		return true;
	}
	return false;
}

qmpChannelFunc::qmpChannelFunc(qmpChannelsWindow *par)
{p=par;}
void qmpChannelFunc::show()
{p->show();}
void qmpChannelFunc::close()
{p->close();}
