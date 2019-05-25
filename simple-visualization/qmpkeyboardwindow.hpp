#ifndef QMPKEYBOARDWINDOW_HPP
#define QMPKEYBOARDWINDOW_HPP

#include "../include/qmpcorepublic.hpp"
#include "qmppianowidget.hpp"

#include <QWidget>

class EventCallback:public QObject,public ICallBack
{
	Q_OBJECT
	public:
		void callBack(const void *callerdata,void *userdata);
	signals:
		void keystateupdated(int ch,int key,bool state);
};

class qmpKeyboardWindow:public QWidget
{
	Q_OBJECT
	friend class EventCallback;
	private:
		qmpPianoWidget *pw[16];
		qmpPluginAPI *api;
		EventCallback *ec;
	public:
		qmpKeyboardWindow(qmpPluginAPI *_api,QWidget *parent);
		~qmpKeyboardWindow();
		void resetAll();
	protected:
		void closeEvent(QCloseEvent *event);
	public slots:
		void onkeystatesupdate(int ch,int key,bool state);
};

#endif
