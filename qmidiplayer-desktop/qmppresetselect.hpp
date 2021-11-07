#ifndef QMPPRESETSELECT_H
#define QMPPRESETSELECT_H

#include <QDialog>
#include <QShowEvent>

namespace Ui
{
class qmpPresetSelector;
}

class qmpPresetSelector: public QDialog
{
    Q_OBJECT

public:
    explicit qmpPresetSelector(QWidget *parent = 0);
    ~qmpPresetSelector();
    void setupWindow(int chid);

private slots:

    void on_lwBankSelect_currentRowChanged();

    void on_lwPresetSelect_itemDoubleClicked();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

private:
    Ui::qmpPresetSelector *ui;
    void on_pbCancel_clicked();
    void on_pbOk_clicked();
    int ch;
};

#endif // QMPPRESETSELECT_H
