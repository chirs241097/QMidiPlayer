#include <cstdio>
#include "qmpchanneleditor.hpp"
#include "ui_qmpchanneleditor.h"
#include "qmpmainwindow.hpp"

qmpChannelEditor::qmpChannelEditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpChannelEditor)
{
	ui->setupUi(this);ch=0;
	styl=new QDialSkulptureStyle();
	QList<QDial*> dials=findChildren<QDial*>();
	for(int i=0;i<dials.count();++i)
		dials.at(i)->setStyle(styl);
}

qmpChannelEditor::~qmpChannelEditor()
{
	delete styl;
	delete ui;
}

void qmpChannelEditor::setupWindow(int chid)
{
	char str[256];if(~chid)ch=chid;
	setWindowTitle(tr("Channel Parameter Editor - Channel #%1").arg(ch+1));
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	uint16_t b;uint8_t p;std::string pstn;
	if(!player->getChannelOutputDevice(ch)->getChannelPreset(ch,&b,&p,pstn))
	{
		b=player->getCC(ch,0)<<7|player->getCC(ch,32);
		p=player->getCC(ch,128);
		pstn=player->getChannelOutputDevice(ch)->getPresetName(b,p);
	}
	ui->lbPresetName->setText(pstn.c_str());
	sprintf(str,"BK: %03d",b);ui->lbBank->setText(str);
	sprintf(str,"PC: %03d",p);ui->lbPreset->setText(str);
	ui->lbChannelNumber->setText(QString::number(ch+1));
#define setupControl(ccid,lb,d,ccname)\
	b=player->getCC(ch,ccid);\
	sprintf(str,"%s %d",ccname,b);\
	ui->lb->setText(str);\
	ui->d->setValue(b);
	setupControl(7,lbVol,dVol,"Vol.");
	setupControl(91,lbReverb,dReverb,"Rev.");
	setupControl(93,lbChorus,dChorus,"Chr.");
	setupControl(71,lbReso,dReso,"Res.");
	setupControl(74,lbCut,dCut,"Cut.");
	setupControl(73,lbAttack,dAttack,"Atk.");
	setupControl(75,lbDecay,dDecay,"Dec.");
	setupControl(72,lbRelease,dRelease,"Rel.");
	setupControl(76,lbRate,dRate,"Rate");
	setupControl(77,lbDepth,dDepth,"Dep.");
	setupControl(78,lbDelay,dDelay,"Del.");
	b=player->getCC(ch,10);
	if(b==64)strcpy(str,"Pan. C");
	else if(b<64)sprintf(str,"Pan. L%d",64-b);else sprintf(str,"Pan. R%d",b-64);
	ui->lbPan->setText(str);
	ui->dPan->setValue(b);
}

void qmpChannelEditor::sendCC()
{
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	player->setCC(ch,7,ui->dVol->value());
	player->setCC(ch,10,ui->dPan->value());
	player->setCC(ch,91,ui->dReverb->value());
	player->setCC(ch,93,ui->dChorus->value());
	player->setCC(ch,71,ui->dReso->value());
	player->setCC(ch,74,ui->dCut->value());
	player->setCC(ch,73,ui->dAttack->value());
	player->setCC(ch,75,ui->dDecay->value());
	player->setCC(ch,72,ui->dRelease->value());
	player->setCC(ch,76,ui->dRate->value());
	player->setCC(ch,77,ui->dDepth->value());
	player->setCC(ch,78,ui->dDelay->value());
}

void qmpChannelEditor::showEvent(QShowEvent *e)
{
	knobpressed=0;
	setupWindow();
	connectSlots();
	updconn=connect(qmpMainWindow::getInstance()->getTimer(),&QTimer::timeout,std::bind(&qmpChannelEditor::setupWindow,this,-1));
	e->accept();
}
void qmpChannelEditor::closeEvent(QCloseEvent *e)
{
	disconnectSlots();
	disconnect(updconn);
	e->accept();
}

void qmpChannelEditor::on_pbChLeft_clicked()
{
	disconnectSlots();
	if(ch>0)--ch;else ch=15;setupWindow();
	connectSlots();
}

void qmpChannelEditor::on_pbChRight_clicked()
{
	disconnectSlots();
	if(ch<15)++ch;else ch=0;setupWindow();
	connectSlots();
}

void qmpChannelEditor::commonPressed()
{
	disconnect(updconn);
	knobpressed=1;
}
void qmpChannelEditor::commonReleased()
{
	updconn=connect(qmpMainWindow::getInstance()->getTimer(),&QTimer::timeout,std::bind(&qmpChannelEditor::setupWindow,this,-1));
	sendCC();knobpressed=0;
}
void qmpChannelEditor::commonChanged()
{if(knobpressed){sendCC();setupWindow();}}

void qmpChannelEditor::connectSlots()
{
	connect(ui->dCut,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dReso,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dReverb,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dChorus,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dVol,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dPan,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dAttack,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dDecay,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dRelease,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dRate,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dDepth,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	connect(ui->dDelay,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);

	connect(ui->dCut,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dReso,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dReverb,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dChorus,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dVol,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dPan,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dAttack,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dDecay,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dRelease,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dRate,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dDepth,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	connect(ui->dDelay,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);

	connect(ui->dCut,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dReso,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dReverb,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dChorus,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dVol,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dPan,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dAttack,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dDecay,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dRelease,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dRate,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dDepth,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	connect(ui->dDelay,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
}

void qmpChannelEditor::disconnectSlots()
{
	disconnect(ui->dCut,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dReso,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dReverb,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dChorus,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dVol,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dPan,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dAttack,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dDecay,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dRelease,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dRate,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dDepth,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
	disconnect(ui->dDelay,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);

	disconnect(ui->dCut,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dReso,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dReverb,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dChorus,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dVol,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dPan,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dAttack,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dDecay,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dRelease,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dRate,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dDepth,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
	disconnect(ui->dDelay,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);

	disconnect(ui->dCut,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dReso,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dReverb,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dChorus,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dVol,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dPan,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dAttack,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dDecay,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dRelease,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dRate,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dDepth,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	disconnect(ui->dDelay,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
}
