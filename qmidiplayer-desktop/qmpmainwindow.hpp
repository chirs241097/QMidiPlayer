#ifndef QMPMAINWINDOW_H
#define QMPMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <thread>
#include <chrono>
#include "../core/qmpmidiplay.hpp"
#include "qmpplistwindow.hpp"
#include "qmpchannelswindow.hpp"
#include "qmpefxwindow.hpp"
#include "qmpinfowindow.hpp"
#include "qmpsettingswindow.hpp"
#include "qmphelpwindow.hpp"

namespace Ui {
	class qmpMainWindow;
}

class qmpMainWindow:public QMainWindow
{
	Q_OBJECT

	public:
		explicit qmpMainWindow(QWidget *parent = 0);
		void init();
		void closeEvent(QCloseEvent *event);
		void moveEvent(QMoveEvent *event);
		void dropEvent(QDropEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);
		~qmpMainWindow();
		CMidiPlayer* getPlayer(){return player;}
		QTimer* getTimer(){return timer;}
		bool isFinalizing(){return fin;}
		QString getFileName();
		int pharseArgs();

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
		void on_pbSettings_clicked();
		void onfnA1();
		void onfnA2();
		void onfnA3();

		void on_pushButton_clicked();

	public slots:
		void dialogClosed();
		void selectionChanged();

	private:
		Ui::qmpMainWindow *ui;
		QTimer *timer;
		bool playing,stopped,dragging,fin,singleFS,havemidi;
		std::thread *playerTh=NULL;
		std::thread *renderTh=NULL;
		std::chrono::steady_clock::time_point st;
		double offset;
		CMidiPlayer *player;
		qmpPlistWindow *plistw;
		qmpChannelsWindow *chnlw;
		qmpEfxWindow *efxw;
		qmpInfoWindow *infow;
		qmpSettingsWindow *settingsw;
		qmpHelpWindow *helpw;

		QAction *fnA1,*fnA2,*fnA3;
		void playerSetup();

	private:
		static qmpMainWindow* ref;
	public: static qmpMainWindow* getInstance(){return ref;}
};

#endif // QMPMAINWINDOW_H
