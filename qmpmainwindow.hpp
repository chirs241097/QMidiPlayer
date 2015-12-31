#ifndef QMPMAINWINDOW_H
#define QMPMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <thread>
#include <chrono>
#include "qmpmidiplay.hpp"
#include "qmpplistwindow.hpp"
#include "qmpchannelswindow.hpp"
#include "qmpefxwindow.hpp"
#include "qmpinfowindow.hpp"

namespace Ui {
	class qmpMainWindow;
}

class qmpMainWindow:public QMainWindow
{
	Q_OBJECT

	public:
		explicit qmpMainWindow(QWidget *parent = 0);
		void closeEvent(QCloseEvent *event);
		~qmpMainWindow();
		CMidiPlayer* getPlayer(){return player;}
		QTimer* getTimer(){return timer;}

	private slots:
		void on_pbPlayPause_clicked();
		void updateWidgets();
		void on_hsTimer_sliderPressed();
		void on_hsTimer_sliderReleased();
		void on_vsMasterVol_valueChanged();
		void on_pbStop_clicked();
		void on_pbPList_clicked();
		void on_pbPrev_clicked();
		void on_pbNext_clicked();
		void on_pbChannels_clicked();
		void on_pbEfx_clicked();
		void on_lbFileName_customContextMenuRequested(const QPoint &pos);
		void onfnA1();

	public slots:
		void dialogClosed();
		void selectionChanged();

	private:
		Ui::qmpMainWindow *ui;
		QTimer *timer;
		bool playing,stopped,dragging;
		std::thread *playerTh=NULL;
		std::chrono::steady_clock::time_point st;
		double offset;
		CMidiPlayer *player;
		qmpPlistWindow *plistw;
		qmpChannelsWindow *chnlw;
		qmpEfxWindow *efxw;
		qmpInfoWindow *infow;
		QAction *fnA1,*fnA2;
	public:
		QString getFileName();

		static qmpMainWindow* ref;
	public: static qmpMainWindow* getInstance(){return ref;}
};

#endif // QMPMAINWINDOW_H
