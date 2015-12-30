#include <cstdio>
#include "qmpchanneleditor.hpp"
#include "ui_qmpchanneleditor.h"
#include "qmpmainwindow.hpp"

qmpchanneleditor::qmpchanneleditor(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpchanneleditor)
{
	ui->setupUi(this);
}

qmpchanneleditor::~qmpchanneleditor()
{
	delete ui;
}

void qmpchanneleditor::setupWindow(int chid)
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

void qmpchanneleditor::sendCC()
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

void qmpchanneleditor::showEvent(QShowEvent *e)
{
	e->accept();
	connect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));
}
void qmpchanneleditor::closeEvent(QCloseEvent *e)
{
	e->accept();
	disconnect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));
}

void qmpchanneleditor::on_pbChLeft_clicked()
{
	if(ch>0)--ch;else ch=15;setupWindow();
}

void qmpchanneleditor::on_pbChRight_clicked()
{
	if(ch<15)++ch;else ch=0;setupWindow();
}

#define dc disconnect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()))
#define rc connect(qmpMainWindow::getInstance()->getTimer(),SIGNAL(timeout()),this,SLOT(setupWindow()));sendCC()

void qmpchanneleditor::on_dCut_sliderPressed()
{dc;}

void qmpchanneleditor::on_dReso_sliderPressed()
{dc;}

void qmpchanneleditor::on_dReverb_sliderPressed()
{dc;}

void qmpchanneleditor::on_dChorus_sliderPressed()
{dc;}

void qmpchanneleditor::on_dVol_sliderPressed()
{dc;}

void qmpchanneleditor::on_dPan_sliderPressed()
{dc;}

void qmpchanneleditor::on_dAttack_sliderPressed()
{dc;}

void qmpchanneleditor::on_dDecay_sliderPressed()
{dc;}

void qmpchanneleditor::on_dRelease_sliderPressed()
{dc;}

void qmpchanneleditor::on_dRate_sliderPressed()
{dc;}

void qmpchanneleditor::on_dDepth_sliderPressed()
{dc;}

void qmpchanneleditor::on_dDelay_sliderPressed()
{dc;}

void qmpchanneleditor::on_dAttack_sliderReleased()
{rc;}

void qmpchanneleditor::on_dDecay_sliderReleased()
{rc;}

void qmpchanneleditor::on_dRelease_sliderReleased()
{rc;}

void qmpchanneleditor::on_dRate_sliderReleased()
{rc;}

void qmpchanneleditor::on_dDepth_sliderReleased()
{rc;}

void qmpchanneleditor::on_dDelay_sliderReleased()
{rc;}

void qmpchanneleditor::on_dCut_sliderReleased()
{rc;}

void qmpchanneleditor::on_dReso_sliderReleased()
{rc;}

void qmpchanneleditor::on_dReverb_sliderReleased()
{rc;}

void qmpchanneleditor::on_dChorus_sliderReleased()
{rc;}

void qmpchanneleditor::on_dVol_sliderReleased()
{rc;}

void qmpchanneleditor::on_dPan_sliderReleased()
{rc;}

void qmpchanneleditor::on_dCut_valueChanged()
{rc;}

void qmpchanneleditor::on_dReso_valueChanged()
{rc;}

void qmpchanneleditor::on_dReverb_valueChanged()
{rc;}

void qmpchanneleditor::on_dChorus_valueChanged()
{rc;}

void qmpchanneleditor::on_dVol_valueChanged()
{rc;}

void qmpchanneleditor::on_dPan_valueChanged()
{rc;}

void qmpchanneleditor::on_dAttack_valueChanged()
{rc;}

void qmpchanneleditor::on_dDecay_valueChanged()
{rc;}

void qmpchanneleditor::on_dRelease_valueChanged()
{rc;}

void qmpchanneleditor::on_dRate_valueChanged()
{rc;}

void qmpchanneleditor::on_dDepth_valueChanged()
{rc;}

void qmpchanneleditor::on_dDelay_valueChanged()
{rc;}
