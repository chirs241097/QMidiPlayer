#ifndef QMPMAINWINDOW_H
#define QMPMAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QCloseEvent>
#include <thread>
#include <chrono>
#include "qmpmidiplay.hpp"
#include "qmpplistwindow.hpp"
#include "qmpchannelswindow.hpp"

namespace Ui {
	class qmpMainWindow;
}

class qmpMainWindow : public QMainWindow
{
	Q_OBJECT

	public:
		explicit qmpMainWindow(QWidget *parent = 0);
		void closeEvent(QCloseEvent *event);
		~qmpMainWindow();
		CMidiPlayer* getPlayer(){return player;}

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
		qmpplistwindow *plistw;
		qmpchannelswindow *chnlw;

};

#endif // QMPMAINWINDOW_HPP
