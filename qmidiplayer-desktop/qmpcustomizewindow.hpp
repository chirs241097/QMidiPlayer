#ifndef QMPCUSTOMIZEWINDOW_HPP
#define QMPCUSTOMIZEWINDOW_HPP

#include <QDialog>

namespace Ui
{
class qmpCustomizeWindow;
}

class qmpCustomizeWindow: public QDialog
{
    Q_OBJECT

public:
    explicit qmpCustomizeWindow(QWidget *parent = nullptr);
    ~qmpCustomizeWindow();
    void load(void *data);
    void *save();

private slots:
    void on_tbAdd_clicked();
    void on_tbRemove_clicked();
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::qmpCustomizeWindow *ui;
};

#endif // QMPCUSTOMIZEWINDOW_HPP
