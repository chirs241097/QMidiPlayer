#ifndef QMPCHANNELEDITOR_H
#define QMPCHANNELEDITOR_H

#include <QDialog>
#include <QShowEvent>
#include <QCloseEvent>
#include <QMouseEvent>
#include "qdialskulpturestyle.hpp"

namespace Ui
{
class qmpChannelEditor;
}

class QDial;
class qmpChannelEditor: public QDialog
{
    Q_OBJECT

public:
    explicit qmpChannelEditor(QWidget *parent = nullptr);
    ~qmpChannelEditor();
protected:
    void showEvent(QShowEvent *e);
    void closeEvent(QCloseEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *event);
public slots:
    void setupWindow(int chid = -1);

private slots:
    void commonPressed();
    void commonReleased();
    void commonChanged();
    void on_pbChLeft_clicked();
    void on_pbChRight_clicked();

private:
    Ui::qmpChannelEditor *ui;
    int ch, knobpressed;
    void sendCC();
    void connectSlots();
    void disconnectSlots();
    QList<QDial *> dials;
    QMetaObject::Connection updconn;
    QCommonStyle *styl;
};

#endif // QMPCHANNELEDITOR_H
