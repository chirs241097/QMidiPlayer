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
		if(c!=0)return;
		this->ui->twProps->edit(ui->twProps->model()->index(r,c));
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
	QWidget *fw=new QWidget;
	QLabel *lb;QPushButton *pb;
	fw->setLayout(new QHBoxLayout);
	fw->layout()->addWidget(lb=new QLabel);
	lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	if(din.length()){lb->setText(din);fw->setProperty("fn",din);}
	fw->layout()->addWidget(pb=new QPushButton);
	pb->setText("...");
	pb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	cbx->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
	cbx->setText("Disconnected");
	ui->twProps->setCellWidget(r,2,fw);
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
	connect(pb,&QPushButton::clicked,this,[this,lb,fw]{
		lb->setText(QFileDialog::getOpenFileUrl(this,"Select Device Initialization File",QUrl()).path());
		fw->setProperty("fn",lb->text());
	});
}

void qmpDevPropDialog::on_buttonBox_accepted()
{
	QSettings *s=qmpSettingsWindow::getSettingsIntf();
	s->beginGroup("DevInit");
	s->remove("");
	for(int i=0;i<ui->twProps->rowCount();++i)
	{
		s->setValue(ui->twProps->item(i,0)->text(),
					ui->twProps->cellWidget(i,2)->property("fn").toString());
	}
	s->endGroup();
	s->sync();
}
