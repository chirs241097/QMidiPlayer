#include <cstdio>
#include <cmath>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QFont>
#include <QTextCodec>
#include <QDirIterator>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QCheckBox>
#include "qmpmainwindow.hpp"
#include "ui_qmpmainwindow.h"
#include "../core/qmpmidiplay.hpp"
#define setButtonHeight(x,h) {x->setMaximumHeight(h*(logicalDpiY()/96.));x->setMinimumHeight(h*(logicalDpiY()/96.));}
#define setButtonWidth(x,h) {x->setMaximumWidth(h*(logicalDpiY()/96.));x->setMinimumWidth(h*(logicalDpiY()/96.));}
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
		char* c=wcsto8bit(settingsw->getSFWidget()->item(i,1)->text().toStdWString().c_str());\
		player->pushSoundFont(c);\
		free(c);\
	}
#define LOAD_FILE \
	{\
		for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->reset();\
		char* c=wcsto8bit(fns.toStdWString().c_str());\
		if(!player->playerLoadFile(c)){free(c);QMessageBox::critical(this,tr("Error"),tr("%1 is not a valid midi file.").arg(fns));return;}\
		free(c);\
	}
#else
#define LOAD_SOUNDFONT \
	player->pushSoundFont(settingsw->getSFWidget()->item(i,1)->text().toStdString().c_str())
#define LOAD_FILE \
	{\
		for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->reset();\
		if(!player->playerLoadFile(fns.toStdString().c_str())){QMessageBox::critical(this,tr("Error"),tr("%1 is not a valid midi file.").arg(fns));return;}\
	}
#endif
#define UPDATE_INTERVAL 66

qmpMainWindow* qmpMainWindow::ref=NULL;

qmpMainWindow::qmpMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::qmpMainWindow)
{
	ui->setupUi(this);
	ui->lnPolyphone->display("00000-00000");
	ui->lbFileName->setText("");ref=this;ui->verticalLayout->setAlignment(ui->pushButton,Qt::AlignRight);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
	setButtonHeight(ui->pbNext,36);setButtonHeight(ui->pbPlayPause,36);
	setButtonHeight(ui->pbPrev,36);setButtonHeight(ui->pbSettings,36);setButtonHeight(ui->pbStop,36);
	//setButtonHeight(ui->pbChannels,36);setButtonHeight(ui->pbPList,36);
	//setButtonHeight(ui->pbEfx,36);setButtonHeight(ui->pbVisualization,36);
	playing=false;stopped=true;dragging=false;
	settingsw=new qmpSettingsWindow(this);pmgr=new qmpPluginManager();
	plistw=new qmpPlistWindow(this);player=NULL;timer=NULL;
	singleFS=qmpSettingsWindow::getSettingsIntf()->value("Behavior/SingleInstance",0).toInt();
}

qmpMainWindow::~qmpMainWindow()
{
	pmgr->deinitPlugins();
	delete pmgr;if(player)delete player;
	if(timer)delete timer;
	delete helpw;helpw=NULL;
	delete efxw;efxw=NULL;
	delete chnlw;chnlw=NULL;
	delete plistw;plistw=NULL;
	delete infow;infow=NULL;
	delete settingsw;settingsw=NULL;
	delete ui;
}

