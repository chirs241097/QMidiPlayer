#ifndef QMPCHANNELEDITOR_H
#define QMPCHANNELEDITOR_H

#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>

namespace Ui {
	class qmpChannelEditor;
}

class qmpChannelEditor:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpChannelEditor(QWidget *parent=0);
		~qmpChannelEditor();
	protected:
		void showEvent(QShowEvent *e);
		void closeEvent(QCloseEvent *e);
	public slots:
		void setupWindow(int chid=-1);

	private slots:
		void commonPressed();
		void commonReleased();
		void commonChanged();
		void on_pbChLeft_clicked();
		void on_pbChRight_clicked();

	private:
		Ui::qmpChannelEditor *ui;
		int ch,knobpressed;
		void sendCC();
		void connectSlots();
		void disconnectSlots();
};

#endif // QMPCHANNELEDITOR_H
