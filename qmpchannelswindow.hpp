#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include "qmppresetselect.hpp"
#include "qmpchanneleditor.hpp"

namespace Ui {
	class qmpChannelsWindow;
}

class QDCLabel:public QLabel
{
	Q_OBJECT
	using QLabel::QLabel;
	private:
		int id;
	protected:
		void mouseDoubleClickEvent(QMouseEvent *event){event->accept();emit onDoubleClick(id);}
	public:
		void setID(int _id){id=_id;}
	signals:
		void onDoubleClick(int id);
};

class QDCPushButton:public QPushButton
{
	Q_OBJECT
	using QPushButton::QPushButton;
	private:
		int id;
	protected:
		void mousePressEvent(QMouseEvent *event){QPushButton::mousePressEvent(event);emit onClick(id);}
	public:
		void setID(int _id){id=_id;}
	signals:
		void onClick(int id);
};

class qmpChannelsWindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpChannelsWindow(QWidget *parent = 0);
		~qmpChannelsWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void moveEvent(QMoveEvent *event);
	signals:
		void dialogClosing();
	public slots:
		void channelWindowsUpdate();
		void channelMSChanged();
		void showPresetWindow(int chid);
		void showChannelEditorWindow(int chid);
		void on_pbUnmute_clicked();
		void on_pbUnsolo_clicked();

	private:
		Ui::qmpChannelsWindow *ui;
		qmpPresetSelector *pselectw;
		qmpChannelEditor *ceditw;
};

#endif // QMPCHANNELSWINDOW_H
