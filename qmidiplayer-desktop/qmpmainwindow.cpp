#include <cstdio>
#include <cmath>
#include <functional>
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
#endif
#define UPDATE_INTERVAL 66

qmpMainWindow* qmpMainWindow::ref=NULL;

qmpMainWindow::qmpMainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::qmpMainWindow)
{
	ui->setupUi(this);
	ui->lbCurPoly->setText("00000");ui->lbMaxPoly->setText("00000");
	ui->lbFileName->setText("");ref=this;ui->verticalLayout->setAlignment(ui->pushButton,Qt::AlignRight);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
	setButtonHeight(ui->pbNext,36);setButtonHeight(ui->pbPlayPause,36);setButtonHeight(ui->pbAdd,36);
	setButtonHeight(ui->pbPrev,36);setButtonHeight(ui->pbSettings,36);setButtonHeight(ui->pbStop,36);
	playing=false;stopped=true;dragging=false;fin=false;
	settingsw=new qmpSettingsWindow(this);pmgr=new qmpPluginManager();
	plistw=new qmpPlistWindow(this);player=NULL;timer=NULL;fluidrenderer=NULL;
}

qmpMainWindow::~qmpMainWindow()
{
	QList<QAction*>a=ui->lbFileName->actions();
	for(unsigned i=0;i<a.size();++i)
	{
		ui->lbFileName->removeAction(a[i]);
		delete a[i];
	}
	pmgr->deinitPlugins();
	std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> rtdev=rtmididev->getDevices();
	for(auto &i:rtdev)player->unregisterMidiOutDevice(i.second);
	rtmididev->deleteDevices();
	delete pmgr;if(player)delete player;
	if(timer)delete timer;
	delete helpw;helpw=NULL;
	delete efxw;efxw=NULL;
	delete chnlw;chnlw=NULL;
	delete plistw;plistw=NULL;
	delete infow;infow=NULL;
	delete settingsw;settingsw=NULL;
	delete panicf;panicf=NULL;
	delete renderf;renderf=NULL;
	delete reloadsynf;reloadsynf=NULL;
	delete ui;
}

