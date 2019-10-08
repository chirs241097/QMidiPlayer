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
			QString p=QFileDialog::getOpenFileUrl(this,"Select Device Initialization File",QUrl()).path();
			if(p.length())this->ui->twProps->item(r,2)->setText(p);
		}
	});
	connect(ui->twProps,&QTableWidget::cellChanged,this,[this](int r,int c){
		if(c!=0)return;
		QString connst("Disconnected");
		for(auto&ds:qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices())
			if(ui->twProps->item(r,c)->text()==QString::fromStdString(ds)){connst="Connected";break;}
		ui->twProps->item(r,1)->setText(connst);
	});
}

qmpDevPropDialog::~qmpDevPropDialog()
{
	delete ui;
}

void qmpDevPropDialog::launch()
{
	QSettings *s=qmpSettingsWindow::getSettingsIntf();
	ui->twProps->clearContents();
	ui->twProps->setRowCount(0);
	s->beginGroup("DevInit");
	for(auto&k:s->allKeys())
		setupRow(k,s->value(k).toString());
	s->endGroup();
	show();
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
	cbx->setText("Disconnected");
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
	QSettings *s=qmpSettingsWindow::getSettingsIntf();
	s->beginGroup("DevInit");
	s->remove("");
	for(int i=0;i<ui->twProps->rowCount();++i)
	{
		s->setValue(ui->twProps->item(i,0)->text(),
					ui->twProps->item(i,2)->text());
	}
	s->endGroup();
	s->sync();
}