void qmpMainWindow::init()
{
	player=new CMidiPlayer(singleFS);
	chnlw=new qmpChannelsWindow(this);
	efxw=new qmpEfxWindow(this);
	infow=new qmpInfoWindow(this);
	helpw=new qmpHelpWindow(this);
	timer=new QTimer(this);
	renderf=new qmpRenderFunc(this);
	panicf=new qmpPanicFunc(this);
	reloadsynf=new qmpReloadSynthFunc(this);
	registerFunctionality(renderf,"Render",tr("Render to wave").toStdString(),NULL,0,false);
	registerFunctionality(panicf,"Panic",tr("Panic").toStdString(),NULL,0,false);
	registerFunctionality(reloadsynf,"ReloadSynth",tr("Restart fluidsynth").toStdString(),NULL,0,false);
	pmgr->scanPlugins();settingsw->updatePluginList(pmgr);pmgr->initPlugins();
	if(singleFS){player->fluidPreInitialize();playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
			LOAD_SOUNDFONT;
		}}
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus",0).toInt())
	{
		QRect g=geometry();
		g.setTopLeft(qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/MainW",QPoint(-999,-999)).toPoint());
		if(g.topLeft()!=QPoint(-999,-999))setGeometry(g);
		else setGeometry(QStyle::alignedRect(
							 Qt::LeftToRight,Qt::AlignCenter,size(),
							 qApp->desktop()->availableGeometry()));
	}show();
	ui->vsMasterVol->setValue(qmpSettingsWindow::getSettingsIntf()->value("Audio/Gain",50).toInt());
	connect(timer,SIGNAL(timeout()),this,SLOT(updateWidgets()));
	connect(timer,SIGNAL(timeout()),chnlw,SLOT(channelWindowsUpdate()));
	connect(timer,SIGNAL(timeout()),infow,SLOT(updateInfo()));
	ui->pbNext->setIcon(QIcon(getThemedIcon(":/img/next.png")));
	ui->pbPrev->setIcon(QIcon(getThemedIcon(":/img/prev.png")));
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.png")));
	ui->pbStop->setIcon(QIcon(getThemedIcon(":/img/stop.png")));
	//ui->pbChannels->setIcon(QIcon(getThemedIcon(":/img/channel.png")));
	//ui->pbEfx->setIcon(QIcon(getThemedIcon(":/img/effects.png")));
	//ui->pbPList->setIcon(QIcon(getThemedIcon(":/img/list.png")));
	//ui->pbVisualization->setIcon(QIcon(getThemedIcon(":/img/visualization.png")));
	ui->pbSettings->setIcon(QIcon(getThemedIcon(":/img/settings.png")));
	if(havemidi)on_pbPlayPause_clicked();
	setupWidget();
}

int qmpMainWindow::pharseArgs()
{
	bool loadfolder=false;havemidi=false;
	QStringList args=QApplication::arguments();
	for(int i=1;i<args.size();++i)
	{
		if(args.at(i).at(0)=='-')
		{
			if(args.at(i)==QString("--help"))
			{
				printf("Usage: %s [Options] [Midi Files]\n",args.at(0).toStdString().c_str());
				printf("Possible options are: \n");
				printf("  -l, --load-all-files  Load all files from the same folder.\n");
				printf("  --help                Show this help and exit.\n");
				printf("  --version             Show this version information and exit.\n");
				return 1;
			}
			if(args.at(i)==QString("--version"))
			{
				printf("QMidiPlayer %s\n",APP_VERSION);
				return 1;
			}
			if(args.at(i)==QString("-l")||args.at(i)==QString("--load-all-files"))
				loadfolder=true;
		}
		else
#ifdef _WIN32
		{
			char* c=wcsto8bit(args.at(i).toStdWString().c_str());
			if(fluid_is_midifile(c))
			{
				if(!havemidi){havemidi=true;plistw->emptyList();}
				if(loadfolder||qmpSettingsWindow::getSettingsIntf()->value("Behavior/LoadFolder",0).toInt())
				{
					QDirIterator di(QFileInfo(args.at(i)).absolutePath());
					while(di.hasNext())
					{
						QString c=di.next();char* cc=wcsto8bit(c.toStdWString().c_str());
						if((c.endsWith(".mid")||c.endsWith(".midi"))&&fluid_is_midifile(cc))
						plistw->insertItem(c);free(cc);
					}
				}
				else
				plistw->insertItem(args.at(i));
			}
			free(c);
		}
#else
		if(fluid_is_midifile(args.at(i).toStdString().c_str()))
		{
			if(!havemidi){havemidi=true;plistw->emptyList();}
			if(loadfolder||qmpSettingsWindow::getSettingsIntf()->value("Behavior/LoadFolder",0).toInt())
			{
				QDirIterator di(QFileInfo(args.at(i)).absolutePath());
				while(di.hasNext())
				{
					QString c=di.next();
					if((c.endsWith(".mid")||c.endsWith(".midi"))&&fluid_is_midifile(c.toStdString().c_str()))
					plistw->insertItem(c.toStdString().c_str());
				}
			}
			else
			plistw->insertItem(args.at(i).toStdString().c_str());
		}
#endif
	}
	return 0;
}

void qmpMainWindow::closeEvent(QCloseEvent *event)
{
	on_pbStop_clicked();fin=true;
	for(auto i=mfunc.begin();i!=mfunc.end();++i)
	i->second.setAssignedControl((QReflectiveAction*)NULL),
	i->second.setAssignedControl((QReflectivePushButton*)NULL);
	efxw->close();chnlw->close();
	plistw->close();infow->close();
	settingsw->close();
	event->accept();
}

