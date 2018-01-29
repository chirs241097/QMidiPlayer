#ifndef QMPMAINWINDOW_H
#define QMPMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QIcon>
#include <QSlider>
#include <QPointer>
#include <QApplication>
#include <thread>
#include <chrono>
#include <future>
#include <map>
#include <unordered_map>
#include "../core/qmpmidiplay.hpp"
#include "qmpplugin.hpp"
#include "qmpplistwindow.hpp"
#include "qmpchannelswindow.hpp"
#include "qmpefxwindow.hpp"
#include "qmpinfowindow.hpp"
#include "qmpsettingswindow.hpp"
#include "qmphelpwindow.hpp"

#define getThemedIcon(x) (qmpMainWindow::getInstance()->isDarkTheme()?QString(x).insert(QString(x).lastIndexOf('.'),"_i"):QString(x))
#define getThemedIconc(x) ((qmpMainWindow::getInstance()->isDarkTheme()?QString(x).insert(QString(x).lastIndexOf('.'),"_i"):QString(x)).toStdString().c_str())

namespace Ui {
	class qmpMainWindow;
}

class QClickableSlider:public QSlider
{
	Q_OBJECT
	public:
		explicit QClickableSlider(QWidget *parent=0):QSlider(parent){}
	protected:
		void mouseReleaseEvent(QMouseEvent *e)
		{
			QSlider::mouseReleaseEvent(e);
			if(e->buttons()^Qt::LeftButton)
			{
				double p=e->pos().x()/(double)width();
				setValue(p*(maximum()-minimum())+minimum());
				emit sliderReleased();
			}
		}
};

class QReflectiveAction:public QAction
{
	Q_OBJECT
	private:
		std::string reflt;
	signals:
		void onClick(std::string s);
	private slots:
		void triggerslot(){
			emit(onClick(reflt));
		}
	public:
		explicit QReflectiveAction(const QIcon& icon,const QString& text,const std::string& ref):
		QAction(icon,text,NULL),reflt(ref){
			connect(this,SIGNAL(triggered(bool)),this,SLOT(triggerslot()));
		}
};

class QReflectivePushButton:public QPushButton
{
	Q_OBJECT
	private:
		std::string reflt;
	signals:
		void onClick(std::string s);
	private slots:
		void clickslot(){
			emit(onClick(reflt));
		}
	public:
		explicit QReflectivePushButton(const QIcon& icon,const QString& text,const std::string& ref):
		QPushButton(icon,""),reflt(ref){
			connect(this,SIGNAL(clicked(bool)),this,SLOT(clickslot()));
			setToolTip(text);
		}
};

class qmpFuncPrivate
{
	private:
		qmpFuncBaseIntf* _i;
		QIcon _icon;
		std::string des;
		bool _checkable,checked;
		QReflectiveAction* asgna;
		QReflectivePushButton* asgnb;
	public:
		qmpFuncPrivate(){}
		qmpFuncPrivate(qmpFuncBaseIntf* i,std::string _desc,const char* icon,int iconlen,bool checkable);
		~qmpFuncPrivate(){asgna=NULL;asgnb=NULL;}
		qmpFuncBaseIntf* i(){return _i;}
		void setAssignedControl(QReflectiveAction* a){asgna=a;if(!a)return;asgna->setCheckable(_checkable);asgna->setChecked(checked);}
		void setAssignedControl(QReflectivePushButton* a){asgnb=a;if(!a)return;asgnb->setCheckable(_checkable);asgnb->setChecked(checked);}
		const QIcon& icon(){return _icon;}
		const std::string& desc(){return des;}
		bool isCheckable(){return _checkable;}
		bool isChecked(){return checked;}
		void setEnabled(bool e){if(asgna)asgna->setEnabled(e);if(asgnb)asgnb->setEnabled(e);}
		void setChecked(bool _c){checked=_c;if(asgna)asgna->setChecked(checked);if(asgnb)asgnb->setChecked(checked);}
};

class qmpRenderFunc;
class qmpPanicFunc;
class qmpReloadSynthFunc;

class qmpCallBack
{
	private:
		int t;
		ICallBack* cbc;
		callback_t cbf;
	public:
		qmpCallBack(){t=-1;cbc=NULL;cbf=NULL;}
		qmpCallBack(ICallBack* _cb){t=0;cbc=_cb;cbf=NULL;}
		qmpCallBack(callback_t _cb){t=1;cbf=_cb;cbc=NULL;}
		void operator ()(void* cbd,void* usrd)
		{
			if(t<0)return;
			if(t)cbf(cbd,usrd);
			else cbc->callBack(cbd,usrd);
		}
};

