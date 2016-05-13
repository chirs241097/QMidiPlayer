#include <QLineEdit>
#include <QFileDialog>
#include <QDir>
#include "qmpsettingswindow.hpp"
#include "ui_qmpsettingswindow.h"
#include "qmpmainwindow.hpp"

QSettings* qmpSettingsWindow::settings=NULL;
QComboBox* qmpSettingsWindow::outwidget=NULL;

void qmpFluidForEachOpt(void* data,char* /*name*/,char* option)
{
	QComboBox *pcb=(QComboBox*)data;
	pcb->addItem(option);
}

qmpSettingsWindow::qmpSettingsWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpSettingsWindow)
{
	ui->setupUi(this);customOptions.clear();customOptPages.clear();
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	settings=new QSettings(QDir::homePath()+QString("/.config/qmprc"),QSettings::IniFormat);
	settingsInit();outwidget=ui->cbOutputDevice;
}

qmpSettingsWindow::~qmpSettingsWindow()
{
	delete settings;settings=NULL;
	delete ui;
}

void qmpSettingsWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	settings->sync();
	emit dialogClosing();
	event->accept();
}

QListWidget* qmpSettingsWindow::getSFWidget(){return ui->lwSoundfont;}
QComboBox* qmpSettingsWindow::getDefaultOutWidget(){return outwidget;}

void qmpSettingsWindow::on_buttonBox_accepted()
{
	settingsUpdate();
	emit dialogClosing();
}

void qmpSettingsWindow::on_buttonBox_rejected()
{
	settingsInit();
	emit dialogClosing();
}

