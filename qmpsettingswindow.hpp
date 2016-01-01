#ifndef QMPSETTINGSWINDOW_H
#define QMPSETTINGSWINDOW_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
	class qmpSettingsWindow;
}

class qmpSettingsWindow:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpSettingsWindow(QWidget *parent=0);
		~qmpSettingsWindow();
		void closeEvent(QCloseEvent *event);
	signals:
		void dialogClosing();

		private slots:
		void on_buttonBox_accepted();

		void on_buttonBox_rejected();

	private:
		Ui::qmpSettingsWindow *ui;
};

#endif // QMPSETTINGSWINDOW_H