void qmpMainWindow::init()
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus",0).toInt())
	{
		QRect g=qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/MainW",QRect(-999,-999,999,999)).toRect();
		if(g!=QRect(-999,-999,999,999))setGeometry(g);
	}show();

	ui->centralWidget->setEnabled(false);
	std::future<void> f=std::async(std::launch::async,
		[this]
		{
			player=new CMidiPlayer();
			rtmididev=new qmpRtMidiManager();
			rtmididev->createDevices();
			std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> rtdev=rtmididev->getDevices();
			for(auto &i:rtdev)player->registerMidiOutDevice(i.first,i.second);
			reloadsynf=new qmpReloadSynthFunc(this);
			playerSetup(player->fluid());player->fluid()->deviceInit();
			loadSoundFont(player->fluid());
		}
	);
	while(f.wait_for(std::chrono::milliseconds(100))==std::future_status::timeout);
	ui->centralWidget->setEnabled(true);

	chnlw=new qmpChannelsWindow(this);
	efxw=new qmpEfxWindow(this);
	infow=new qmpInfoWindow(this);
	helpw=new qmpHelpWindow(this);
	timer=new QTimer(this);
	renderf=new qmpRenderFunc(this);
	panicf=new qmpPanicFunc(this);
	registerFunctionality(renderf,"Render",tr("Render to wave").toStdString(),getThemedIconc(":/img/render.svg"),0,false);
	registerFunctionality(panicf,"Panic",tr("Panic").toStdString(),getThemedIconc(":/img/panic.svg"),0,false);
	registerFunctionality(reloadsynf,"ReloadSynth",tr("Restart fluidsynth").toStdString(),getThemedIconc(":/img/repeat-base.svg"),0,false);
	pmgr->scanPlugins();settingsw->updatePluginList(pmgr);pmgr->initPlugins();
	ui->vsMasterVol->setValue(qmpSettingsWindow::getSettingsIntf()->value("Audio/Gain",50).toInt());
	connect(timer,SIGNAL(timeout()),this,SLOT(updateWidgets()));
	connect(timer,SIGNAL(timeout()),chnlw,SLOT(channelWindowsUpdate()));
	connect(timer,SIGNAL(timeout()),infow,SLOT(updateInfo()));
	ui->pbNext->setIcon(QIcon(getThemedIcon(":/img/next.svg")));
	ui->pbPrev->setIcon(QIcon(getThemedIcon(":/img/prev.svg")));
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
	ui->pbStop->setIcon(QIcon(getThemedIcon(":/img/stop.svg")));
	ui->pbSettings->setIcon(QIcon(getThemedIcon(":/img/settings.svg")));
	ui->pbAdd->setIcon(QIcon(getThemedIcon(":/img/open.svg")));
	if(havemidi)on_pbPlayPause_clicked();
	setupWidget();settingsw->verifySF();
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
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/MainW",geometry());
	}
	on_pbStop_clicked();fin=true;
	for(auto i=mfunc.begin();i!=mfunc.end();++i)
	i->second.setAssignedControl((QReflectiveAction*)NULL),
	i->second.setAssignedControl((QReflectivePushButton*)NULL);
	efxw->close();chnlw->close();
	plistw->close();infow->close();
	settingsw->close();
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
			invokeCallback("main.stop",NULL);
			setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
			chnlw->resetAcitivity();
			player->playerDeinit();playerTh->join();
			delete playerTh;playerTh=NULL;
			player->playerPanic(true);
			chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
			ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
			ui->hsTimer->setValue(0);
			ui->lbCurPoly->setText("00000");ui->lbMaxPoly->setText("00000");
			ui->lbCurTime->setText("00:00");
		}
		else
			switchTrack(plistw->getNextItem());
	}
	if(renderTh)
	{
		if(fluidrenderer->isFinished())
		{
			renderTh->join();timer->stop();
			ui->centralWidget->setEnabled(true);
			delete renderTh;renderTh=NULL;
			fluidrenderer->renderDeinit();
			delete fluidrenderer;fluidrenderer=NULL;
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
		ui->lbCurPoly->setText(QString("%1").arg(player->fluid()->getPolyphone(),5,10,QChar('0')));
		ui->lbMaxPoly->setText(QString("%1").arg(player->fluid()->getMaxPolyphone(),5,10,QChar('0')));
	}
}

