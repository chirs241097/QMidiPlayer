#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QTableWidgetItem>
#include "qmpdevpropdialog.hpp"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"
#include "qmpchannelswindow.hpp"
#include "ui_qmpdevpropdialog.h"

qmpDevPropDialog::qmpDevPropDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpDevPropDialog)
{
	ui->setupUi(this);
	ui->twProps->setItemDelegateForColumn(0,new qmpDeviceItemDelegate(true,ui->twProps));
	ui->twProps->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
	connect(ui->twProps,&QTableWidget::cellClicked,[this](int r,int c){
		if(c==0)
			this->ui->twProps->edit(ui->twProps->model()->index(r,c));
		if(c==3)
		{
			QString p=QFileDialog::getOpenFileUrl(this,tr("Select Device Initialization File"),QUrl()).toLocalFile();
			if(p.length())this->ui->twProps->item(r,2)->setText(p);
		}
	});
	connect(ui->twProps,&QTableWidget::cellChanged,this,[this](int r,int c){
		if(c!=0)return;
		QString connst(tr("Disconnected"));
		for(auto&ds:qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices())
			if(ui->twProps->item(r,c)->text()==QString::fromStdString(ds))
			{
				connst=tr("Connected");
				break;
			}
		ui->twProps->item(r,1)->setText(connst);
	});
}

qmpDevPropDialog::~qmpDevPropDialog()
{
	delete ui;
}

void qmpDevPropDialog::load(void *data)
{
	QList<QVariant> lst=static_cast<QVariant*>(data)->toList();
	ui->twProps->clearContents();
	ui->twProps->setRowCount(0);
	for(auto&i:lst)
	{
		QPair<QString,QString> p=i.value<QPair<QString,QString>>();
		setupRow(p.first,p.second);
	}
}

void *qmpDevPropDialog::save()
{
	QList<QVariant> ret;
	for(int i=0;i<ui->twProps->rowCount();++i)
	{
		QPair<QString,QString> p
		{
			ui->twProps->item(i,0)->text(),
			ui->twProps->item(i,2)->text()
		};
		ret.push_back(QVariant::fromValue(p));
	}
	return new QVariant(ret);
}

void qmpDevPropDialog::on_pbAdd_clicked()
{
	setupRow();
}

void qmpDevPropDialog::on_pbRemove_clicked()
{
	ui->twProps->removeRow(ui->twProps->currentRow());
}

void qmpDevPropDialog::setupRow(const QString&dn,const QString&din)
{
	int r;
	ui->twProps->insertRow(r=ui->twProps->rowCount());
	ui->twProps->setRowHeight(r,32);
	QTableWidgetItem *cbx=new QTableWidgetItem;
	ui->twProps->setItem(r,1,cbx);
	QTableWidgetItem *cb;
	ui->twProps->setItem(r,0,cb=new QTableWidgetItem);
	ui->twProps->setItem(r,2,new QTableWidgetItem);
	ui->twProps->setItem(r,3,new QTableWidgetItem("..."));
	if(din.length())ui->twProps->item(r,2)->setText(din);
	cbx->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
	cbx->setText(tr("Disconnected"));
	ui->twProps->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	if(dn.length())
	{
		cb->setText(dn);
		cb->setFlags(cb->flags()|Qt::ItemFlag::ItemIsEditable);
		std::vector<std::string> dsv=CMidiPlayer::getInstance()->getMidiOutDevices();
		if(std::find(dsv.begin(),dsv.end(),dn.toStdString())==dsv.end())
			cb->setFlags(cb->flags()&(~Qt::ItemFlag::ItemIsEnabled));
		else cb->setFlags(cb->flags()|Qt::ItemFlag::ItemIsEnabled);
	}
}

void qmpDevPropDialog::on_buttonBox_accepted()
{
	accept();
}

void qmpDevPropDialog::on_buttonBox_rejected()
{
	reject();
}
