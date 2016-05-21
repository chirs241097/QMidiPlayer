#include <cstdlib>
#include <ctime>
#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QSettings>
#include <QUrl>
#include <QMimeData>
#include "qmpplistwindow.hpp"
#include "ui_qmpplistwindow.h"
#include "qmpmainwindow.hpp"
#define setButtonHeight(x,h) {x->setMaximumHeight(h*(logicalDpiY()/96.));x->setMinimumHeight(h*(logicalDpiY()/96.));}

qmpPlistWindow::qmpPlistWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpPlistWindow)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
	setButtonHeight(ui->pbAdd,36);setButtonHeight(ui->pbAddFolder,36);
	setButtonHeight(ui->pbClear,36);setButtonHeight(ui->pbLoad,36);
	setButtonHeight(ui->pbRemove,36);setButtonHeight(ui->pbRepeat,36);
	setButtonHeight(ui->pbSave,36);setButtonHeight(ui->pbShuffle,36);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	connect(this,SIGNAL(selectionChanging()),parent,SLOT(selectionChanged()));
	repeat=0;shuffle=0;
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/RestorePlaylist","").toInt())
	{
		QSettings* plist=new QSettings(QDir::homePath()+QString("/.config/qmpplist"),
									   QSettings::IniFormat);
		int fc=plist->value("Playlist/FileCount",0).toInt();
		ui->lwFiles->clear();for(int i=1;i<=fc;++i)
		ui->lwFiles->addItem(plist->value("Playlist/File"+QString("%1").arg(i,5,10,QChar('0')),"").toString());
		repeat=plist->value("Playlist/Repeat",0).toInt();
		shuffle=plist->value("Playlist/Shuffle",0).toInt();
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
		delete plist;
	}
}

qmpPlistWindow::~qmpPlistWindow()
{
	delete ui;
}

void qmpPlistWindow::showEvent(QShowEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/PListWShown",1);
	}
	event->accept();
}

void qmpPlistWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	if(!qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/PListWShown",0);
	}
	if(qmpMainWindow::getInstance()->isFinalizing()&&qmpSettingsWindow::getSettingsIntf()->value("Behavior/RestorePlaylist","").toInt())
	{
		QSettings* plist=new QSettings(QDir::homePath()+QString("/.config/qmpplist"),
									   QSettings::IniFormat);
		plist->setValue("Playlist/FileCount",ui->lwFiles->count());
		for(int i=0;i<ui->lwFiles->count();++i)
		plist->setValue("Playlist/File"+QString("%1").arg(i+1,5,10,QChar('0')),ui->lwFiles->item(i)->text());
		plist->setValue("Playlist/Repeat",repeat);
		plist->setValue("Playlist/Shuffle",shuffle);
		plist->sync();
		delete plist;
	}
	emit dialogClosing();
	event->accept();
}

void qmpPlistWindow::moveEvent(QMoveEvent *event)
{
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
	{
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/PListW",event->pos());
	}
}

void qmpPlistWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> l=event->mimeData()->urls();
	QStringList sl;
	for(int i=0;i<l.size();++i)
		sl.push_back(l.at(i).toLocalFile());
	insertItems(sl);
}
void qmpPlistWindow::dragEnterEvent(QDragEnterEvent *event)
{
	//if(event->mimeData()->hasFormat("application/x-midi"))
	event->acceptProposedAction();
}

void qmpPlistWindow::emptyList()
{
	ui->lwFiles->clear();
}
void qmpPlistWindow::insertItem(QString i)
{
	ui->lwFiles->addItem(new QListWidgetItem(i));
}
void qmpPlistWindow::insertItems(QStringList il)
{
	ui->lwFiles->addItems(il);
}

