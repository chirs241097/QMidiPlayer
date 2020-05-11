#include <set>
#include <string>
#include "qmpdeviceprioritydialog.hpp"
#include "qmpsettingswindow.hpp"
#include "../core/qmpmidiplay.hpp"
#include "ui_qmpdeviceprioritydialog.h"

qmpDevicePriorityDialog::qmpDevicePriorityDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qmpDevicePriorityDialog)
{
    ui->setupUi(this);
    model = new QStandardItemModel(this);
    ui->tvDevices->setModel(model);
    ui->tvDevices->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    ui->tvDevices->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    ui->tvDevices->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    model->setHorizontalHeaderLabels({"E", "Device", "Connected?"});
}

qmpDevicePriorityDialog::~qmpDevicePriorityDialog()
{
    delete ui;
}

void qmpDevicePriorityDialog::setupRegisteredDevices()
{
    std::set<std::string> sset, sconn;
    auto conndevs = CMidiPlayer::getInstance()->getMidiOutDevices();
    for (auto dev : conndevs)
        sconn.insert(dev);
    model->removeRows(0, model->rowCount());
    for (auto dev : setdevs)
    {
        QStandardItem *e = new QStandardItem;
        e->setCheckable(true);
        e->setEditable(false);
        e->setCheckState(Qt::CheckState::Checked);
        QStandardItem *a = new QStandardItem;
        a->setText(sconn.find(dev.toString().toStdString()) != sconn.end() ? "Connected" : "Disconnected");
        a->setEditable(false);
        QStandardItem *devn = new QStandardItem(dev.toString());
        devn->setEditable(false);
        model->appendRow({e, devn, a});
        sset.insert(dev.toString().toStdString());
    }
    for (auto dev : conndevs)
    {
        if (sset.find(dev) != sset.end())
            continue;
        QStandardItem *e = new QStandardItem;
        e->setCheckable(true);
        e->setEditable(false);
        e->setCheckState(Qt::CheckState::Unchecked);
        QStandardItem *a = new QStandardItem;
        a->setText("Connected");
        a->setEditable(false);
        QStandardItem *devn = new QStandardItem(QString::fromStdString(dev));
        devn->setEditable(false);
        model->appendRow({e, devn, a});
    }
}

void qmpDevicePriorityDialog::load(void *data)
{
    setdevs = static_cast<QVariant *>(data)->toList();
    setupRegisteredDevices();
}

void *qmpDevicePriorityDialog::save()
{
    QList<QVariant> ret;
    for (int i = 0; i < model->rowCount(); ++i)
        if (model->item(i, 0)->checkState() == Qt::CheckState::Checked)
            ret.push_back(model->item(i, 1)->text());
    return new QVariant(ret);
}

void qmpDevicePriorityDialog::on_pbUp_clicked()
{
    const QModelIndex &idx = ui->tvDevices->selectionModel()->currentIndex();
    if (idx.isValid() && idx.row() > 0)
    {
        int row = idx.row();
        auto r = model->takeRow(row);
        model->insertRow(row - 1, r);
        ui->tvDevices->clearSelection();
        ui->tvDevices->selectionModel()->setCurrentIndex(model->index(row - 1, idx.column()), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }
}

void qmpDevicePriorityDialog::on_pbDown_clicked()
{
    const QModelIndex &idx = ui->tvDevices->selectionModel()->currentIndex();
    if (idx.isValid() && idx.row() < model->rowCount() - 1)
    {
        int row = idx.row();
        auto r = model->takeRow(row);
        model->insertRow(row + 1, r);
        ui->tvDevices->clearSelection();
        ui->tvDevices->selectionModel()->setCurrentIndex(model->index(row + 1, idx.column()), QItemSelectionModel::Rows | QItemSelectionModel::Select);
    }
}

void qmpDevicePriorityDialog::on_buttonBox_accepted()
{
    accept();
}

void qmpDevicePriorityDialog::on_buttonBox_rejected()
{
    reject();
}
