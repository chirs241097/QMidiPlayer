#ifndef QMPPLISTWINDOW_H
#define QMPPLISTWINDOW_H

#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QListWidgetItem>

namespace Ui {
	class qmpPlistWindow;
}

class qmpPlistWindow : public QDialog
{
	Q_OBJECT

	public:
		explicit qmpPlistWindow(QWidget *parent=0);
		~qmpPlistWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void moveEvent(QMoveEvent *event);
		int getRepeat();
		QString getFirstItem();
		QString getNextItem();
		QString getPrevItem();
		QString getSelectedItem();
		//void loadPList(const char*);
		//void savePList(const char*);
	signals:
		void dialogClosing();
		void selectionChanging();

	private slots:
		void on_pbAdd_clicked();

		void on_pbAddFolder_clicked();

		void on_pbRemove_clicked();

		void on_pbClear_clicked();

		void on_pbRepeat_clicked();

		void on_pbShuffle_clicked();

		void on_lwFiles_itemDoubleClicked();

		void on_pbSave_clicked();

		void on_pbLoad_clicked();

		private:
		Ui::qmpPlistWindow *ui;
		int shuffle,repeat;//rep 0=off 1=one 2=all
};

#endif // QMPPLISTWINDOW_H