class qmpMainWindow:public QMainWindow
{
	Q_OBJECT

	public:
		explicit qmpMainWindow(QWidget *parent = 0);
		void init();
		void closeEvent(QCloseEvent *event);
		void dropEvent(QDropEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);
		~qmpMainWindow();
		CMidiPlayer* getPlayer(){return player;}
		qmpSettingsWindow* getSettingsWindow(){return settingsw;}
		QTimer* getTimer(){return timer;}
		bool isFinalizing(){return fin;}
		QString getFileName();
		void switchTrack(QString s);
		std::string getTitle();
		std::wstring getWTitle();
		uint32_t getPlaybackPercentage();
		void playerSeek(uint32_t percentage);
		int pharseArgs();
		void registerFunctionality(qmpFuncBaseIntf* i,std::string name,std::string desc,const char* icon,int iconlen,bool checkable);
		void unregisterFunctionality(std::string name);
		int registerUIHook(std::string e,ICallBack* callback,void* userdat);
		int registerUIHook(std::string e,callback_t callback,void* userdat);
		void unregisterUIHook(std::string e,int hook);
		void setFuncState(std::string name,bool state);
		void setFuncEnabled(std::string name,bool enable);
		bool isDarkTheme();
		void startRender();
		void reloadSynth();
		void setupWidget();
		void invokeCallback(std::string cat,void *callerdat);
		std::vector<std::string>& getWidgets(int w);
		std::map<std::string,qmpFuncPrivate>& getFunc();

	private slots:
		void on_pbPlayPause_clicked();
		void updateWidgets();
		void on_hsTimer_sliderPressed();
		void on_hsTimer_sliderReleased();
		void on_vsMasterVol_valueChanged();
		void on_pbStop_clicked();
		void on_pbPrev_clicked();
		void on_pbNext_clicked();
		void on_lbFileName_customContextMenuRequested(const QPoint &pos);
		void on_pbSettings_clicked();
		void funcReflector(std::string reflt);
		void on_pushButton_clicked();
		void on_pbAdd_clicked();

	public slots:
		void dialogClosed();
		void selectionChanged();

	private:
		Ui::qmpMainWindow *ui;
		QTimer *timer;
		bool playing,stopped,dragging,fin,havemidi;
		std::thread *playerTh=NULL;
		std::thread *renderTh=NULL;
		std::chrono::steady_clock::time_point st;
		double offset;
		CMidiPlayer *player;
		qmpRtMidiManager *rtmididev;
		qmpFileRendererFluid *fluidrenderer;
		qmpPluginManager *pmgr;
		QPointer<qmpPlistWindow> plistw;
		QPointer<qmpChannelsWindow> chnlw;
		QPointer<qmpEfxWindow> efxw;
		QPointer<qmpInfoWindow> infow;
		QPointer<qmpSettingsWindow> settingsw;
		QPointer<qmpHelpWindow> helpw;
		std::map<std::string,qmpFuncPrivate> mfunc;
		std::unordered_map<std::string,std::map<int,std::pair<qmpCallBack,void*>>> muicb;
		qmpRenderFunc* renderf;
		qmpPanicFunc* panicf;
		qmpReloadSynthFunc* reloadsynf;
		std::vector<std::string> enabled_buttons,enabled_actions;

		void onfnChanged();
		void playerSetup(IFluidSettings *fs);
		void loadSoundFont(IFluidSettings *fs);
		int loadFile(QString fns);

	private:
		static qmpMainWindow* ref;
	public: static qmpMainWindow* getInstance(){return ref;}
};

class qmpRenderFunc:public qmpFuncBaseIntf
{
	private:
		qmpMainWindow *p;
	public:
		qmpRenderFunc(qmpMainWindow *par){p=par;}
		void show(){p->startRender();}
		void close(){}
};
class qmpPanicFunc:public qmpFuncBaseIntf
{
	private:
		qmpMainWindow *p;
	public:
		qmpPanicFunc(qmpMainWindow *par){p=par;}
		void show(){p->getPlayer()->playerPanic();}
		void close(){}
};
class qmpReloadSynthFunc:public qmpFuncBaseIntf
{
	private:
		qmpMainWindow *p;
	public:
		qmpReloadSynthFunc(qmpMainWindow *par){p=par;}
		void show(){p->reloadSynth();}
		void close(){}
};

#endif // QMPMAINWINDOW_H