void qmpMainWindow::moveEvent(QMoveEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/MainW",event->pos());
	}
	event->accept();
}
void qmpMainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> l=event->mimeData()->urls();
	QStringList sl;
	for(int i=0;i<l.size();++i)
		sl.push_back(l.at(i).toLocalFile());
	plistw->insertItems(sl);
	switchTrack(plistw->getLastItem());
}
void qmpMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	//if(event->mimeData()->hasFormat("application/x-midi"))
	event->acceptProposedAction();
}

void qmpMainWindow::updateWidgets()
{
	setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
	if(player->isFinished()&&playerTh)
	{
		if(!plistw->getRepeat())
		{
			timer->stop();stopped=true;playing=false;
			for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->stop();
			setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
			chnlw->resetAcitivity();
			player->playerDeinit();playerTh->join();
			delete playerTh;playerTh=NULL;
			if(singleFS)player->playerPanic(true);
			chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
			ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.png")));
			ui->hsTimer->setValue(0);
			ui->lnPolyphone->display("00000-00000");
			ui->lbCurTime->setText("00:00");
		}
		else
			switchTrack(plistw->getNextItem());
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
				for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
					LOAD_SOUNDFONT;
				}
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
		//sprintf(ts,"Poly: %d/%d",player->getPolyphone(),player->getMaxPolyphone());
		ui->lnPolyphone->display(QString("%1-%2").arg(player->getPolyphone(),5,10,QChar('0'))
		.arg(player->getMaxPolyphone(),5,10,QChar('0')));
	}
}

QString qmpMainWindow::getFileName(){return ui->lbFileName->text();}
void qmpMainWindow::switchTrack(QString s)
{
	timer->stop();player->playerDeinit();playerTh->join();
	delete playerTh;playerTh=NULL;
	ui->hsTimer->setValue(0);
	for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->stop();
	if(singleFS)player->playerPanic(true);
	chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=s;setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.'))+" - QMidiPlayer");
	ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
	onfnChanged();
	LOAD_FILE;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
			LOAD_SOUNDFONT;
		}}
	for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->start();
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
	player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
	SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(UPDATE_INTERVAL);
}
std::string qmpMainWindow::getTitle()
{
	if(!qmpSettingsWindow::getSettingsIntf())return "";
	if(qmpSettingsWindow::getSettingsIntf()->value("Midi/TextEncoding","").toString()
		=="Unicode")return std::string(player->getTitle());
	return QTextCodec::codecForName(
				qmpSettingsWindow::getSettingsIntf()->value("Midi/TextEncoding","").
				toString().toStdString().c_str())->
			toUnicode(player->getTitle()).toStdString();
}
std::wstring qmpMainWindow::getWTitle()
{
	if(!qmpSettingsWindow::getSettingsIntf())return L"";
	if(qmpSettingsWindow::getSettingsIntf()->value("Midi/TextEncoding","").toString()
		=="Unicode")return QString(player->getTitle()).toStdWString();
	return QTextCodec::codecForName(
				qmpSettingsWindow::getSettingsIntf()->value("Midi/TextEncoding","").
				toString().toStdString().c_str())->
			toUnicode(player->getTitle()).toStdWString();
}

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
		}setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.'))+" - QMidiPlayer");
		ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
		onfnChanged();
		LOAD_FILE;
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
		ui->lbFinTime->setText(ts);
		player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
			LOAD_SOUNDFONT;
		}
		}
		for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->start();
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
			player->playerPanic();chnlw->resetAcitivity();
			offset=ui->hsTimer->value()/100.*player->getFtime();
		}
		else
		{
			st=std::chrono::steady_clock::now();
			player->setResumed();
		}
		player->setTCpaused(!playing);
		for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->pause();
	}
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(playing?":/img/pause.png":":/img/play.png")));
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
		if(stopped){ui->hsTimer->setValue(0);return;}
		player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
		offset=ui->hsTimer->value()/100.*player->getFtime();
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)(offset)/60,(int)(offset)%60);
		ui->lbCurTime->setText(ts);
	}
}

