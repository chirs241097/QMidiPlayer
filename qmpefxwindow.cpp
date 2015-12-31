#include <cmath>
#include "qmpefxwindow.hpp"
#include "ui_qmpefxwindow.h"
#include "qmpmainwindow.hpp"

qmpEfxWindow::qmpEfxWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpEfxWindow)
{
	ui->setupUi(this);initialized=false;
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	//stub. read these from settings after the setting module is implemented
	ui->cbEnabledC->setChecked(true);
	ui->cbEnabledR->setChecked(true);
}

qmpEfxWindow::~qmpEfxWindow()
{
	delete ui;
}

void qmpEfxWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	emit dialogClosing();
	event->accept();
}

void qmpEfxWindow::showEvent(QShowEvent *event)
{
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	player->getReverbPara(&rr,&rd,&rw,&rl);
	player->getChorusPara(&cfb,&cl,&cr,&cd,&ct);
	ui->sbRoom->setValue((int)round(rr*100));ui->dRoom->setValue((int)round(rr*100));
	ui->sbDamp->setValue((int)round(rd*100));ui->dRoom->setValue((int)round(rd*100));
	ui->sbWidth->setValue((int)round(rw*100));ui->dWidth->setValue((int)round(rw*100));
	ui->sbLevelR->setValue((int)round(rl*100));ui->dLevelR->setValue((int)round(rl*100));

	ui->sbFeedBack->setValue(cfb);ui->dFeedBack->setValue(cfb);
	ui->sbRate->setValue(cr);ui->dRate->setValue((int)round(cr*100));
	ui->sbDepth->setValue(cd);ui->dDepth->setValue((int)round(cd*10));
	ui->sbLevelC->setValue((int)round(cl*100));ui->dLevelC->setValue((int)round(cl*100));
	if(ct==FLUID_CHORUS_MOD_SINE)ui->rbSine->setChecked(true),ui->rbTriangle->setChecked(false);
	if(ct==FLUID_CHORUS_MOD_TRIANGLE)ui->rbSine->setChecked(false),ui->rbTriangle->setChecked(true);
	initialized=true;event->accept();
}

void qmpEfxWindow::sendEfxChange()
{
	if(!qmpMainWindow::getInstance()||!initialized)return;
	rr=ui->sbRoom->value()/100.;rd=ui->sbDamp->value()/100.;
	rw=ui->sbWidth->value()/100.;rl=ui->sbLevelR->value()/100.;
	ct=ui->rbSine->isChecked()?FLUID_CHORUS_MOD_SINE:FLUID_CHORUS_MOD_TRIANGLE;
	cfb=ui->sbFeedBack->value();cl=ui->sbLevelC->value()/100.;
	cr=ui->sbRate->value();cd=ui->sbDepth->value();
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	player->setReverbPara(ui->cbEnabledR->isChecked()?1:0,rr,rd,rw,rl);
	player->setChorusPara(ui->cbEnabledC->isChecked()?1:0,cfb,cl,cr,cd,ct);
}

void qmpEfxWindow::dailValueChange()
{
	ui->sbRoom->setValue(ui->dRoom->value());
	ui->sbDamp->setValue(ui->dDamp->value());
	ui->sbWidth->setValue(ui->dWidth->value());
	ui->sbLevelR->setValue(ui->dLevelR->value());
	ui->sbFeedBack->setValue(ui->dFeedBack->value());
	ui->sbRate->setValue(ui->dRate->value()/100.);
	ui->sbDepth->setValue(ui->dDepth->value()/10.);
	ui->sbLevelC->setValue(ui->dLevelC->value());
	sendEfxChange();
}

void qmpEfxWindow::spinValueChange()
{
	ui->dRoom->setValue(ui->sbRoom->value());
	ui->dDamp->setValue(ui->sbDamp->value());
	ui->dWidth->setValue(ui->sbWidth->value());
	ui->dLevelR->setValue(ui->sbLevelR->value());
	ui->dFeedBack->setValue(ui->sbFeedBack->value());
	ui->dRate->setValue((int)(ui->sbRate->value()*100));
	ui->dDepth->setValue((int)(ui->sbDepth->value()*10));
	ui->dLevelC->setValue(ui->sbLevelC->value());
	sendEfxChange();
}

void qmpEfxWindow::on_dRoom_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dDamp_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dWidth_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dLevelR_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dFeedBack_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dRate_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dDepth_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_dLevelC_valueChanged()
{dailValueChange();}

void qmpEfxWindow::on_sbRoom_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbDamp_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbWidth_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbLevelR_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbFeedBack_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbRate_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbDepth_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_sbLevelC_valueChanged(QString s)
{s=QString();spinValueChange();}

void qmpEfxWindow::on_cbEnabledC_stateChanged()
{sendEfxChange();}

void qmpEfxWindow::on_cbEnabledR_stateChanged()
{sendEfxChange();}

void qmpEfxWindow::on_rbSine_toggled()
{sendEfxChange();}

void qmpEfxWindow::on_rbTriangle_toggled()
{sendEfxChange();}
