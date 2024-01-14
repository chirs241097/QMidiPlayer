#ifndef QMPINFOWINDOW_HPP
#define QMPINFOWINDOW_HPP

#include <QDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QHideEvent>
#include "qmpcorepublic.hpp"

namespace Ui
{
class qmpInfoWindow;
}

class QClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QClickableLabel(QWidget *parent = nullptr) : QLabel(parent) {}
protected:
    void mousePressEvent(QMouseEvent *e)
    {
        QLabel::mousePressEvent(e);
        if (e->buttons() & Qt::LeftButton)
            QApplication::clipboard()->setText(text());
    }
};

class qmpInfoWindow;
class qmpInfoFunc : public qmpFuncBaseIntf
{
private:
    qmpInfoWindow *p;
public:
    qmpInfoFunc(qmpInfoWindow *par);
    void show();
    void close();
};

class qmpInfoWindow : public QDialog
{
    Q_OBJECT

public:
    explicit qmpInfoWindow(QWidget *parent = nullptr);
    ~qmpInfoWindow();
    void closeEvent(QCloseEvent *e);
    void hideEvent(QHideEvent *e);
public slots:
    void updateInfo();

private:
    Ui::qmpInfoWindow *ui;
    qmpInfoFunc *infof;
};

#endif // QMPINFOWINDOW_HPP