void qmpSettingsWindow::settingsInit()
{
	fluid_settings_t *fsettings=new_fluid_settings();

	settings->setValue("Midi/DefaultOutput",settings->value("Midi/DefaultOutput","Internal FluidSynth"));
	//this item is still a stub...

	settings->setValue("Midi/DisableMapping",settings->value("Midi/DisableMapping",0));
	ui->cbDisableMapping->setChecked(settings->value("Midi/DisableMapping",0).toInt());

	settings->setValue("Midi/SendSysEx",settings->value("Midi/SendSysEx",1));
	ui->cbSendSysx->setChecked(settings->value("Midi/SendSysEx",1).toInt());

	settings->setValue("Midi/WaitVoice",settings->value("Midi/WaitVoice",1));
	ui->cbWaitVoice->setChecked(settings->value("Midi/WaitVoice",1).toInt());

	int selected=-1;
	for(int i=0;i<ui->cbEncoding->count();++i)
	if(ui->cbEncoding->itemText(i)==settings->value("Midi/TextEncoding","Unicode").toString())
	{selected=i;break;}
	if(~selected)ui->cbEncoding->setCurrentIndex(selected);
	settings->setValue("Midi/TextEncoding",ui->cbEncoding->currentText());

	fluid_settings_foreach_option(fsettings,"audio.driver",(void*)ui->cbAudioDrv,qmpFluidForEachOpt);
	selected=-1;
	for(int i=0;i<ui->cbAudioDrv->count();++i)
	if(ui->cbAudioDrv->itemText(i)==settings->value("Audio/Driver","pulseaudio").toString())
	{selected=i;break;}
	if(~selected)ui->cbAudioDrv->setCurrentIndex(selected);
	settings->setValue("Audio/Driver",ui->cbAudioDrv->currentText());

#ifdef _WIN32
#define DefBufSize 256
#else
#define DefBufSize 128
#endif
	selected=-1;
	for(int i=0;i<ui->cbBufSize->count();++i)
	if(ui->cbBufSize->itemText(i).toInt()==settings->value("Audio/BufSize",DefBufSize).toInt())
	{selected=i;break;}
	if(~selected)ui->cbBufSize->setCurrentIndex(selected);
	else if(settings->value("Audio/BufSize",DefBufSize).toInt()>=64&&settings->value("Audio/BufSize",DefBufSize).toInt()<=8192)
		ui->cbBufSize->setCurrentText(settings->value("Audio/BufSize",DefBufSize).toString());
	else ui->cbBufSize->setCurrentText(QString::number(DefBufSize));
	settings->setValue("Audio/BufSize",ui->cbBufSize->currentText().toInt());
#undef DefBufSize

#ifdef _WIN32
#define DefBufCnt 8
#else
#define DefBufCnt 2
#endif
	selected=-1;
	for(int i=0;i<ui->cbBufCnt->count();++i)
	if(ui->cbBufCnt->itemText(i).toInt()==settings->value("Audio/BufCnt",DefBufCnt).toInt())
	{selected=i;break;}
	if(~selected)ui->cbBufCnt->setCurrentIndex(selected);
	else if(settings->value("Audio/BufCnt",DefBufCnt).toInt()>=2&&settings->value("Audio/BufCnt",DefBufCnt).toInt()<=64)
		ui->cbBufCnt->setCurrentText(settings->value("Audio/BufCnt",DefBufCnt).toString());
	else ui->cbBufCnt->setCurrentText(QString::number(DefBufCnt));
	settings->setValue("Audio/BufCnt",ui->cbBufCnt->currentText().toInt());
#undef DefBufCnt

	selected=-1;
	for(int i=0;i<ui->cbFormat->count();++i)
	if(ui->cbFormat->itemText(i)==settings->value("Audio/Format","16bits").toString())
	{selected=i;break;}
	if(~selected)ui->cbFormat->setCurrentIndex(selected);
	settings->setValue("Audio/Format",ui->cbFormat->currentText());

	selected=-1;
	for(int i=0;i<ui->cbFrequency->count();++i)
	if(ui->cbFormat->itemText(i).toInt()==settings->value("Audio/Frequency",48000).toInt())
	{selected=i;break;}
	if(~selected)ui->cbFrequency->setCurrentIndex(selected);
	settings->setValue("Audio/Frequency",ui->cbFrequency->currentText());

	ui->sbPolyphony->setValue(settings->value("Audio/Polyphony",2048).toInt());
	if(ui->sbPolyphony->value()<1||ui->sbPolyphony->value()>65535)ui->sbPolyphony->setValue(2048);
	settings->setValue("Audio/Polyphony",ui->sbPolyphony->value());

	ui->sbCPUCores->setValue(settings->value("Audio/Threads",1).toInt());
	if(ui->sbCPUCores->value()<1||ui->sbCPUCores->value()>256)ui->sbCPUCores->setValue(1);
	settings->setValue("Audio/Threads",ui->sbCPUCores->value());

	settings->setValue("Audio/AutoBS",settings->value("Audio/AutoBS",1));
	ui->cbAutoBS->setChecked(settings->value("Audio/AutoBS",1).toInt());
	ui->lbBSMode->setText(ui->cbAutoBS->isChecked()?"Fallback bank select mode":"Bank select mode");

	selected=-1;
	for(int i=0;i<ui->cbBSMode->count();++i)
	if(ui->cbBSMode->itemText(i)==settings->value("Audio/BankSelect","GS").toString())
	{selected=i;break;}
	if(~selected)ui->cbBSMode->setCurrentIndex(selected);
	settings->setValue("Audio/BankSelect",ui->cbBSMode->currentText());
	settings->setValue("Audio/Gain",settings->value("Audio/Gain",50));

	int sfc=settings->value("SoundFonts/SFCount",0).toInt();
	ui->lwSoundfont->clear();for(int i=1;i<=sfc;++i)
	ui->lwSoundfont->addItem(settings->value("SoundFonts/SF"+QString::number(i),"").toString());
	settings->setValue("SoundFonts/SFCount",sfc);

	settings->setValue("Behavior/RestorePlaylist",settings->value("Behavior/RestorePlaylist",0));
	ui->cbRestorePlaylist->setChecked(settings->value("Behavior/RestorePlaylist",0).toInt());

	settings->setValue("Behavior/LoadFolder",settings->value("Behavior/LoadFolder",0));
	ui->cbLoadFolder->setChecked(settings->value("Behavior/LoadFolder",0).toInt());

	settings->setValue("Behavior/DialogStatus",settings->value("Behavior/DialogStatus",1));
	ui->cbDialogStatus->setChecked(settings->value("Behavior/DialogStatus",1).toInt());

	settings->setValue("Behavior/SaveEfxParam",settings->value("Behavior/SaveEfxParam",1));
	ui->cbSaveEfxParam->setChecked(settings->value("Behavior/SaveEfxParam",1).toInt());

	settings->setValue("Behavior/SingleInstance",settings->value("Behavior/SingleInstance",0));
	ui->cbPersistentfs->setChecked(settings->value("Behavior/SingleInstance",0).toInt());

	settings->sync();
	delete_fluid_settings(fsettings);
}

