#ifndef QMPPRESETSELECT_H
#define QMPPRESETSELECT_H

#include <QDialog>
#include <QShowEvent>

namespace Ui {
	class qmpPresetSelector;
}

class qmpPresetSelector:public QDialog
{
	Q_OBJECT

	public:
		explicit qmpPresetSelector(QWidget *parent = 0);
		~qmpPresetSelector();
		void showEvent(QShowEvent* e);
		void setupWindow(int chid);

	private slots:
		void on_pbCancel_clicked();

		void on_pbOk_clicked();

		void on_lwBankSelect_currentRowChanged();

		void on_lwPresetSelect_itemDoubleClicked();

	private:
		Ui::qmpPresetSelector *ui;
		char presets[129][128][24];
		int ch;
};

#endif // QMPPRESETSELECT_H