uint32_t qmpMainWindow::getPlaybackPercentage(){return ui->hsTimer->value();}
void qmpMainWindow::playerSeek(uint32_t percentage)
{
	if(percentage>100)percentage=100;
	if(percentage<0)percentage=0;
	if(playing)
	{
		if(percentage==100){on_pbNext_clicked();return;}
		player->setTCeptr(player->getStamp(percentage),percentage);
		player->playerPanic();ui->hsTimer->setValue(percentage);
		offset=percentage/100.*player->getFtime();
		st=std::chrono::steady_clock::now();
	}
	else
	{
		if(stopped){ui->hsTimer->setValue(0);return;}
		player->setTCeptr(player->getStamp(percentage),percentage);
		offset=percentage/100.*player->getFtime();ui->hsTimer->setValue(percentage);
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
		for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->stop();
		player->playerDeinit();
		setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
		if(singleFS)player->playerPanic(true);chnlw->resetAcitivity();
		if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
		chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
		ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.png")));
		ui->hsTimer->setValue(0);
		ui->lnPolyphone->display("00000-00000");
		ui->lbCurTime->setText("00:00");
	}
}

void qmpMainWindow::dialogClosed()
{
	if(!settingsw->isVisible())ui->pbSettings->setChecked(false);
}

void qmpMainWindow::on_pbPrev_clicked()
{
	switchTrack(plistw->getPrevItem());
}

void qmpMainWindow::on_pbNext_clicked()
{
	switchTrack(plistw->getNextItem());
}

void qmpMainWindow::selectionChanged()
{
	stopped=false;playing=true;
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/pause.png")));
	timer->stop();player->playerDeinit();
	for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->stop();
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	if(singleFS)player->playerPanic(true);
	ui->hsTimer->setValue(0);
	chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=plistw->getSelectedItem();
	ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
	setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.'))+" - QMidiPlayer");
	onfnChanged();
	LOAD_FILE;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();if(!singleFS){playerSetup();player->fluidInitialize();
		for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
			LOAD_SOUNDFONT;
		}}
	for(auto i=mfunc.begin();i!=mfunc.end();++i)if(i->second.isVisualization())((qmpVisualizationIntf*)(i->second.i()))->start();
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
	player->setWaitVoice(qmpSettingsWindow::getSettingsIntf()->value("Midi/WaitVoice",1).toInt());
	playerTh=new std::thread(&CMidiPlayer::playerThread,player);
#ifdef _WIN32
			SetThreadPriority(playerTh->native_handle(),THREAD_PRIORITY_TIME_CRITICAL);
#endif
	st=std::chrono::steady_clock::now();offset=0;
	timer->start(UPDATE_INTERVAL);
}

void qmpMainWindow::on_lbFileName_customContextMenuRequested(const QPoint &pos)
{
	QMenu menu(ui->lbFileName);
	menu.addActions(ui->lbFileName->actions());
	menu.exec(this->pos()+ui->lbFileName->pos()+pos);
}

void qmpMainWindow::onfnChanged()
{
	if(!ui->lbFileName->text().length())return;
	QFont f=ui->lbFileName->font();f.setPointSize(18);
	QFontMetrics fm(f);
	QSize size=fm.size(0,ui->lbFileName->text());
	double fw=ui->lbFileName->width()/(double)size.width();
	double fh=ui->lbFileName->height()/(double)size.height();
	double ps=floor(f.pointSizeF()*(fw<fh?fw:fh));if(ps<6)ps=6;
	f.setPointSizeF(ps>18?18:ps);
	ui->lbFileName->setFont(f);
}

void qmpMainWindow::registerVisualizationIntf(qmpVisualizationIntf* intf,std::string name,std::string desc,const char* icon,int iconlen)
{
	registerFunctionality(intf,name,desc,icon,iconlen,true,true);
}
void qmpMainWindow::unregisterVisualizationIntf(std::string name)
{
	unregisterFunctionality(name);
}

void qmpMainWindow::registerFunctionality(qmpFuncBaseIntf *i,std::string name,std::string desc,const char *icon,int iconlen,bool checkable,bool isv)
{
	if(mfunc.find(name)!=mfunc.end())return;
	mfunc[name]=qmpFuncPrivate(i,desc,icon,iconlen,checkable,isv);
}

void qmpMainWindow::unregisterFunctionality(std::string name)
{
	mfunc.erase(name);
	for(auto i=enabled_actions.begin();i!=enabled_actions.end();++i)
	if(*i==name){enabled_actions.erase(i);break;}
	for(auto i=enabled_buttons.begin();i!=enabled_buttons.end();++i)
	if(*i==name){enabled_buttons.erase(i);break;}
	setupWidget();
}

