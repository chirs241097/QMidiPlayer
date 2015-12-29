#ifndef QMPPRESETSELECT_H
#define QMPPRESETSELECT_H

#include <QDialog>
#include <QShowEvent>

namespace Ui {
	class qmppresetselect;
}

class qmppresetselect:public QDialog
{
	Q_OBJECT

	public:
		explicit qmppresetselect(QWidget *parent = 0);
		~qmppresetselect();
		void showEvent(QShowEvent* e);
		void setupWindow(int chid);

		private slots:
		void on_pbCancel_clicked();

		void on_pbOk_clicked();

		void on_lwBankSelect_currentRowChanged();

		void on_lwPresetSelect_itemDoubleClicked();

		private:
		Ui::qmppresetselect *ui;
		char presets[129][128][24];
		int ch;
};

#endif // QMPPRESETSELECT_H
