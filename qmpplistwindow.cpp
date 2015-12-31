#include <cstdlib>
#include <ctime>
#include <QFileDialog>
#include "qmpplistwindow.hpp"
#include "ui_qmpplistwindow.h"
#include "qmpmainwindow.hpp"

qmpPlistWindow::qmpPlistWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpPlistWindow)
{
	ui->setupUi(this);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	connect(this,SIGNAL(selectionChanging()),parent,SLOT(selectionChanged()));
	repeat=0;shuffle=0;
}

qmpPlistWindow::~qmpPlistWindow()
{
	delete ui;
}

void qmpPlistWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	emit dialogClosing();
	event->accept();
}

void qmpPlistWindow::on_pbAdd_clicked()
{
	QStringList sl=QFileDialog::getOpenFileNames(this,"Add File","","Midi files (*.mid *.midi)");
	for(int i=0;i<sl.size();++i)
	{
		ui->lwFiles->addItem(new QListWidgetItem(sl.at(i)));
	}
}

void qmpPlistWindow::on_pbAddFolder_clicked()
{
	QFileDialog::getExistingDirectory(this,"Add Folder");
	//...
}

void qmpPlistWindow::on_pbRemove_clicked()
{
	QList<QListWidgetItem*> sl=ui->lwFiles->selectedItems();
	for(int i=0;i<sl.size();++i)
	{
		ui->lwFiles->removeItemWidget(sl.at(i));
		delete sl.at(i);
	}
}

void qmpPlistWindow::on_pbClear_clicked()
{
	while(ui->lwFiles->count()>0)
	{
		QListWidgetItem *d=ui->lwFiles->item(0);
		ui->lwFiles->removeItemWidget(d);
		delete d;
	}
}

void qmpPlistWindow::on_pbRepeat_clicked()
{
	++repeat;repeat%=3;
	switch(repeat)
	{
		case 0:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-non.png"));
			ui->pbRepeat->setText("Repeat Off");
		break;
		case 1:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-one.png"));
			ui->pbRepeat->setText("Repeat One");
		break;
		case 2:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-all.png"));
			ui->pbRepeat->setText("Repeat All");
		break;
	}
}

void qmpPlistWindow::on_pbShuffle_clicked()
{
	shuffle=1-shuffle;
	switch(shuffle)
	{
		case 1:
			ui->pbShuffle->setIcon(QIcon(":/img/shuffle.png"));
			ui->pbShuffle->setText("Shuffle On");
		break;
		case 0:
		default:
			ui->pbShuffle->setIcon(QIcon(":/img/shuffle-off.png"));
			ui->pbShuffle->setText("Shuffle Off");
		break;
	}
}

QString qmpPlistWindow::getFirstItem()
{
	if(ui->lwFiles->count()==0)return QString();
	int id=0;
	if(shuffle)id=rand()%ui->lwFiles->count();
	ui->lwFiles->setCurrentRow(id);
	return ui->lwFiles->item(id)->text();
}
QString qmpPlistWindow::getNextItem()
{
	if(ui->lwFiles->count()==0)return QString();
	if(repeat==1)return ui->lwFiles->item(ui->lwFiles->currentRow())->text();
	int id=ui->lwFiles->currentRow();++id;id%=ui->lwFiles->count();
	if(shuffle)id=rand()%ui->lwFiles->count();
	ui->lwFiles->setCurrentRow(id);
	return ui->lwFiles->item(id)->text();
}
QString qmpPlistWindow::getPrevItem()
{
	if(ui->lwFiles->count()==0)return QString();
	if(repeat==1)return ui->lwFiles->item(ui->lwFiles->currentRow())->text();
	int id=ui->lwFiles->currentRow();--id;
	id<0?id+=ui->lwFiles->count():0;
	if(shuffle)id=rand()%ui->lwFiles->count();
	ui->lwFiles->setCurrentRow(id);
	return ui->lwFiles->item(id)->text();
}
QString qmpPlistWindow::getSelectedItem()
{
	if(ui->lwFiles->count()==0)return QString();
	return ui->lwFiles->item(ui->lwFiles->currentRow())->text();
}
int qmpPlistWindow::getRepeat(){return repeat;}

void qmpPlistWindow::on_lwFiles_itemDoubleClicked()
{
	emit selectionChanging();
}
