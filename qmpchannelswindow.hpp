#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QLabel>
#include <QDialog>
#include <QCloseEvent>
#include "qmppresetselect.hpp"

namespace Ui {
	class qmpchannelswindow;
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

class qmpchannelswindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpchannelswindow(QWidget *parent = 0);
		~qmpchannelswindow();
		void closeEvent(QCloseEvent *event);
	signals:
		void dialogClosing();
	public slots:
		void channelWindowsUpdate();
		void channelMSChanged();
		void showPresetWindow(int chid);
	private slots:
		void on_pbUnmute_clicked();

		void on_pbUnsolo_clicked();

	private:
		Ui::qmpchannelswindow *ui;
		qmppresetselect *pselectw;
};

#endif // QMPCHANNELSWINDOW_H