void qmpSettingsWindow::settingsUpdate()
{
	settings->setValue("Midi/DefaultOutput",ui->cbOutputDevice->currentText());

	settings->setValue("Midi/DisableMapping",ui->cbDisableMapping->isChecked()?1:0);

	settings->setValue("Midi/SendSysEx",ui->cbSendSysx->isChecked()?1:0);

	settings->setValue("Midi/WaitVoice",ui->cbWaitVoice->isChecked()?1:0);

	settings->setValue("Midi/TextEncoding",ui->cbEncoding->currentText());

	settings->setValue("Audio/Driver",ui->cbAudioDrv->currentText());

	settings->setValue("Audio/BufSize",ui->cbBufSize->currentText().toInt());

	settings->setValue("Audio/BufCnt",ui->cbBufCnt->currentText().toInt());

	settings->setValue("Audio/Format",ui->cbFormat->currentText());

	settings->setValue("Audio/Frequency",ui->cbFrequency->currentText());

	settings->setValue("Audio/Polyphony",ui->sbPolyphony->value());

	settings->setValue("Audio/Threads",ui->sbCPUCores->value());

	settings->setValue("Audio/AutoBS",ui->cbAutoBS->isChecked()?1:0);

	settings->setValue("Audio/BankSelect",ui->cbBSMode->currentText());

	settings->setValue("SoundFonts/SFCount",ui->lwSoundfont->count());
	for(int i=0;i<ui->lwSoundfont->count();++i)
	settings->setValue("SoundFonts/SF"+QString::number(i+1),ui->lwSoundfont->item(i)->text());

	settings->setValue("Behavior/RestorePlaylist",ui->cbRestorePlaylist->isChecked()?1:0);

	settings->setValue("Behavior/LoadFolder",ui->cbLoadFolder->isChecked()?1:0);

	settings->setValue("Behavior/DialogStatus",ui->cbDialogStatus->isChecked()?1:0);
	if(!ui->cbDialogStatus->isChecked())
	{
		settings->remove("DialogStatus/MainW");
		settings->remove("DialogStatus/PListW");
		settings->remove("DialogStatus/PListWShown");
		settings->remove("DialogStatus/ChnlW");
		settings->remove("DialogStatus/ChnlWShown");
		settings->remove("DialogStatus/EfxW");
		settings->remove("DialogStatus/EfxWShown");
		settings->remove("DialogStatus/FileDialogPath");
	}

	settings->setValue("Behavior/SaveEfxParam",ui->cbSaveEfxParam->isChecked()?1:0);
	if(!ui->cbSaveEfxParam->isChecked())
	{
		settings->remove("Effects/ChorusEnabled");
		settings->remove("Effects/ReverbEnabled");
		settings->remove("Effects/ReverbRoom");
		settings->remove("Effects/ReverbDamp");
		settings->remove("Effects/ReverbWidth");
		settings->remove("Effects/ReverbLevel");

		settings->remove("Effects/ChorusFeedbk");
		settings->remove("Effects/ChorusLevel");
		settings->remove("Effects/ChorusRate");
		settings->remove("Effects/ChorusDepth");
		settings->remove("Effects/ChorusType");
	}

	settings->setValue("Behavior/SingleInstance",ui->cbPersistentfs->isChecked()?1:0);

	for(int i=0;i<ui->twPluginList->rowCount();++i)
		settings->setValue(
		QString("PluginSwitch/")+ui->twPluginList->item(i,1)->text(),
		((QCheckBox*)ui->twPluginList->cellWidget(i,0))->isChecked()?1:0);
	updateCustomOptions();
	settings->sync();
}

void qmpSettingsWindow::on_cbBufSize_currentTextChanged(const QString &s)
{
	if(s.toInt()<64||s.toInt()>8192)ui->cbBufSize->setCurrentIndex(1);
}

void qmpSettingsWindow::on_cbBufCnt_currentTextChanged(const QString &s)
{
	if(s.toInt()<2||s.toInt()>64)ui->cbBufCnt->setCurrentIndex(1);
}

void qmpSettingsWindow::on_pbAdd_clicked()
{
	QStringList sl=QFileDialog::getOpenFileNames(this,"Add File","","SoundFont files (*.sf2)");
	for(int i=0;i<sl.size();++i)
		ui->lwSoundfont->addItem(new QListWidgetItem(sl.at(i)));
}

