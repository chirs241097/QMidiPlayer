#ifndef QMPPLISTWINDOW_H
#define QMPPLISTWINDOW_H

#include <QWidget>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMoveEvent>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QListWidgetItem>
#include "../include/qmpcorepublic.hpp"

namespace Ui {
	class qmpPlistWindow;
}

class qmpPlistWindow;
class qmpSettings;

class qmpPlistFunc:public qmpFuncBaseIntf
{
	private:
		qmpPlistWindow* p;
	public:
		qmpPlistFunc(qmpPlistWindow *par);
		void show();
		void close();
};

class qmpPlistWindow:public QWidget
{
	Q_OBJECT

	public:
		explicit qmpPlistWindow(QWidget *parent=0);
		~qmpPlistWindow();
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void dropEvent(QDropEvent *event);
		void dragEnterEvent(QDragEnterEvent *event);
		int getRepeat();
		QString getFirstItem(bool a=false);
		QString getLastItem(bool a=false);
		QString getNextItem();
		QString getPrevItem();
		QString getSelectedItem();
		void emptyList();
		void insertItem(QString i);
		void insertItems(QStringList il);
	signals:
		void selectionChanging();

	public slots:
		int on_pbAdd_clicked();
	private slots:
		void on_pbAddFolder_clicked();
		void on_pbRemove_clicked();
		void on_pbClear_clicked();
		void on_pbRepeat_clicked();
		void on_pbShuffle_clicked();
		void on_lwFiles_itemDoubleClicked();
		void on_pbSave_clicked();
		void on_pbLoad_clicked();

	private:
		qmpPlistFunc* plistf;
		Ui::qmpPlistWindow *ui;
		int shuffle,repeat;//rep 0=off 1=one 2=all
		qmpSettings* settings;
};

#endif // QMPPLISTWINDOW_H
