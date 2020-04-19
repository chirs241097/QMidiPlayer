#ifndef QMPDEVPROPDIALOG_HPP
#define QMPDEVPROPDIALOG_HPP

#include <QDialog>

namespace Ui {
class qmpDevPropDialog;
}

class qmpDevPropDialog : public QDialog
{
	Q_OBJECT

	public:
		explicit qmpDevPropDialog(QWidget *parent = nullptr);
		~qmpDevPropDialog();
		void load(void* data);
		void* save();

	private slots:
		void on_pbAdd_clicked();

		void on_pbRemove_clicked();

		void on_buttonBox_accepted();

		void on_buttonBox_rejected();

	private:
		Ui::qmpDevPropDialog *ui;
		void setupRow(const QString &dn="",const QString &din="");
};

#endif // QMPDEVPROPDIALOG_HPP
