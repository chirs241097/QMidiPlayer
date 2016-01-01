#include <cstdio>
#include "qmpchanneleditor.hpp"
#include "ui_qmpchanneleditor.h"
#include "qmpmainwindow.hpp"

qmpChannelEditor::qmpChannelEditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpChannelEditor)
{
	ui->setupUi(this);
}

qmpChannelEditor::~qmpChannelEditor()
{
	delete ui;
}

void qmpChannelEditor::setupWindow(int chid)
{
	char str[30];if(~chid)ch=chid;
	sprintf(str,"Channel Parameter Editor - Channel #%d",ch+1);
	setWindowTitle(str);
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	int b,p;
	player->getChannelPreset(ch,&b,&p,str);
	ui->lbPresetName->setText(str);
	sprintf(str,"BK: %d",b);ui->lbBank->setText(str);
	sprintf(str,"PC: %d",p);ui->lbPreset->setText(str);
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
	connect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));
	e->accept();
}
void qmpChannelEditor::closeEvent(QCloseEvent *e)
{
	disconnectSlots();
	disconnect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));
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
{disconnect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));knobpressed=1;}
void qmpChannelEditor::commonReleased()
{connect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));sendCC();knobpressed=0;}
void qmpChannelEditor::commonChanged()
{if(knobpressed){sendCC();setupWindow();}}

void qmpChannelEditor::connectSlots()
{
	connect(ui->dCut,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dReso,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dReverb,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dChorus,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dVol,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dPan,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dAttack,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dDecay,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dRelease,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dRate,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dDepth,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	connect(ui->dDelay,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));

	connect(ui->dCut,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dReso,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dReverb,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dChorus,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dVol,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dPan,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dAttack,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dDecay,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dRelease,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dRate,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dDepth,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	connect(ui->dDelay,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));

	connect(ui->dCut,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dReso,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dReverb,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dChorus,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dVol,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dPan,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dAttack,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dDecay,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dRelease,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dRate,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dDepth,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	connect(ui->dDelay,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
}

void qmpChannelEditor::disconnectSlots()
{
	disconnect(ui->dCut,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dReso,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dReverb,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dChorus,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dVol,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dPan,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dAttack,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dDecay,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dRelease,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dRate,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dDepth,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));
	disconnect(ui->dDelay,SIGNAL(sliderPressed()),this,SLOT(commonPressed()));

	disconnect(ui->dCut,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dReso,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dReverb,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dChorus,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dVol,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dPan,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dAttack,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dDecay,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dRelease,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dRate,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dDepth,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));
	disconnect(ui->dDelay,SIGNAL(sliderReleased()),this,SLOT(commonReleased()));

	disconnect(ui->dCut,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dReso,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dReverb,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dChorus,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dVol,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dPan,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dAttack,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dDecay,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dRelease,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dRate,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dDepth,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
	disconnect(ui->dDelay,SIGNAL(valueChanged(int)),this,SLOT(commonChanged()));
}
