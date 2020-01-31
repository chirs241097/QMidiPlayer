#include <cstdio>
#include "qmpchanneleditor.hpp"
#include "ui_qmpchanneleditor.h"
#include "qmpmainwindow.hpp"

qmpChannelEditor::qmpChannelEditor(QWidget *parent):
	QDialog(parent),
	ui(new Ui::qmpChannelEditor)
{
	ui->setupUi(this);ch=0;
	styl=new QDialSkulptureStyle();
	dials=findChildren<QDial*>();
	for(auto&d:dials)
		d->setStyle(styl);
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
	auto setupControl=[this,player](int ccid,QLabel* lb,QDial* d,QString ccname,std::function<QString(uint16_t)> valconv)
	{
		uint16_t b=player->getCC(ch,ccid);
		lb->setText(QString("%1 %2").arg(ccname).arg(valconv(b)));
		d->setValue(b);
	};
	auto defconv=std::bind(static_cast<QString(*)(uint,int)>(&QString::number),std::placeholders::_1,10);
	auto panconv=[](uint v)->QString{
		if(v==64)return tr("C");
		else if(v<64)return tr("L%1").arg(64-v);
		else return tr("R%1").arg(v-64);
	};
	setupControl(7,ui->lbVol,ui->dVol,tr("Vol."),defconv);
	setupControl(91,ui->lbReverb,ui->dReverb,tr("Rev."),defconv);
	setupControl(93,ui->lbChorus,ui->dChorus,tr("Chr."),defconv);
	setupControl(71,ui->lbReso,ui->dReso,tr("Res."),defconv);
	setupControl(74,ui->lbCut,ui->dCut,tr("Cut."),defconv);
	setupControl(73,ui->lbAttack,ui->dAttack,tr("Atk."),defconv);
	setupControl(75,ui->lbDecay,ui->dDecay,tr("Dec."),defconv);
	setupControl(72,ui->lbRelease,ui->dRelease,tr("Rel."),defconv);
	setupControl(76,ui->lbRate,ui->dRate,tr("Rate"),defconv);
	setupControl(77,ui->lbDepth,ui->dDepth,tr("Dep."),defconv);
	setupControl(78,ui->lbDelay,ui->dDelay,tr("Del."),defconv);
	setupControl(10,ui->lbPan,ui->dPan,tr("Pan."),panconv);
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
	qmpMainWindow::getInstance()->invokeCallback("channel.ccchange",nullptr);
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
	for(auto&d:dials)
	{
		connect(d,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
		connect(d,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
		connect(d,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	}
}

void qmpChannelEditor::disconnectSlots()
{
	for(auto&d:dials)
	{
		disconnect(d,&QDial::sliderPressed,this,&qmpChannelEditor::commonPressed);
		disconnect(d,&QDial::sliderReleased,this,&qmpChannelEditor::commonReleased);
		disconnect(d,&QDial::valueChanged,this,&qmpChannelEditor::commonChanged);
	}
}
