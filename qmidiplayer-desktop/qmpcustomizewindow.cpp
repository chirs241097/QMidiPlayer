#include <map>
#include <set>
#include <vector>
#include <QSettings>
#include <QList>
#include <QVariant>
#include "qmpmainwindow.hpp"
#include "qmpcustomizewindow.hpp"
#include "ui_qmpcustomizewindow.h"

qmpCustomizeWindow::qmpCustomizeWindow(QWidget *parent):
    QDialog(parent),
    ui(new Ui::qmpCustomizeWindow)
{
    ui->setupUi(this);
}

qmpCustomizeWindow::~qmpCustomizeWindow()
{
    delete ui;
}

void qmpCustomizeWindow::load(void *data)
{
    ui->lwAvail->clear();
    ui->lwEnabled->clear();
    QList<QVariant> list = static_cast<QVariant *>(data)->toList();
    std::vector<std::string> v;
    for (auto i : list)
        v.push_back(i.toString().toStdString());
    std::map<std::string, qmpFuncPrivate> &m = qmpMainWindow::getInstance()->getFunc();
    std::set<std::string> s;
    for (auto i = v.begin(); i != v.end(); ++i)
    {
        if (m.find(*i) == m.end())
            continue;
        s.insert(*i);
        QListWidgetItem *it = new QListWidgetItem(
            m[*i].icon(),
            QString(m[*i].desc().c_str())
        );
        it->setToolTip(i->c_str());
        ui->lwEnabled->addItem(it);
    }
    for (auto i = m.begin(); i != m.end(); ++i)
    {
        if (s.find(i->first) == s.end())
        {
            QListWidgetItem *it = new QListWidgetItem(
                i->second.icon(),
                QString(i->second.desc().c_str())
            );
            it->setToolTip(i->first.c_str());
            ui->lwAvail->addItem(it);
        }
    }
}

void *qmpCustomizeWindow::save()
{
    QList<QVariant> ret;
    for (int i = 0; i < ui->lwEnabled->count(); ++i)
    {
        ret.push_back(QVariant(ui->lwEnabled->item(i)->toolTip()));
    }
    return new QVariant(ret);
}

void qmpCustomizeWindow::on_tbAdd_clicked()
{
    if (!ui->lwAvail->currentItem())
        return;
    ui->lwEnabled->addItem(ui->lwAvail->takeItem(ui->lwAvail->currentRow()));
    ui->lwAvail->removeItemWidget(ui->lwAvail->currentItem());
}

void qmpCustomizeWindow::on_tbRemove_clicked()
{
    if (!ui->lwEnabled->currentItem())
        return;
    ui->lwAvail->addItem(ui->lwEnabled->takeItem(ui->lwEnabled->currentRow()));
}

void qmpCustomizeWindow::on_buttonBox_accepted()
{
    accept();
    close();
}

void qmpCustomizeWindow::on_buttonBox_rejected()
{
    reject();
    close();
}
