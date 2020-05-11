#ifndef QMPDEVICEPRIORITYDIALOG_HPP
#define QMPDEVICEPRIORITYDIALOG_HPP

#include <QDialog>
#include <QShowEvent>
#include <QStandardItemModel>

namespace Ui
{
class qmpDevicePriorityDialog;
}

class qmpDevicePriorityDialog : public QDialog
{
    Q_OBJECT

public:
    explicit qmpDevicePriorityDialog(QWidget *parent = nullptr);
    ~qmpDevicePriorityDialog();
    void load(void *data);
    void *save();

private slots:
    void on_pbUp_clicked();
    void on_pbDown_clicked();
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::qmpDevicePriorityDialog *ui;
    QStandardItemModel *model;
    QList<QVariant> setdevs;
    void setupRegisteredDevices();
};

#endif // QMPDEVICEPRIORITYDIALOG_HPP