void qmpMainWindow::setFuncState(std::string name,bool state)
{mfunc[name].setChecked(state);}
void qmpMainWindow::setFuncEnabled(std::string name,bool enable)
{mfunc[name].setEnabled(enable);}

bool qmpMainWindow::isDarkTheme()
{
	if(!qmpSettingsWindow::getSettingsIntf()->value("Behavior/IconTheme",0).toInt())
	{
		return ui->centralWidget->palette().color(QPalette::Background).lightness()<128;
	}
	else return 2-qmpSettingsWindow::getSettingsIntf()->value("Behavior/IconTheme",0).toInt();
}

void qmpMainWindow::startRender()
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
	for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
		LOAD_SOUNDFONT;
	}
	player->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();timer->start(UPDATE_INTERVAL);
	renderTh=new std::thread(&CMidiPlayer::rendererThread,player);
}

void qmpMainWindow::reloadSynth()
{
	player->fluidDeinitialize();player->fluidPreInitialize();playerSetup();player->fluidInitialize();
	for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i){if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
		LOAD_SOUNDFONT;
	}
}

std::vector<std::string>& qmpMainWindow::getWidgets(int w)
{return w?enabled_actions:enabled_buttons;}
std::map<std::string,qmpFuncPrivate>& qmpMainWindow::getFunc()
{return mfunc;}

void qmpMainWindow::setupWidget()
{
	QList<QWidget*>w=ui->buttonwidget->findChildren<QWidget*>("",Qt::FindDirectChildrenOnly);
	for(unsigned i=0;i<w.size();++i)
	delete w[i];
	QList<QAction*>a=ui->lbFileName->actions();
	for(unsigned i=0;i<a.size();++i)
	{
		ui->lbFileName->removeAction(a[i]);
		delete a[i];
	}
	for(unsigned i=0;i<enabled_buttons.size();++i)
	{
		QReflectivePushButton *pb=new QReflectivePushButton(
			mfunc[enabled_buttons[i]].icon(),
			tr(mfunc[enabled_buttons[i]].desc().c_str()),
			enabled_buttons[i]
		);
		setButtonHeight(pb,32);
		setButtonWidth(pb,32);
		ui->buttonwidget->layout()->addWidget(pb);
		mfunc[enabled_buttons[i]].setAssignedControl(pb);
		connect(pb,SIGNAL(onClick(std::string)),this,SLOT(funcReflector(std::string)));
	}
	for(unsigned i=0;i<enabled_actions.size();++i)
	{
		QReflectiveAction *a=new QReflectiveAction(
			mfunc[enabled_actions[i]].icon(),
			tr(mfunc[enabled_actions[i]].desc().c_str()),
			enabled_actions[i]
		);
		ui->lbFileName->addAction(a);
		mfunc[enabled_actions[i]].setAssignedControl(a);
		connect(a,SIGNAL(onClick(std::string)),this,SLOT(funcReflector(std::string)));
	}
	ui->buttonwidget->layout()->setAlignment(Qt::AlignLeft);
}

void qmpMainWindow::on_pbSettings_clicked()
{
	if(ui->pbSettings->isChecked())settingsw->show();else settingsw->close();
}

void qmpMainWindow::funcReflector(std::string reflt)
{
	if(mfunc[reflt].isCheckable())
	{
		mfunc[reflt].setChecked(!mfunc[reflt].isChecked());
		if(mfunc[reflt].isChecked())
			mfunc[reflt].i()->show();
		else
			mfunc[reflt].i()->close();
	}
	else mfunc[reflt].i()->show();
}

void qmpMainWindow::on_pushButton_clicked()
{
	helpw->show();
}

qmpFuncPrivate::qmpFuncPrivate(qmpFuncBaseIntf *i,std::string _desc,const char *icon,int iconlen,bool checkable,bool _isv):
	_i(i),des(_desc),_checkable(checkable),visual(_isv)
{
	if(icon)
	{
		QImage img;
		if(icon[0]==':'&&icon[1]=='/'||icon[0]=='q'&&icon[1]=='r'&&icon[2]=='c')
			img=QImage(QString(icon));
		else
			img.loadFromData((uchar*)icon,iconlen);
		QPixmap pixm;pixm.convertFromImage(img);
		_icon=QIcon(pixm);
	}else _icon=QIcon();
	checked=false;
	asgna=NULL;asgnb=NULL;
}
