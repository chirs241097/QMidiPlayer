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
	ui->setupUi(this);
	connect(this,SIGNAL(dialogClosing()),parent,SLOT(dialogClosed()));
	settings=new QSettings(QDir::homePath()+QString("/.config/qmprc"),QSettings::IniFormat);
	settingsInit();outwidget=ui->cbOutputDevice;
}

qmpSettingsWindow::~qmpSettingsWindow()
{
	delete settings;
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
	{
		ui->lwSoundfont->addItem(new QListWidgetItem(sl.at(i)));
	}
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
