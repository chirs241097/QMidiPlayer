#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <QComboBox>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include "qmppresetselect.hpp"
#include "qmpchanneleditor.hpp"
#include "../core/qmpmidiplay.hpp"
#include "../core/qmpmidioutrtmidi.hpp"

namespace Ui {
	class qmpChannelsWindow;
}

class QDCPushButton:public QPushButton
{
	Q_OBJECT
	private:
		int id;
	protected:
		void mousePressEvent(QMouseEvent *event){QPushButton::mousePressEvent(event);emit onClick(id);}
	public:
		QDCPushButton(QString s):QPushButton(s){id=-1;}
		void setID(int _id){id=_id;}
		QSize sizeHint()const{return QSize();}
	signals:
		void onClick(int id);
};

class QDCComboBox:public QComboBox
{
	Q_OBJECT
	private:
		int id;
	public:
		QDCComboBox():QComboBox(){id=-1;connect(this,SIGNAL(currentIndexChanged(int)),this,SLOT(indexChangedSlot(int)));}
		void setID(int _id){id=_id;}
		QSize sizeHint()const{return QSize();}
		QSize minimumSizeHint()const{return QSize();}
	signals:
		void onChange(int id,int idx);
	public slots:
		void indexChangedSlot(int idx){emit(onChange(id,idx));}
};

class qmpChannelsWindow;

class qmpChannelFunc:public qmpFuncBaseIntf
{
	private:
		qmpChannelsWindow *p;
	public:
		qmpChannelFunc(qmpChannelsWindow *par);
		void show();
		void close();
};

class qmpChannelsWindow:public QWidget
{
	Q_OBJECT

	public:
		explicit qmpChannelsWindow(QWidget *parent=nullptr);
		~qmpChannelsWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void resetAcitivity();
	public slots:
		void channelWindowsUpdate();
		void updateChannelActivity();
		void channelMSChanged();
		void showPresetWindow(int chid,int col);
		void showChannelEditorWindow(int chid);
		void changeMidiMapping(int chid,int idx);
		void on_pbUnmute_clicked();
		void on_pbUnsolo_clicked();

	signals:
		void noteOn();

	protected:
		bool eventFilter(QObject *o,QEvent *e);

	private:
		Ui::qmpChannelsWindow *ui;
		qmpPresetSelector *pselectw;
		qmpChannelEditor *ceditw;
		QIcon *cha,*chi;
		qmpChannelFunc *chnlf;
		int eh;
		//callback fuse... (avoid black midi blocking the main thread)
		int callbacksc,cbcnt,fused;
};

#endif // QMPCHANNELSWINDOW_H