QString qmpMainWindow::getFileName(){return ui->lbFileName->text();}
void qmpMainWindow::switchTrack(QString s)
{
	stopped=false;playing=true;
	setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/pause.svg")));
	timer->stop();player->playerDeinit();
	invokeCallback("main.stop",NULL);
	if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
	player->playerPanic(true);
	ui->hsTimer->setValue(0);
	chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
	QString fns=s;setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.'))+" - QMidiPlayer");
	ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
	onfnChanged();
	if(!loadFile(fns))return;
	char ts[100];
	sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
	ui->lbFinTime->setText(ts);
	player->playerInit();
	invokeCallback("main.start",NULL);
	player->fluid()->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
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

void qmpMainWindow::playerSetup(IFluidSettings* fs)
{
	QSettings* settings=qmpSettingsWindow::getSettingsIntf();
	fs->setOptStr("audio.driver",settings->value("Audio/Driver","").toString().toStdString().c_str());
	fs->setOptInt("audio.period-size",settings->value("Audio/BufSize","").toInt());
	fs->setOptInt("audio.periods",settings->value("Audio/BufCnt","").toInt());
	fs->setOptStr("audio.sample-format",settings->value("Audio/Format","").toString().toStdString().c_str());
	fs->setOptInt("synth.sample-rate",settings->value("Audio/Frequency","").toInt());
	fs->setOptInt("synth.polyphony",settings->value("Audio/Polyphony","").toInt());
	fs->setOptInt("synth.cpu-cores",settings->value("Audio/Threads","").toInt());
	char bsmode[4];
	if(settings->value("Audio/AutoBS",1).toInt()&&player->getFileStandard())
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
	fs->setOptStr("synth.midi-bank-select",bsmode);
	player->sendSysX(settings->value("Midi/SendSysEx",1).toInt());
}
void qmpMainWindow::loadSoundFont(IFluidSettings *fs)
{
	for(int i=settingsw->getSFWidget()->rowCount()-1;i>=0;--i)
	{
		if(!((QCheckBox*)settingsw->getSFWidget()->cellWidget(i,0))->isChecked())continue;
#ifdef _WIN32
		char* c=wcsto8bit(settingsw->getSFWidget()->item(i,1)->text().toStdWString().c_str());
		fs->loadSFont(c);
		free(c);
#else
		fs->loadSFont(settingsw->getSFWidget()->item(i,1)->text().toStdString().c_str());
#endif
	}
}
int qmpMainWindow::loadFile(QString fns)
{
#ifdef _WIN32
	char* c=wcsto8bit(fns.toStdWString().c_str());
#else
	std::string s=fns.toStdString();
	const char* c=s.c_str();
#endif
	int ret=1;
	invokeCallback("main.reset",NULL);
	if(!player->playerLoadFile(c))
	{QMessageBox::critical(this,tr("Error"),tr("%1 is not a valid midi file.").arg(fns));ret=0;}
#ifdef _WIN32
	free(c);
#endif
	return ret;
}

void qmpMainWindow::on_pbPlayPause_clicked()
{
	playing=!playing;
	if(stopped)
	{
		QString fns=plistw->getFirstItem();
		if(!fns.length())
		{
			if(!plistw->on_pbAdd_clicked()){playing=false;return;}
			fns=plistw->getFirstItem();
			if(!fns.length())return(void)(playing=false);
		}setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.'))+" - QMidiPlayer");
		ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
		onfnChanged();
		if(!loadFile(fns))return;
		char ts[100];
		sprintf(ts,"%02d:%02d",(int)player->getFtime()/60,(int)player->getFtime()%60);
		ui->lbFinTime->setText(ts);
		player->playerInit();
		invokeCallback("main.start",NULL);
		player->fluid()->setGain(ui->vsMasterVol->value()/250.);efxw->sendEfxChange();
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
		invokeCallback("main.pause",NULL);
	}
	ui->pbPlayPause->setIcon(QIcon(getThemedIcon(playing?":/img/pause.svg":":/img/play.svg")));
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
		player->playerPanic();
		player->setTCeptr(player->getStamp(ui->hsTimer->value()),ui->hsTimer->value());
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
		player->playerPanic();ui->hsTimer->setValue(percentage);
		player->setTCeptr(player->getStamp(percentage),percentage);
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
	if(!stopped)player->fluid()->setGain(ui->vsMasterVol->value()/250.);
	qmpSettingsWindow::getSettingsIntf()->setValue("Audio/Gain",ui->vsMasterVol->value());
}

void qmpMainWindow::on_pbStop_clicked()
{
	if(!stopped)
	{
		timer->stop();stopped=true;playing=false;
		invokeCallback("main.stop",NULL);
		player->playerDeinit();
		setFuncEnabled("Render",stopped);setFuncEnabled("ReloadSynth",stopped);
		player->playerPanic(true);chnlw->resetAcitivity();
		if(playerTh){playerTh->join();delete playerTh;playerTh=NULL;}
		chnlw->on_pbUnmute_clicked();chnlw->on_pbUnsolo_clicked();
		ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
		ui->hsTimer->setValue(0);
		ui->lbCurPoly->setText("00000");ui->lbMaxPoly->setText("00000");
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
	switchTrack(plistw->getSelectedItem());
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

int qmpMainWindow::registerUIHook(std::string e,ICallBack *callback,void* userdat)
{
	std::map<int,std::pair<qmpCallBack,void*>>& m=muicb[e];
	int id=0;
	if(m.size())id=m.rbegin()->first+1;
	m[id]=std::make_pair(qmpCallBack(callback),userdat);
	return id;
}
int qmpMainWindow::registerUIHook(std::string e,callback_t callback,void *userdat)
{
	std::map<int,std::pair<qmpCallBack,void*>>& m=muicb[e];
	int id=0;
	if(m.size())id=m.rbegin()->first+1;
	m[id]=std::make_pair(qmpCallBack(callback),userdat);
	return id;
}
void qmpMainWindow::unregisterUIHook(std::string e,int hook)
{
	std::map<int,std::pair<qmpCallBack,void*>>& m=muicb[e];
	m.erase(hook);
}

void qmpMainWindow::registerFunctionality(qmpFuncBaseIntf *i,std::string name,std::string desc,const char *icon,int iconlen,bool checkable)
{
	if(mfunc.find(name)!=mfunc.end())return;
	mfunc[name]=qmpFuncPrivate(i,desc,icon,iconlen,checkable);
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
#ifdef _WIN32
	char* ofstr=wcsto8bit((plistw->getSelectedItem()+QString(".wav")).toStdWString().c_str());
	char* ifstr=wcsto8bit(plistw->getSelectedItem().toStdWString().c_str());
	fluidrenderer=new qmpFileRendererFluid(ifstr,ofstr);
	playerSetup(fluidrenderer);
	fluidrenderer->renderInit();
	free(ofstr);free(ifstr);
#else
	fluidrenderer=new qmpFileRendererFluid(
		plistw->getSelectedItem().toStdString().c_str(),
		(plistw->getSelectedItem()+QString(".wav")).toStdString().c_str()
	);
	playerSetup(fluidrenderer);
	fluidrenderer->renderInit();
#endif
	loadSoundFont(fluidrenderer);
	ui->centralWidget->setEnabled(false);
	fluidrenderer->setGain(ui->vsMasterVol->value()/250.);
	efxw->sendEfxChange(fluidrenderer);timer->start(UPDATE_INTERVAL);
	renderTh=new std::thread(&qmpFileRendererFluid::renderWorker,fluidrenderer);
}

void qmpMainWindow::reloadSynth()
{
	ui->centralWidget->setEnabled(false);
	std::future<void> f=std::async(std::launch::async,
		[this]
		{
				player->fluid()->deviceDeinit(true);
				playerSetup(player->fluid());
				player->fluid()->deviceInit();
				loadSoundFont(player->fluid());
		}
	);
	while(f.wait_for(std::chrono::milliseconds(100))==std::future_status::timeout);
	ui->centralWidget->setEnabled(true);
}

std::vector<std::string>& qmpMainWindow::getWidgets(int w)
{return w?enabled_actions:enabled_buttons;}
std::map<std::string,qmpFuncPrivate>& qmpMainWindow::getFunc()
{return mfunc;}

void qmpMainWindow::setupWidget()
{
	for(auto i=mfunc.begin();i!=mfunc.end();++i)
	i->second.setAssignedControl((QReflectiveAction*)NULL),
	i->second.setAssignedControl((QReflectivePushButton*)NULL);
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
		if(mfunc.find(enabled_buttons[i])==mfunc.end())continue;
		QReflectivePushButton *pb=new QReflectivePushButton(
			mfunc[enabled_buttons[i]].icon(),
			tr(mfunc[enabled_buttons[i]].desc().c_str()),
			enabled_buttons[i]
		);
		setButtonHeight(pb,32);
		if(getSettingsWindow()->getSettingsIntf()->value("Behavior/ShowButtonLabel",0).toInt())
		{
			pb->setText(tr(mfunc[enabled_buttons[i]].desc().c_str()));
			pb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
		}
		else
		setButtonWidth(pb,32);
		ui->buttonwidget->layout()->addWidget(pb);
		mfunc[enabled_buttons[i]].setAssignedControl(pb);
		connect(pb,SIGNAL(onClick(std::string)),this,SLOT(funcReflector(std::string)));
	}
	for(unsigned i=0;i<enabled_actions.size();++i)
	{
		if(mfunc.find(enabled_actions[i])==mfunc.end())continue;
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

void qmpMainWindow::invokeCallback(std::string cat,void* callerdat)
{
	std::map<int,std::pair<qmpCallBack,void*>> *mp;
	mp=&muicb[cat];
	for(auto&i:*mp)
		i.second.first(callerdat,i.second.second);
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

qmpFuncPrivate::qmpFuncPrivate(qmpFuncBaseIntf *i,std::string _desc,const char *icon,int iconlen,bool checkable):
	_i(i),des(_desc),_checkable(checkable)
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

void qmpMainWindow::on_pbAdd_clicked()
{
	if(plistw->on_pbAdd_clicked())
	switchTrack(plistw->getLastItem());
}
