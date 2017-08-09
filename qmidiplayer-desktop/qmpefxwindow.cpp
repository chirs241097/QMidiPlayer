#include <cmath>
#include "qmpefxwindow.hpp"
#include "ui_qmpefxwindow.h"
#include "qmpmainwindow.hpp"

qmpEfxWindow::qmpEfxWindow(QWidget *parent) :
	QWidget(parent,Qt::Window),
	ui(new Ui::qmpEfxWindow)
{
	ui->setupUi(this);initialized=false;
	styl=new QDialSkulptureStyle();
	QList<QDial*> dials=findChildren<QDial*>();
	for(int i=0;i<dials.count();++i)
		dials.at(i)->setStyle(styl);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
	QSettings *settings=qmpSettingsWindow::getSettingsIntf();
	ui->cbEnabledC->setChecked(settings->value("Effects/ChorusEnabled",1).toInt());
	ui->cbEnabledR->setChecked(settings->value("Effects/ReverbEnabled",1).toInt());
	rr=settings->value("Effects/ReverbRoom",FLUID_REVERB_DEFAULT_ROOMSIZE).toDouble();
	rd=settings->value("Effects/ReverbDamp",FLUID_REVERB_DEFAULT_DAMP).toDouble();
	rw=settings->value("Effects/ReverbWidth",FLUID_REVERB_DEFAULT_WIDTH).toDouble();
	rl=settings->value("Effects/ReverbLevel",FLUID_REVERB_DEFAULT_LEVEL).toDouble();

	cfb=settings->value("Effects/ChorusFeedbk",FLUID_CHORUS_DEFAULT_N).toInt();
	cl=settings->value("Effects/ChorusLevel",FLUID_CHORUS_DEFAULT_LEVEL).toDouble();
	cr=settings->value("Effects/ChorusRate",FLUID_CHORUS_DEFAULT_SPEED).toDouble();
	cd=settings->value("Effects/ChorusDepth",FLUID_CHORUS_DEFAULT_DEPTH).toDouble();
	ct=settings->value("Effects/ChorusType",FLUID_CHORUS_DEFAULT_TYPE).toInt();
	qmpMainWindow::getInstance()->registerFunctionality(
		efxf=new qmpEfxFunc(this),
		std::string("Effects"),
		tr("Effects").toStdString(),
		getThemedIconc(":/img/effects.svg"),
		0,
		true
	);
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxW",QRect(-999,-999,999,999)).toRect());
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxWShown",0).toInt())
	{show();qmpMainWindow::getInstance()->setFuncState("Effects",true);}
}

qmpEfxWindow::~qmpEfxWindow()
{
	qmpMainWindow::getInstance()->unregisterFunctionality("Effects");
	delete efxf;
	delete styl;
	delete ui;
}

void qmpEfxWindow::closeEvent(QCloseEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/EfxW",geometry());
	}
	setVisible(false);
	if(!qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/EfxWShown",0);
	}
	qmpMainWindow::getInstance()->setFuncState("Effects",false);
	event->accept();
}

void qmpEfxWindow::showEvent(QShowEvent *event)
{
	//CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	//These parameters will never be modified outside this window...
	/*if(initialized)
	{
		player->getReverbPara(&rr,&rd,&rw,&rl);
		player->getChorusPara(&cfb,&cl,&cr,&cd,&ct);
	}*/
	ui->sbRoom->setValue((int)round(rr*100));ui->dRoom->setValue((int)round(rr*100));
	ui->sbDamp->setValue((int)round(rd*100));ui->dDamp->setValue((int)round(rd*100));
	ui->sbWidth->setValue((int)round(rw*100));ui->dWidth->setValue((int)round(rw*100));
	ui->sbLevelR->setValue((int)round(rl*100));ui->dLevelR->setValue((int)round(rl*100));

	ui->sbFeedBack->setValue(cfb);ui->dFeedBack->setValue(cfb);
	ui->sbRate->setValue(cr);ui->dRate->setValue((int)round(cr*100));
	ui->sbDepth->setValue(cd);ui->dDepth->setValue((int)round(cd*10));
	ui->sbLevelC->setValue((int)round(cl*100));ui->dLevelC->setValue((int)round(cl*100));
	if(ct==FLUID_CHORUS_MOD_SINE)ui->rbSine->setChecked(true),ui->rbTriangle->setChecked(false);
	if(ct==FLUID_CHORUS_MOD_TRIANGLE)ui->rbSine->setChecked(false),ui->rbTriangle->setChecked(true);
	initialized=true;
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxW",QRect(-999,-999,999,999)).toRect()!=QRect(-999,-999,999,999))
		setGeometry(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxW",QRect(-999,-999,999,999)).toRect());
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/EfxWShown",1);
	}
	event->accept();
}

void qmpEfxWindow::sendEfxChange(void *_fs)
{
	if(!qmpMainWindow::getInstance()||!initialized)return;
	rr=ui->sbRoom->value()/100.;rd=ui->sbDamp->value()/100.;
	rw=ui->sbWidth->value()/100.;rl=ui->sbLevelR->value()/100.;
	ct=ui->rbSine->isChecked()?FLUID_CHORUS_MOD_SINE:FLUID_CHORUS_MOD_TRIANGLE;
	cfb=ui->sbFeedBack->value();cl=ui->sbLevelC->value()/100.;
	cr=ui->sbRate->value();cd=ui->sbDepth->value();
	IFluidSettings* fs=(IFluidSettings*)_fs;
	if(!_fs)fs=qmpMainWindow::getInstance()->getPlayer()->fluid();
	fs->setReverbPara(ui->cbEnabledR->isChecked()?1:0,rr,rd,rw,rl);
	fs->setChorusPara(ui->cbEnabledC->isChecked()?1:0,cfb,cl,cr,cd,ct);

	QSettings *settings=qmpSettingsWindow::getSettingsIntf();
	settings->setValue("Effects/ChorusEnabled",ui->cbEnabledC->isChecked()?1:0);
	settings->setValue("Effects/ReverbEnabled",ui->cbEnabledR->isChecked()?1:0);
	settings->setValue("Effects/ReverbRoom",rr);
	settings->setValue("Effects/ReverbDamp",rd);
	settings->setValue("Effects/ReverbWidth",rw);
	settings->setValue("Effects/ReverbLevel",rl);

	settings->setValue("Effects/ChorusFeedbk",cfb);
	settings->setValue("Effects/ChorusLevel",cl);
	settings->setValue("Effects/ChorusRate",cr);
	settings->setValue("Effects/ChorusDepth",cd);
	settings->setValue("Effects/ChorusType",ct);
}

void qmpEfxWindow::dailValueChange()
{
	if(!initialized)return;
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
	if(!initialized)return;
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

qmpEfxFunc::qmpEfxFunc(qmpEfxWindow *par)
{p=par;}
void qmpEfxFunc::show()
{p->show();}
void qmpEfxFunc::close()
{p->close();}
