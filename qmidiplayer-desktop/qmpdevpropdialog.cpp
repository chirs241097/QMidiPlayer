#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QCheckBox>
#include "qmpdevpropdialog.hpp"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"
#include "ui_qmpdevpropdialog.h"

qmpDevPropDialog::qmpDevPropDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpDevPropDialog)
{
	ui->setupUi(this);
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
	QComboBox *cb;
	ui->twProps->setCellWidget(r,0,cb=new QComboBox);
	for(auto&s:qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices())
	if(s!="Internal FluidSynth")cb->addItem(QString::fromStdString(s));
	QWidget *fw=new QWidget;
	QLabel *lb;QPushButton *pb;
	fw->setLayout(new QHBoxLayout);
	fw->layout()->addWidget(lb=new QLabel);
	lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	if(din.length()){lb->setText(din);fw->setProperty("fn",din);}
	fw->layout()->addWidget(pb=new QPushButton);
	pb->setText("...");
	pb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
	QCheckBox *cbx;
	ui->twProps->setCellWidget(r,1,cbx=new QCheckBox);
	cbx->setEnabled(false);
	cbx->setChecked(false);
	ui->twProps->setCellWidget(r,2,fw);
	ui->twProps->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	connect(cb,&QComboBox::currentTextChanged,this,[this,r](const QString&s){
		bool conn=false;
		for(auto&ds:qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices())
			if(s==QString::fromStdString(ds)){conn=true;break;}
		((QCheckBox*)ui->twProps->cellWidget(r,1))->setChecked(conn);
	});
	if(dn.length())cb->setCurrentText(dn);
	emit cb->currentTextChanged(cb->currentText());
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
		s->setValue(((QComboBox*)ui->twProps->cellWidget(i,0))->currentText(),
					ui->twProps->cellWidget(i,2)->property("fn").toString());
	}
	s->endGroup();
	s->sync();
}
