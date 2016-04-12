#include <cstdio>
#include <QUrl>
#include <QMimeData>
#include <QDirIterator>
#include <QDesktopWidget>
#include "qmpmainwindow.hpp"
#include "ui_qmpmainwindow.h"
#include "../core/qmpmidiplay.hpp"
#ifdef _WIN32
#include <Windows.h>
char* wcsto8bit(const wchar_t* s)
{
	int size=WideCharToMultiByte(CP_OEMCP,WC_NO_BEST_FIT_CHARS,s,-1,0,0,0,0);
	char* c=(char*)calloc(size,sizeof(char));
	WideCharToMultiByte(CP_OEMCP,WC_NO_BEST_FIT_CHARS,s,-1,c,size,0,0);
	return c;
}
#define LOAD_SOUNDFONT \
	{\
		char* c=wcsto8bit(settingsw->getSFWidget()->item(i)->text().toStdWString().c_str());\
		player->pushSoundFont(c);\
		free(c);\
	}
#define LOAD_FILE \
	{\
		char* c=wcsto8bit(fns.toStdWString().c_str());\
		if(!player->playerLoadFile(c)){free(c);return;}\
		free(c);\
	}
#else
#define LOAD_SOUNDFONT \
	player->pushSoundFont(settingsw->getSFWidget()->item(i)->text().toStdString().c_str())
#define LOAD_FILE \
	if(!player->playerLoadFile(fns.toStdString().c_str()))return
#endif
#define UPDATE_INTERVAL 66

qmpMainWindow* qmpMainWindow::ref=NULL;

qmpMainWindow::qmpMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::qmpMainWindow)
{
	ui->setupUi(this);
	ui->lbFileName->setText("");ref=this;
	playing=false;stopped=true;dragging=false;
	settingsw=new qmpSettingsWindow(this);
	singleFS=qmpSettingsWindow::getSettingsIntf()->value("Behavior/SingleInstance",0).toInt();
	player=new CMidiPlayer(singleFS);
	plistw=new qmpPlistWindow(this);
	chnlw=new qmpChannelsWindow(this);
	efxw=new qmpEfxWindow(this);
	infow=new qmpInfoWindow(this);
	helpw=new qmpHelpWindow(this);
	timer=new QTimer(this);
	fnA1=new QAction("File Information",ui->lbFileName);
	fnA2=new QAction("Render to Wave",ui->lbFileName);
	ui->lbFileName->addAction(fnA1);
	ui->lbFileName->addAction(fnA2);
	if(singleFS){player->fluidPreInitialize();playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
			LOAD_SOUNDFONT;}
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus",0).toInt())
	{
		QRect g=geometry();
		g.setTopLeft(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/MainW",QPoint(-999,-999)).toPoint());
		if(g.topLeft()!=QPoint(-999,-999))setGeometry(g);
		else setGeometry(QStyle::alignedRect(
							 Qt::LeftToRight,Qt::AlignCenter,size(),
							 qApp->desktop()->availableGeometry()));
	}show();
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/PListWShown",0).toInt())
	{ui->pbPList->setChecked(true);on_pbPList_clicked();}
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlWShown",0).toInt())
	{ui->pbChannels->setChecked(true);on_pbChannels_clicked();}
	if(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxWShown",0).toInt())
	{ui->pbEfx->setChecked(true);on_pbEfx_clicked();}
	ui->vsMasterVol->setValue(qmpSettingsWindow::getSettingsIntf()->value("Audio/Gain",50).toInt());
	connect(fnA1,SIGNAL(triggered()),this,SLOT(onfnA1()));
	connect(fnA2,SIGNAL(triggered()),this,SLOT(onfnA2()));
	connect(timer,SIGNAL(timeout()),this,SLOT(updateWidgets()));
	connect(timer,SIGNAL(timeout()),chnlw,SLOT(channelWindowsUpdate()));
	connect(timer,SIGNAL(timeout()),infow,SLOT(updateInfo()));
}

qmpMainWindow::~qmpMainWindow()
{
	delete player;
	delete timer;
	delete ui;
}

int qmpMainWindow::pharseArgs(int argc,char** argv)
{
	bool havemidi=false,loadfolder=false;
	for(int i=1;i<argc;++i)
	{
		if(argv[i][0]=='-')
		{
			if(!strcmp(argv[i],"--help"))
			{
				printf("Usage: %s [Options] [Midi Files]\n",argv[0]);
				printf("Possible options are: \n");
				printf("-l, --load-all-files  Load all files from the same folder.\n");
				printf("--help                Show this help and exit.\n");
				printf("--version             Show this version information and exit.\n");
				return 1;
			}
			if(!strcmp(argv[i],"--version"))
			{
				printf("QMidiPlayer %s\n",APP_VERSION);
				return 1;
			}
			if(!strcmp(argv[i],"-l")||!strcmp(argv[i],"--load-all-files"))
				loadfolder=true;
		}
		else
		if(fluid_is_midifile(argv[i]))
		{
			if(!havemidi){havemidi=true;plistw->emptyList();}
			if(loadfolder||qmpSettingsWindow::getSettingsIntf()->value("Behavior/LoadFolder",0).toInt())
			{
				QDirIterator di(QUrl(argv[i]).adjusted(QUrl::RemoveFilename).toString());
				while(di.hasNext())
				{
					QString c=di.next();
					if((c.endsWith(".mid")||c.endsWith(".midi"))&&fluid_is_midifile(c.toStdString().c_str()))
					plistw->insertItem(c.toStdString().c_str());
				}
			}
			else
			plistw->insertItem(argv[i]);
		}
	}
	if(havemidi)on_pbPlayPause_clicked();
	return 0;
}

void qmpMainWindow::closeEvent(QCloseEvent *event)
{
	on_pbStop_clicked();fin=true;
	efxw->close();chnlw->close();
	plistw->close();infow->close();
	settingsw->close();
	delete helpw;helpw=NULL;
	delete efxw;efxw=NULL;
	delete chnlw;chnlw=NULL;
	delete plistw;plistw=NULL;
	delete infow;infow=NULL;
	delete settingsw;settingsw=NULL;
	event->accept();
}

void qmpMainWindow::moveEvent(QMoveEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/MainW",event->pos());
	}
}
void qmpMainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> l=event->mimeData()->urls();
	QStringList sl;
	for(int i=0;i<l.size();++i)
		sl.push_back(l.at(i).toLocalFile());
	plistw->insertItems(sl);
}
void qmpMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	//if(event->mimeData()->hasFormat("application/x-midi"))
	event->acceptProposedAction();
}

