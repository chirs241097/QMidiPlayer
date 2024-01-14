#ifndef QMPEFXWINDOW_HPP
#define QMPEFXWINDOW_HPP

#include <QWidget>
#include <QCloseEvent>
#include <QShowEvent>
#include <QMoveEvent>

#include "qdialskulpturestyle.hpp"
#include "qmpcorepublic.hpp"

namespace Ui
{
class qmpEfxWindow;
}

class qmpEfxWindow;

class qmpEfxFunc : public qmpFuncBaseIntf
{
private:
    qmpEfxWindow *p;
public:
    qmpEfxFunc(qmpEfxWindow *par);
    void show();
    void close();
};

class qmpEfxWindow : public QWidget
{
    Q_OBJECT

public:
    explicit qmpEfxWindow(QWidget *parent = nullptr);
    ~qmpEfxWindow();
    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    void sendEfxChange(void *_fs = nullptr);

private slots:
    void on_dRoom_valueChanged();
    void on_dDamp_valueChanged();
    void on_dWidth_valueChanged();
    void on_dLevelR_valueChanged();
    void on_dFeedBack_valueChanged();
    void on_dRate_valueChanged();
    void on_dDepth_valueChanged();
    void on_dLevelC_valueChanged();
    void on_cbEnabledC_stateChanged();
    void on_cbEnabledR_stateChanged();
    void on_rbSine_toggled();
    void on_rbTriangle_toggled();
    void spinValueChange();

private:
    void dailValueChange();
    Ui::qmpEfxWindow *ui;
    double rr, rd, rw, rl;
    int cfb, ct, initialized;
    double cl, cr, cd;
    QCommonStyle *styl;
    qmpEfxFunc *efxf;
};

#endif // QMPEFXWINDOW_HPP