void qmpSettingsWindow::on_pbRemove_clicked()
{
	QList<QListWidgetItem*> sl=ui->lwSoundfont->selectedItems();
	for(int i=0;i<sl.size();++i)
	{
		ui->lwSoundfont->removeItemWidget(sl.at(i));
		delete sl.at(i);
	}
}

void qmpSettingsWindow::on_pbUp_clicked()
{
	int cid=ui->lwSoundfont->currentRow();
	QListWidgetItem *ci=ui->lwSoundfont->takeItem(cid);
	ui->lwSoundfont->insertItem(cid-1,ci);
	ui->lwSoundfont->setCurrentRow(cid-1);
}

void qmpSettingsWindow::on_pbDown_clicked()
{
	int cid=ui->lwSoundfont->currentRow();
	QListWidgetItem *ci=ui->lwSoundfont->takeItem(cid);
	ui->lwSoundfont->insertItem(cid+1,ci);
	ui->lwSoundfont->setCurrentRow(cid+1);
}

void qmpSettingsWindow::on_cbAutoBS_stateChanged()
{
	ui->lbBSMode->setText(ui->cbAutoBS->isChecked()?"Fallback bank select mode":"Bank select mode");
}

void qmpSettingsWindow::updatePluginList(qmpPluginManager *pmgr)
{
	std::vector<qmpPlugin> *plugins=pmgr->getPlugins();
	for(unsigned i=0;i<plugins->size();++i)
	{
		ui->twPluginList->insertRow(i);
		ui->twPluginList->setCellWidget(i,0,new QCheckBox(""));
		if(settings->value(QString("PluginSwitch/")+QString(plugins->at(i).name.c_str()),0).toInt())
		{((QCheckBox*)ui->twPluginList->cellWidget(i,0))->setChecked(true);plugins->at(i).enabled=true;}
		else
		{((QCheckBox*)ui->twPluginList->cellWidget(i,0))->setChecked(false);plugins->at(i).enabled=false;}
		ui->twPluginList->setItem(i,1,new QTableWidgetItem(plugins->at(i).name.c_str()));
		ui->twPluginList->setItem(i,2,new QTableWidgetItem(plugins->at(i).version.c_str()));
		ui->twPluginList->setItem(i,3,new QTableWidgetItem(plugins->at(i).path.c_str()));
		for(int j=1;j<=3;++j)
		ui->twPluginList->item(i,j)->setFlags(ui->twPluginList->item(i,j)->flags()^Qt::ItemIsEditable);
	}
	ui->twPluginList->setColumnWidth(0,22);
	ui->twPluginList->setColumnWidth(1,192);
	ui->twPluginList->setColumnWidth(2,64);
	ui->twPluginList->setColumnWidth(3,128);
}

void qmpSettingsWindow::updateCustomOptions()
{
	for(auto i=customOptions.begin();i!=customOptions.end();++i)
	switch(i->second.type)
	{
		case 0:
		{
			QSpinBox* sb=(QSpinBox*)i->second.widget;if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),sb->value());
			break;
		}
		case 1:
		{
			QHexSpinBox* sb=(QHexSpinBox*)i->second.widget;if(!i->second.widget)break;
			int v=sb->value();
			settings->setValue(QString(i->first.c_str()),*reinterpret_cast<unsigned int*>(&v));
			break;
		}
		case 2:
		{
			if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),((QCheckBox*)i->second.widget)->isChecked()?1:0);
			break;
		}
		case 3:
		{
			QDoubleSpinBox* sb=(QDoubleSpinBox*)i->second.widget;if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),sb->value());
			break;
		}
		case 4:
		{
			QLineEdit* te=(QLineEdit*)i->second.widget;if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),te->text());
			break;
		}
		case 5:
		{
			QComboBox* cb=(QComboBox*)i->second.widget;if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),cb->currentIndex());
			break;
		}
	}
}

void qmpSettingsWindow::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=0;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QSpinBox* sb=new QSpinBox(page->parentWidget());
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		customOptions[key].widget=sb;
		sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(lb,row,0);
		page->addWidget(sb,row,1);
		sb->setMaximum(max);
		sb->setMinimum(min);
		sb->setValue(settings->value(QString(key.c_str()),defaultval).toInt());
	}
}
int qmpSettingsWindow::getOptionInt(std::string key)
{
	return settings->value(QString(key.c_str()),customOptions[key].defaultval).toInt();
}
void qmpSettingsWindow::setOptionInt(std::string key,int val)
{
	settings->setValue(QString(key.c_str()),val);
	if(customOptions[key].widget)
	((QSpinBox*)customOptions[key].widget)->setValue(val);
}

