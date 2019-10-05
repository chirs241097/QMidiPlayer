#ifndef QMPDEVICEPRIORITYDIALOG_HPP
#define QMPDEVICEPRIORITYDIALOG_HPP

#include <QDialog>
#include <QShowEvent>
#include <QStandardItemModel>

namespace Ui {
class qmpDevicePriorityDialog;
}

class qmpDevicePriorityDialog : public QDialog
{
	Q_OBJECT

public:
	explicit qmpDevicePriorityDialog(QWidget *parent=nullptr);
	~qmpDevicePriorityDialog();
	void setupRegisteredDevices();

private slots:
	void on_pbUp_clicked();
	void on_pbDown_clicked();
	void on_buttonBox_accepted();

private:
	Ui::qmpDevicePriorityDialog *ui;
	QStandardItemModel *model;
	QList<QVariant> setdevs;
};

#endif // QMPDEVICEPRIORITYDIALOG_HPP