void qmpMainWindow::updateWidgets()
{
	fnA2->setEnabled(stopped);
	if(player->isFinished()&&playerTh)
	{
		if(!plistw->getRepeat())
		{
			timer->stop();stopped=true;playing=false;
			fnA2->setEnabled(stopped);
			player->playerDeinit();playerTh->join();
			delete playerTh;playerTh=NULL;
			if(singleFS)player->playerPanic(true);
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
			if(singleFS)player->playerPanic(true);
			chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
			QString fns=plistw->getNextItem();
			ui->lbFileName->setText(QUrl(fns).fileName());
			LOAD_FILE;
			char ts[100];
			sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
			ui->lbFinTime->setText(ts);
			player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
			for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
				LOAD_SOUNDFONT;}
			player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
			player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
			playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
			st=std::chrono::steady_clock::now();offset=0;
			timer->start(UPDATE_INTERVAL);
		}
	}
	if(renderTh)
	{
		if(player->isFinished())
		{
			renderTh->join();timer->stop();
			ui->centralWidget->setEnabled(true);
			delete renderTh;renderTh=NULL;
			player->rendererDeinit();
			if(singleFS)
			{
				player->fluidPreInitialize();
				playerSetup();
				player->fluidInitialize();
				for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
				LOAD_SOUNDFONT;
			}
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

void qmpMainWindow::playerSetup()
{
	fluid_settings_t* fsettings=player->getFluidSettings();
	QSettings* settings=qmpSettingsWindow::getSettingsIntf();
	fluid_settings_setstr(fsettings,"audio.driver",settings->value("Audio/Driver","").toString().toStdString().c_str());
	fluid_settings_setint(fsettings,"audio.period-size",settings->value("Audio/BufSize","").toInt());
	fluid_settings_setint(fsettings,"audio.periods",settings->value("Audio/BufCnt","").toInt());
	fluid_settings_setstr(fsettings,"audio.sample-format",settings->value("Audio/Format","").toString().toStdString().c_str());
	fluid_settings_setint(fsettings,"synth.sample-rate",settings->value("Audio/Frequency","").toInt());
	fluid_settings_setint(fsettings,"synth.polyphony",settings->value("Audio/Polyphony","").toInt());
	fluid_settings_setint(fsettings,"synth.cpu-cores",settings->value("Audio/Threads","").toInt());
	char bsmode[4];
	if(!singleFS&&settings->value("Audio/AutoBS",1).toInt()&&player->getFileStandard())
		switch(player->getFileStandard())
		{
			case 1:strcpy(bsmode,"gm");break;
			case 2:strcpy(bsmode,"mma");break;
			case 3:strcpy(bsmode,"gs");break;
			case 4:strcpy(bsmode,"xg");break;
		}
	else
	{
		if(settings->value("Audio/BankSelect","CC#0").toString()==QString("Ignored"))
			strcpy(bsmode,"gm");
		if(settings->value("Audio/BankSelect","CC#0").toString()==QString("CC#0"))
			strcpy(bsmode,"gs");
		if(settings->value("Audio/BankSelect","CC#0").toString()==QString("CC#32"))
			strcpy(bsmode,"xg");
		if(settings->value("Audio/BankSelect","CC#0").toString()==QString("CC#0*128+CC#32"))
			strcpy(bsmode,"mma");
	}
	fluid_settings_setstr(fsettings,"synth.midi-bank-select",bsmode);
	player->sendSysX(settings->value("Midi/SendSysEx",1).toInt());
}

void qmpMainWindow::on_pbPlayPause_clicked()
{
	playing=!playing;
	if(stopped)
	{
		QString fns=plistw->getFirstItem();
		if(!fns.length())
		{
			plistw->on_pbAdd_clicked();
			fns=plistw->getFirstItem();
			if(!fns.length())return(void)(playing=false);
		}
		ui->lbFileName->setText(QUrl(fns).fileName());
		LOAD_FILE;
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
		ui->lbFinTime->setText(ts);
		player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
			LOAD_SOUNDFONT;}
		player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
		player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
		playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
		st=std::chrono::steady_clock::now();offset=0;
		timer->start(UPDATE_INTERVAL);
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
	qmpSettingsWindow::getSettingsIntf()->setValue("Audio/Gain",ui->vsMasterVol->value());
}

void qmpMainWindow::on_pbStop_clicked()
{
	if(!stopped)
	{
		timer->stop();stopped=true;playing=false;
		player->playerDeinit();fnA2->setEnabled(stopped);
		if(singleFS)player->playerPanic(true);
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
	if(ui->pbPList->isChecked())
	{
		QRect g=plistw->geometry();
		g.setTopLeft(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/PListW",QPoint(-999,-999)).toPoint());
		if(g.topLeft()==QPoint(-999,-999))
			g.setTopLeft(window()->mapToGlobal(window()->rect().center())-plistw->rect().center());
		plistw->setGeometry(g);
		plistw->show();
	}else plistw->close();
}

void qmpMainWindow::on_pbChannels_clicked()
{
	if(ui->pbChannels->isChecked())
	{
		QRect g=chnlw->geometry();
		g.setTopLeft(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/ChnlW",QPoint(-999,-999)).toPoint());
		if(g.topLeft()==QPoint(-999,-999))
			g.setTopLeft(window()->mapToGlobal(window()->rect().center())-chnlw->rect().center());
		chnlw->setGeometry(g);
		chnlw->show();
	}else chnlw->close();
}

void qmpMainWindow::on_pbPrev_clicked()
{
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	if(singleFS)player->playerPanic(true);
	ui->hsTimer->setValue(0);chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getPrevItem();if(fns.length()==0)return on_pbStop_clicked();
	ui->lbFileName->setText(QUrl(fns).fileName());
	LOAD_FILE;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
	for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
		LOAD_SOUNDFONT;}
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
	player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(UPDATE_INTERVAL);
}

void qmpMainWindow::on_pbNext_clicked()
{
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	if(singleFS)player->playerPanic(true);
	ui->hsTimer->setValue(0);chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getNextItem();if(fns.length()==0)return on_pbStop_clicked();
	ui->lbFileName->setText(QUrl(fns).fileName());
	LOAD_FILE;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
	for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
		LOAD_SOUNDFONT;}
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
	player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(UPDATE_INTERVAL);
}

void qmpMainWindow::selectionChanged()
{
	stopped=false;playing=true;
	ui->pbPlayPause->setIcon(QIcon(":/img/pause.png"));
	timer->stop();player->playerDeinit();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	if(singleFS)player->playerPanic(true);
	ui->hsTimer->setValue(0);
	chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getSelectedItem();
	ui->lbFileName->setText(QUrl(fns).fileName());
	LOAD_FILE;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
	for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
		LOAD_SOUNDFONT;}
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
	player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(UPDATE_INTERVAL);
}

void qmpMainWindow::on_pbEfx_clicked()
{
	if(ui->pbEfx->isChecked())
	{
		QRect g=efxw->geometry();
		g.setTopLeft(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/EfxW",QPoint(-999,-999)).toPoint());
		if(g.topLeft()==QPoint(-999,-999))
			g.setTopLeft(window()->mapToGlobal(window()->rect().center())-efxw->rect().center());
		efxw->setGeometry(g);
		efxw->show();
	}
	else efxw->close();
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

void qmpMainWindow::onfnA2()
{
	if(singleFS)player->fluidDeinitialize();
#ifdef _WIN32
	char* ofstr=wcsto8bit((plistw->getSelectedItem()+QString(".wav")).toStdWString().c_str());
	char* ifstr=wcsto8bit(plistw->getSelectedItem().toStdWString().c_str());
	player->rendererLoadFile(ofstr);
	playerSetup();player->rendererInit(ifstr);
	free(ofstr);free(ifstr);
#else
	player->rendererLoadFile((plistw->getSelectedItem()+QString(".wav")).toStdString().c_str());
	playerSetup();player->rendererInit(plistw->getSelectedItem().toStdString().c_str());
#endif
	ui->centralWidget->setEnabled(false);
	for(int i=settingsw->getSFWidget()->count()-1;i>=0;--i)
		LOAD_SOUNDFONT;
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();timer->start(UPDATE_INTERVAL);
	renderTh=new std::thread(&CMidiPlayer::rendererThread,player);
}

void qmpMainWindow::on_pbSettings_clicked()
{
	if(ui->pbSettings->isChecked())settingsw->show();else settingsw->close();
}

void qmpMainWindow::on_pushButton_clicked()
{
	helpw->show();
}
