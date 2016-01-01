#include <cstdio>
#include <QUrl>
#include "qmpmainwindow.hpp"
#include "ui_qmpmainwindow.h"
#include "qmpmidiplay.hpp"

qmpMainWindow* qmpMainWindow::ref=NULL;

qmpMainWindow::qmpMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::qmpMainWindow)
{
	ui->setupUi(this);player=new CMidiPlayer();
	ui->lbFileName->setText("");
	playing=false;stopped=true;dragging=false;
	plistw=new qmpPlistWindow(this);
	chnlw=new qmpChannelsWindow(this);
	efxw=new qmpEfxWindow(this);
	infow=new qmpInfoWindow(this);
	settingsw=new qmpSettingsWindow(this);
	timer=new QTimer(this);ref=this;
	fnA1=new QAction("File Information",ui->lbFileName);
	ui->lbFileName->addAction(fnA1);
	connect(fnA1,SIGNAL(triggered()),this,SLOT(onfnA1()));
	connect(timer,SIGNAL(timeout()),this,SLOT(updateWidgets()));
	connect(timer,SIGNAL(timeout()),chnlw,SLOT(channelWindowsUpdate()));
	connect(timer,SIGNAL(timeout()),infow,SLOT(updateInfo()));
}

qmpMainWindow::~qmpMainWindow()
{
	delete timer;
	delete ui;
}

void qmpMainWindow::closeEvent(QCloseEvent *event)
{
	on_pbStop_clicked();
	efxw->close();chnlw->close();
	plistw->close();infow->close();
	settingsw->close();
	delete efxw;efxw=NULL;
	delete chnlw;chnlw=NULL;
	delete plistw;plistw=NULL;
	delete infow;infow=NULL;
	delete settingsw;settingsw=NULL;
	event->accept();
}

void qmpMainWindow::updateWidgets()
{
	if(player->isFinished()&&playerTh)
	{
		if(!plistw->getRepeat())
		{
			timer->stop();stopped=true;playing=false;
			player->playerDeinit();playerTh->join();
			delete playerTh;playerTh=NULL;
			chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
			ui->pbPlayPause->setIcon(QIcon(":/img/play.png"));
			ui->hsTimer->setValue(0);
			ui->lbPolyphone->setText("Poly: 0/0");
			ui->lbCurTime->setText("00:00");
		}
		else
		{
			timer->stop();player->playerDeinit();playerTh->join();
			delete playerTh;playerTh=NULL;
			ui->hsTimer->setValue(0);
			chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
			QString fns=plistw->getNextItem();
			ui->lbFileName->setText(QUrl(fns).fileName());
			player->playerLoadFile(fns.toStdString().c_str());
			char ts[100];
			sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
			ui->lbFinTime->setText(ts);
			player->playerInit();player->setGain(ui->vsMasterVol->value()/250.);
			playerTh=new std::thread(&CMidiPlayer::playerThread,player);
			st=std::chrono::steady_clock::now();offset=0;
			timer->start(100);
		}
	}
	while(!player->isFinished()&&player->getTCeptr()>player->getStamp(ui->hsTimer->value())
		  &&ui->hsTimer->value()<=100&&!dragging)
		ui->hsTimer->setValue(ui->hsTimer->value()+1);
	if(playing)
	{
		std::chrono::duration<double> elapsed=
				std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now()-st);
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)(elapsed.count()+offset)/60,(int)(elapsed.count()+offset)%60);
		ui->lbCurTime->setText(ts);
		sprintf(ts,"Poly: %d/%d",player->getPolyphone(),player->getMaxPolyphone());
		ui->lbPolyphone->setText(ts);
	}
}

QString qmpMainWindow::getFileName(){return ui->lbFileName->text();}

void qmpMainWindow::on_pbPlayPause_clicked()
{
	playing=!playing;
	if(stopped)
	{
		QString fns=plistw->getFirstItem();
		if(!fns.length())return(void)(playing=false);
		ui->lbFileName->setText(QUrl(fns).fileName());
		player->playerLoadFile(fns.toStdString().c_str());
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
		ui->lbFinTime->setText(ts);
		player->playerInit();player->setGain(ui->vsMasterVol->value()/250.);
		playerTh=new std::thread(&CMidiPlayer::playerThread,player);
		st=std::chrono::steady_clock::now();offset=0;
		timer->start(100);
		stopped=false;
	}
	else
	{
		if(!playing)
		{
			player->playerPanic();
			offset=ui->hsTimer->value()/100.*player->getFtime();
		}
		else
		{
			st=std::chrono::steady_clock::now();
			player->setResumed();
		}
		player->setTCpaused(!playing);
	}
	ui->pbPlayPause->setIcon(QIcon(playing?":/img/pause.png":":/img/play.png"));
}

