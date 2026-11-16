#ifndef QMPKEYBOARDWINDOW_HPP
#define QMPKEYBOARDWINDOW_HPP

#include "qmpcorepublic.hpp"

#include <QWidget>

class QLabel;
class QMdiSubWindow;
class qmpPianoWidget;
class qmpKeyboardWindow: public QWidget
{
    Q_OBJECT
private:
    qmpPianoWidget *pw[16];
    QLabel *lb[16];
    qmpPluginAPI *api;
    int eh;
    QMdiSubWindow* subw;
public:
    qmpKeyboardWindow(qmpPluginAPI *_api, QWidget *parent);
    ~qmpKeyboardWindow();
    void resetAll();
protected:
    void closeEvent(QCloseEvent *event);
Q_SIGNALS:
    void keystateupdated(int ch, int key, bool state);
public Q_SLOTS:
    void onkeystatesupdate(int ch, int key, bool state);

    friend class qmpSimpleVisualization;
};

#endif
