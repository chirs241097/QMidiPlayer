#ifndef QMPHELPWINDOW_H
#define QMPHELPWINDOW_H

#include <QDialog>
#ifndef BUILD_MACHINE
#define BUILD_MACHINE UNKNOWN
#endif

namespace Ui
{
class qmpHelpWindow;
}

class qmpHelpWindow : public QDialog
{
    Q_OBJECT

public:
    explicit qmpHelpWindow(QWidget *parent = nullptr);
    ~qmpHelpWindow();

private slots:
    void on_textBrowser_sourceChanged(const QUrl &src);

private:
    Ui::qmpHelpWindow *ui;
};

#endif // QMPHELPWINDOW_H
