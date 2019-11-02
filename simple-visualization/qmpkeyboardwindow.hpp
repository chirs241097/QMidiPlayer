#ifndef QMPKEYBOARDWINDOW_HPP
#define QMPKEYBOARDWINDOW_HPP

#include "../include/qmpcorepublic.hpp"

#include <QWidget>

class QLabel;
class qmpPianoWidget;
class qmpKeyboardWindow:public QWidget
{
	Q_OBJECT
	private:
		qmpPianoWidget *pw[16];
		QLabel *lb[16];
		qmpPluginAPI *api;
		int eh;
	public:
		qmpKeyboardWindow(qmpPluginAPI *_api,QWidget *parent);
		~qmpKeyboardWindow();
		void resetAll();
	protected:
		void closeEvent(QCloseEvent *event);
	signals:
		void keystateupdated(int ch,int key,bool state);
	public slots:
		void onkeystatesupdate(int ch,int key,bool state);
};

#endif
