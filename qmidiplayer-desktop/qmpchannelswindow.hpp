#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QComboBox>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include "qmppresetselect.hpp"
#include "qmpchanneleditor.hpp"
#include "../core/qmpmidiplay.hpp"
#include "../core/qmpmidimappers.hpp"

namespace Ui {
	class qmpChannelsWindow;
}

class QDCLabel:public QLabel
{
	Q_OBJECT
	private:
		int id;
	protected:
		void mouseDoubleClickEvent(QMouseEvent *event){event->accept();emit onDoubleClick(id);}
	public:
		QDCLabel(QString s):QLabel(s){id=-1;}
		void setID(int _id){id=_id;}
	signals:
		void onDoubleClick(int id);
};

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
	signals:
		void onChange(int id,int idx);
	public slots:
		void indexChangedSlot(int idx){emit(onChange(id,idx));}
};

class qmpCWNoteOnCB:public QObject,public IMidiCallBack
{
	Q_OBJECT
	public:
		void callBack(void* callerdata,void*)
		{if(((((SEventCallBackData*)callerdata)->type)&0xF0)==0x90)emit onNoteOn();}
	signals:
		void onNoteOn();
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

class qmpChannelsWindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpChannelsWindow(QWidget *parent = 0);
		~qmpChannelsWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void resetAcitivity();
	public slots:
		void channelWindowsUpdate();
		void updateChannelActivity();
		void channelMSChanged();
		void showPresetWindow(int chid);
		void showChannelEditorWindow(int chid);
		void changeMidiMapping(int chid,int idx);
		void on_pbUnmute_clicked();
		void on_pbUnsolo_clicked();

	private:
		Ui::qmpChannelsWindow *ui;
		qmpPresetSelector *pselectw;
		qmpChannelEditor *ceditw;
		qmpMidiMapperRtMidi *mapper;
		QIcon *cha,*chi;
		qmpCWNoteOnCB *cb;
		qmpChannelFunc *chnlf;
		//callback fuse... (avoid black midi blocking the main thread)
		int callbacksc,cbcnt,fused;
};

#endif // QMPCHANNELSWINDOW_H