void qmpMainWindow::on_hsTimer_sliderPressed()
{
	dragging=true;
}

void qmpMainWindow::on_hsTimer_sliderReleased()
{
	dragging=false;
	if(playing)
	{
		if(ui->hsTimer->value()==100){on_pbNext_clicked();return;}
		player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
		player->playerPanic();
		offset=ui->hsTimer->value()/100.*player->getFtime();
		st=std::chrono::steady_clock::now();
	}
	else
	{
		player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
		offset=ui->hsTimer->value()/100.*player->getFtime();
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)(offset)/60,(int)(offset)%60);
		ui->lbCurTime->setText(ts);
	}
}

void qmpMainWindow::on_vsMasterVol_valueChanged()
{
	if(!stopped)player->setGain(ui->vsMasterVol->value()/250.);
}

void qmpMainWindow::on_pbStop_clicked()
{
	if(!stopped)
	{
		timer->stop();stopped=true;playing=false;
		player->playerDeinit();
		if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
		chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
		ui->pbPlayPause->setIcon(QIcon(":/img/play.png"));
		ui->hsTimer->setValue(0);
		ui->lbPolyphone->setText("Poly: 0/0");
		ui->lbCurTime->setText("00:00");
	}
}

void qmpMainWindow::dialogClosed()
{
	if(!plistw->isVisible())ui->pbPList->setChecked(false);
	if(!chnlw->isVisible())ui->pbChannels->setChecked(false);
	if(!efxw->isVisible())ui->pbEfx->setChecked(false);
	if(!settingsw->isVisible())ui->pbSettings->setChecked(false);
}

void qmpMainWindow::on_pbPList_clicked()
{
	if(ui->pbPList->isChecked())plistw->show();else plistw->close();
}

void qmpMainWindow::on_pbChannels_clicked()
{
	if(ui->pbChannels->isChecked())chnlw->show();else chnlw->close();
}

void qmpMainWindow::on_pbPrev_clicked()
{
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	ui->hsTimer->setValue(0);chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getPrevItem();if(fns.length()==0)return on_pbStop_clicked();
	ui->lbFileName->setText(QUrl(fns).fileName());
	player->playerLoadFile(fns.toStdString().c_str());
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();player->setGain(ui->vsMasterVol->value()/250.);
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(100);
}

void qmpMainWindow::on_pbNext_clicked()
{
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	ui->hsTimer->setValue(0);chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getNextItem();if(fns.length()==0)return on_pbStop_clicked();
	ui->lbFileName->setText(QUrl(fns).fileName());
	player->playerLoadFile(fns.toStdString().c_str());
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();player->setGain(ui->vsMasterVol->value()/250.);
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(100);
}

void qmpMainWindow::selectionChanged()
{
	stopped=false;playing=true;
	ui->pbPlayPause->setIcon(QIcon(":/img/pause.png"));
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	ui->hsTimer->setValue(0);
	chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getSelectedItem();
	ui->lbFileName->setText(QUrl(fns).fileName());
	player->playerLoadFile(fns.toStdString().c_str());
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();player->setGain(ui->vsMasterVol->value()/250.);
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(100);
}

void qmpMainWindow::on_pbEfx_clicked()
{
	if(ui->pbEfx->isChecked())efxw->show();else efxw->close();
}

void qmpMainWindow::on_lbFileName_customContextMenuRequested(const QPoint &pos)
{
	QMenu menu(ui->lbFileName);
	menu.addActions(ui->lbFileName->actions());
	menu.exec(this->pos()+ui->lbFileName->pos()+pos);
}

void qmpMainWindow::onfnA1()
{
	infow->show();
}

void qmpMainWindow::on_pbSettings_clicked()
{
	if(ui->pbSettings->isChecked())settingsw->show();else settingsw->close();
}
