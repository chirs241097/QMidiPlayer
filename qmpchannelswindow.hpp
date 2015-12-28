#ifndef QMPCHANNELSWINDOW_H
#define QMPCHANNELSWINDOW_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
	class qmpchannelswindow;
}

class qmpchannelswindow : public QDialog
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
	private slots:
		void on_pbUnmute_clicked();

		void on_pbUnsolo_clicked();

		private:
		Ui::qmpchannelswindow *ui;
};

#endif // QMPCHANNELSWINDOW_H