void qmpPlistWindow::on_pbAdd_clicked()
{
	QStringList sl;
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
		sl=QFileDialog::getOpenFileNames(this,"Add File",qmpSettingsWindow::getSettingsIntf()->value("DialogStatus/FileDialogPath","").toString(),"Midi files (*.mid *.midi)");
	else
		sl=QFileDialog::getOpenFileNames(this,"Add File","","Midi files (*.mid *.midi)");
	if(sl.empty())return;
	for(int i=0;i<sl.size();++i)
	{
		ui->lwFiles->addItem(new QListWidgetItem(sl.at(i)));
	}
	if(qmpSettingsWindow::getSettingsIntf()->value("Behavior/DialogStatus","").toInt())
		qmpSettingsWindow::getSettingsIntf()->setValue("DialogStatus/FileDialogPath",
		QUrl(sl.at(0)).toString(QUrl::RemoveFilename));
}

void qmpPlistWindow::on_pbAddFolder_clicked()
{
	QDirIterator di(QFileDialog::getExistingDirectory(this,"Add Folder"));
	while(di.hasNext())
	{
		QString c=di.next();
		if((c.endsWith(".mid")||c.endsWith(".midi"))&&fluid_is_midifile(c.toStdString().c_str()))
		ui->lwFiles->addItem(new QListWidgetItem(c));
	}
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
			ui->pbRepeat->setText(tr("Repeat Off"));
		break;
		case 1:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-one.png"));
			ui->pbRepeat->setText(tr("Repeat One"));
		break;
		case 2:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-all.png"));
			ui->pbRepeat->setText(tr("Repeat All"));
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
			ui->pbShuffle->setText(tr("Shuffle On"));
		break;
		case 0:
		default:
			ui->pbShuffle->setIcon(QIcon(":/img/shuffle-off.png"));
			ui->pbShuffle->setText(tr("Shuffle Off"));
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

void qmpPlistWindow::on_pbSave_clicked()
{
	QSettings* plist=new QSettings(QFileDialog::getSaveFileName(this,tr("Save playlist"),""),
								   QSettings::IniFormat);
	plist->setValue("Playlist/FileCount",ui->lwFiles->count());
	for(int i=0;i<ui->lwFiles->count();++i)
	plist->setValue("Playlist/File"+QString("%1").arg(i+1,5,10,QChar('0')),ui->lwFiles->item(i)->text());
	plist->setValue("Playlist/Repeat",repeat);
	plist->setValue("Playlist/Shuffle",shuffle);
	plist->sync();
	delete plist;
}

void qmpPlistWindow::on_pbLoad_clicked()
{
	QSettings* plist=new QSettings(QFileDialog::getOpenFileName(this,tr("Load playlist"),""),
								   QSettings::IniFormat);
	int fc=plist->value("Playlist/FileCount",0).toInt();
	if(!fc)return;
	ui->lwFiles->clear();for(int i=1;i<=fc;++i)
	ui->lwFiles->addItem(plist->value("Playlist/File"+QString("%1").arg(i,5,10,QChar('0')),"").toString());
	repeat=plist->value("Playlist/Repeat",0).toInt();
	shuffle=plist->value("Playlist/Shuffle",0).toInt();
	switch(shuffle)
	{
		case 1:
			ui->pbShuffle->setIcon(QIcon(":/img/shuffle.png"));
			ui->pbShuffle->setText(tr("Shuffle On"));
		break;
		case 0:
		default:
			ui->pbShuffle->setIcon(QIcon(":/img/shuffle-off.png"));
			ui->pbShuffle->setText(tr("Shuffle Off"));
		break;
	}
	switch(repeat)
	{
		case 0:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-non.png"));
			ui->pbRepeat->setText(tr("Repeat Off"));
		break;
		case 1:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-one.png"));
			ui->pbRepeat->setText(tr("Repeat One"));
		break;
		case 2:
			ui->pbRepeat->setIcon(QIcon(":/img/repeat-all.png"));
			ui->pbRepeat->setText(tr("Repeat All"));
		break;
	}
	delete plist;
}