void qmpSettingsWindow::registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=1;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QHexSpinBox* sb=new QHexSpinBox(page->parentWidget());
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		customOptions[key].widget=sb;
		sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(lb,row,0);
		page->addWidget(sb,row,1);
		//sb->setMaximum(i(max));sb->setMinimum(i(min));
		sb->setValue(settings->value(QString(key.c_str()),defaultval).toUInt());
	}
}
unsigned qmpSettingsWindow::getOptionUint(std::string key)
{
	return settings->value(QString(key.c_str()),customOptions[key].defaultval).toUInt();
}
void qmpSettingsWindow::setOptionUint(std::string key,unsigned val)
{
	settings->setValue(QString(key.c_str()),val);
	if(customOptions[key].widget)
	((QHexSpinBox*)customOptions[key].widget)->setValue(val);
}

void qmpSettingsWindow::registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].type=2;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QCheckBox* cb=new QCheckBox(desc.c_str(),page->parentWidget());
		customOptions[key].widget=cb;
		cb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(cb,row,0,1,2);
		cb->setChecked(settings->value(QString(key.c_str()),(int)defaultval).toInt());
	}
}
bool qmpSettingsWindow::getOptionBool(std::string key)
{
	return settings->value(QString(key.c_str()),(int)customOptions[key].defaultval.toBool()).toInt();
}
void qmpSettingsWindow::setOptionBool(std::string key,bool val)
{
	settings->setValue(QString(key.c_str()),val?1:0);
	if(customOptions[key].widget)
	((QCheckBox*)customOptions[key].widget)->setChecked(val);
}

void qmpSettingsWindow::registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=3;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QDoubleSpinBox* sb=new QDoubleSpinBox(page->parentWidget());
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		customOptions[key].widget=sb;
		sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(lb,row,0);
		page->addWidget(sb,row,1);
		sb->setMaximum(max);
		sb->setMinimum(min);
		sb->setValue(settings->value(QString(key.c_str()),defaultval).toDouble());
	}
}
double qmpSettingsWindow::getOptionDouble(std::string key)
{
	return settings->value(QString(key.c_str()),customOptions[key].defaultval).toDouble();
}
void qmpSettingsWindow::setOptionDouble(std::string key,double val)
{
	settings->setValue(QString(key.c_str()),val);
	if(customOptions[key].widget)
	((QDoubleSpinBox*)customOptions[key].widget)->setValue(val);
}

void qmpSettingsWindow::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=QString(defaultval.c_str());
	customOptions[key].type=4;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QLineEdit* te=new QLineEdit(page->parentWidget());
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		customOptions[key].widget=te;
		te->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(lb,row,0);
		page->addWidget(te,row,1);
		te->setText(settings->value(QString(key.c_str()),defaultval.c_str()).toString());
	}
}
std::string qmpSettingsWindow::getOptionString(std::string key)
{
	return settings->value(QString(key.c_str()),customOptions[key].defaultval).toString().toStdString();
}
void qmpSettingsWindow::setOptionString(std::string key,std::string val)
{
	settings->setValue(QString(key.c_str()),QString(val.c_str()));
	if(customOptions[key].widget)
	((QLineEdit*)customOptions[key].widget)->setText(val.c_str());
}

void qmpSettingsWindow::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> options,int defaultval)
{
	customOptions[key].widget=NULL;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].type=5;
	if(desc.length())
	{
		QGridLayout* page=NULL;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		QComboBox* sb=new QComboBox(page->parentWidget());
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		customOptions[key].widget=sb;
		for(unsigned i=0;i<options.size();++i)sb->addItem(options[i].c_str());
		sb->setCurrentIndex(settings->value(QString(key.c_str()),defaultval).toInt());
		sb->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		int row=page->rowCount();
		page->addWidget(lb,row,0);
		page->addWidget(sb,row,1);

	}
}
int qmpSettingsWindow::getOptionEnumInt(std::string key)
{
	return settings->value(QString(key.c_str()),customOptions[key].defaultval).toInt();
}
void qmpSettingsWindow::setOptionEnumInt(std::string key,int val)
{
	settings->setValue(QString(key.c_str()),val);
	if(customOptions[key].widget)
	((QComboBox*)customOptions[key].widget)->setCurrentIndex(val);
}
