#include <set>
#include <QLineEdit>
#include <QToolButton>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>
#include <QDesktopWidget>
#include <QStandardPaths>
#include "qmpsettingswindow.hpp"
#include "qmpdeviceprioritydialog.hpp"
#include "ui_qmpsettingswindow.h"
#include "qmpmainwindow.hpp"

QSettings* qmpSettingsWindow::settings=nullptr;

void qmpFluidForEachOpt(void* data,const char*,const char* option)
{
	QComboBox *pcb=(QComboBox*)data;
	pcb->addItem(option);
}

qmpSettingsWindow::qmpSettingsWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpSettingsWindow)
{
	ui->setupUi(this);customOptions.clear();customOptPages.clear();
	connect(this,&qmpSettingsWindow::dialogClosing,(qmpMainWindow*)parent,&qmpMainWindow::dialogClosed);
	settings=new QSettings(QStandardPaths::writableLocation(QStandardPaths::StandardLocation::ConfigLocation)+QString("/qmprc"),QSettings::IniFormat);
	settingsInit();
	ui->pbAdd->setIcon(QIcon(getThemedIcon(":/img/add.svg")));
	ui->pbRemove->setIcon(QIcon(getThemedIcon(":/img/remove.svg")));
	ui->pbDown->setIcon(QIcon(getThemedIcon(":/img/down.svg")));
	ui->pbUp->setIcon(QIcon(getThemedIcon(":/img/up.svg")));
	cw=new qmpCustomizeWindow(this);
	dps=new qmpDevPropDialog(this);
	devpriod=new qmpDevicePriorityDialog(this);
}

qmpSettingsWindow::~qmpSettingsWindow()
{
	delete cw;delete dps;
	delete settings;settings=nullptr;
	delete ui;
}

void qmpSettingsWindow::closeEvent(QCloseEvent *event)
{
	setVisible(false);
	settings->sync();
	emit dialogClosing();
	event->accept();
}
void qmpSettingsWindow::hideEvent(QHideEvent *event)
{
	emit dialogClosing();
	event->accept();
}

void qmpSettingsWindow::on_buttonBox_accepted()
{
	settingsUpdate();
	qmpMainWindow::getInstance()->setupWidget();
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
	if(ui->cbBSMode->itemText(i)==settings->value("Audio/BankSelect","CC#0").toString())
	{selected=i;break;}
	if(~selected)ui->cbBSMode->setCurrentIndex(selected);
	settings->setValue("Audio/BankSelect",ui->cbBSMode->currentText());
	settings->setValue("Audio/Gain",settings->value("Audio/Gain",50));

	QList<QVariant> sflist=settings->value("Audio/SoundFonts",QList<QVariant>{}).toList();
	ui->twSoundfont->clear();
	for(int i=0;i<sflist.size();++i)
	{
		ui->twSoundfont->insertRow(i);
		QTableWidgetItem *sfn,*sfe;
		QString sf=sflist[i].toString();
		bool enabled=!sf.startsWith('#');
		if(!enabled)sf=sf.mid(1);
		ui->twSoundfont->setItem(i,1,sfn=new QTableWidgetItem(sf));
		ui->twSoundfont->setItem(i,0,sfe=new QTableWidgetItem());
		sfn->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
		sfe->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
		sfe->setCheckState(enabled?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
	}
	ui->twSoundfont->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	QStringList qs{"E","Path"};
	ui->twSoundfont->setHorizontalHeaderLabels(qs);

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

	settings->setValue("Behavior/ShowButtonLabel",settings->value("Behavior/ShowButtonLabel",0));
	ui->cbShowLabel->setChecked(settings->value("Behavior/ShowButtonLabel",0).toInt());

	settings->setValue("Behavior/IconTheme",settings->value("Behavior/IconTheme",0));
	ui->cbIconTheme->setCurrentIndex(settings->value("Behavior/IconTheme",0).toInt());

	settings->sync();
	delete_fluid_settings(fsettings);
}

void qmpSettingsWindow::settingsUpdate()
{
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

	QList<QVariant> sflist;
	for(int i=0;i<ui->twSoundfont->rowCount();++i)
	{
		QString sfs=ui->twSoundfont->item(i,1)->text();
		if(ui->twSoundfont->item(i,0)->checkState()==Qt::CheckState::Unchecked)sfs="#"+sfs;
		sflist.push_back(sfs);
	}
	settings->setValue("Audio/SoundFonts",sflist);

	settings->setValue("Behavior/RestorePlaylist",ui->cbRestorePlaylist->isChecked()?1:0);

	settings->setValue("Behavior/LoadFolder",ui->cbLoadFolder->isChecked()?1:0);

	settings->setValue("Behavior/DialogStatus",ui->cbDialogStatus->isChecked()?1:0);

	settings->setValue("Behavior/SingleInstance",ui->cbPersistentfs->isChecked()?1:0);

	settings->setValue("Behavior/ShowButtonLabel",ui->cbShowLabel->isChecked()?1:0);

	settings->setValue("Behavior/IconTheme",ui->cbIconTheme->currentIndex());

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

	for(int i=0;i<ui->twPluginList->rowCount();++i)
		settings->setValue(
		QString("PluginSwitch/")+ui->twPluginList->item(i,1)->text(),
		ui->twPluginList->item(i,0)->checkState()==Qt::CheckState::Checked?1:0);
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
	for(int i=0;i<sl.size();++i){
		ui->twSoundfont->insertRow(ui->twSoundfont->rowCount());
		QTableWidgetItem *sfn,*sfe;
		ui->twSoundfont->setItem(ui->twSoundfont->rowCount()-1,1,sfn=new QTableWidgetItem(sl[i]));
		ui->twSoundfont->setItem(ui->twSoundfont->rowCount()-1,0,sfe=new QTableWidgetItem());
		sfn->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable);
		sfe->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
	}
}

void qmpSettingsWindow::on_pbRemove_clicked()
{
	QList<QTableWidgetItem*> sl=ui->twSoundfont->selectedItems();
	for(int i=0;i<sl.size();++i)
	{
		ui->twSoundfont->removeRow(ui->twSoundfont->row(sl[i]));
	}
}

void qmpSettingsWindow::on_pbUp_clicked()
{
	int cid=ui->twSoundfont->currentRow();if(!cid)return;
	QTableWidgetItem *ci=ui->twSoundfont->takeItem(cid,1);
	QTableWidgetItem *ce=ui->twSoundfont->takeItem(cid,0);
	ui->twSoundfont->removeRow(cid);
	ui->twSoundfont->insertRow(cid-1);
	ui->twSoundfont->setItem(cid-1,0,ce);
	ui->twSoundfont->setItem(cid-1,1,ci);
	ui->twSoundfont->setCurrentCell(cid-1,1);
}

void qmpSettingsWindow::on_pbDown_clicked()
{
	int cid=ui->twSoundfont->currentRow();if(cid==ui->twSoundfont->rowCount()-1)return;
	QTableWidgetItem *ci=ui->twSoundfont->takeItem(cid,1);
	QTableWidgetItem *ce=ui->twSoundfont->takeItem(cid,0);
	ui->twSoundfont->removeRow(cid);
	ui->twSoundfont->insertRow(cid+1);
	ui->twSoundfont->setItem(cid+1,0,ce);
	ui->twSoundfont->setItem(cid+1,1,ci);
	ui->twSoundfont->setCurrentCell(cid+1,1);
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
		QTableWidgetItem *icb;
		ui->twPluginList->setItem(i,0,icb=new QTableWidgetItem());
		bool enabled=settings->value(QString("PluginSwitch/")+QString(plugins->at(i).name.c_str()),1).toInt();
		icb->setCheckState(enabled?Qt::CheckState::Checked:Qt::CheckState::Unchecked);
		icb->setFlags(Qt::ItemFlag::ItemIsEnabled|Qt::ItemFlag::ItemIsSelectable|Qt::ItemFlag::ItemIsUserCheckable);
		plugins->at(i).enabled=enabled;
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

void qmpSettingsWindow::postInit()
{
	int sf=0;
	for(int i=0;i<ui->twSoundfont->rowCount();++i)
	if(ui->twSoundfont->item(i,0)->checkState()==Qt::CheckState::Checked)++sf;
	std::string selecteddev;
	std::vector<std::string> devs=qmpMainWindow::getInstance()->getPlayer()->getMidiOutDevices();
	std::set<std::string> devset;
	for(auto dev:devs)devset.insert(dev);
	for(auto setdev:qmpSettingsWindow::getSettingsIntf()->value("Midi/DevicePriority",QList<QVariant>{"Internal FluidSynth"}).toList())
		if(devset.find(setdev.toString().toStdString())!=devset.end())
		{
			selecteddev=setdev.toString().toStdString();
			break;
		}
	if(selecteddev=="Internal FluidSynth"&&!sf)
	{
		// blmark: show dialog at the current screen which user using now.
		int curMonitor = QApplication::desktop()->screenNumber(this);
		if(QMessageBox::question(QDesktopWidget().screen(curMonitor),//this,
		tr("No soundfont loaded"),
		tr("Internal fluidsynth is the only available MIDI output but it has no soundfont set. "
		   "Would you like to setup soundfonts now? You may have to reload the internal synth afterwards."))==QMessageBox::Yes)
		{
			show();
			ui->tabWidget->setCurrentWidget(ui->tab_3);
		}
	}
	devpriod->setupRegisteredDevices();
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
		case 6:
		{
			QFileEdit* fe=(QFileEdit*)i->second.widget;if(!i->second.widget)break;
			settings->setValue(QString(i->first.c_str()),fe->text());
			break;
		}
	}
}

void qmpSettingsWindow::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=0;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
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
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=1;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
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
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].type=2;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
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
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].minv=min;
	customOptions[key].maxv=max;
	customOptions[key].type=3;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
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

void qmpSettingsWindow::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval,bool ispath)
{
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=QString(defaultval.c_str());
	customOptions[key].type=4;
	if(ispath)customOptions[key].type=6;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
		if(customOptPages[tab])page=customOptPages[tab];
		else
		{
			QWidget* w=new QWidget;
			page=new QGridLayout(w);
			w->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
			ui->tabWidget->addTab(w,QString(tab.c_str()));
			customOptPages[tab]=page;
		}
		int row=page->rowCount();
		if(ispath)
		{
			QFileEdit* fe=new QFileEdit(page->parentWidget());
			fe->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
			customOptions[key].widget=fe;
			fe->setText(settings->value(QString(key.c_str()),defaultval.c_str()).toString());
			page->addWidget(fe,row,1);
		}
		else
		{
			QLineEdit* te=new QLineEdit(page->parentWidget());
			te->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
			customOptions[key].widget=te;
			te->setText(settings->value(QString(key.c_str()),defaultval.c_str()).toString());
			page->addWidget(te,row,1);
		}
		QLabel* lb=new QLabel(desc.c_str(),page->parentWidget());
		lb->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
		page->addWidget(lb,row,0);
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
	{
		if(customOptions[key].type==4)
		((QLineEdit*)customOptions[key].widget)->setText(val.c_str());
		else if(customOptions[key].type==6)
		((QFileEdit*)customOptions[key].widget)->setText(val.c_str());
	}
}

void qmpSettingsWindow::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> options,int defaultval)
{
	customOptions[key].widget=nullptr;
	customOptions[key].desc=desc;
	customOptions[key].defaultval=defaultval;
	customOptions[key].type=5;
	if(desc.length())
	{
		QGridLayout* page=nullptr;
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

void qmpSettingsWindow::on_pbCustomizeTb_clicked()
{
	cw->launch(0);
}

void qmpSettingsWindow::on_pbCustomizeAct_clicked()
{
	cw->launch(1);
}

QFileEdit::QFileEdit(QWidget *par):QWidget(par)
{
	QHBoxLayout *layout=new QHBoxLayout(this);
	layout->setMargin(0);
	le=new QLineEdit(this);
	layout->addWidget(le);
	tb=new QToolButton(this);
	tb->setText("...");
	layout->addWidget(tb);
	connect(tb,&QToolButton::clicked,this,&QFileEdit::chooseFile);
}
QString QFileEdit::text(){return le->text();}
void QFileEdit::setText(const QString& s){le->setText(s);}
void QFileEdit::chooseFile()
{
	QString s=QFileDialog::getOpenFileName(nullptr,tr("Select a file"),QFileInfo(text()).dir().absolutePath());
	if(s.length())setText(s);
}

void qmpSettingsWindow::on_pbExtDevSetup_clicked()
{
	dps->launch();
}

void qmpSettingsWindow::on_pbDevPrio_clicked()
{
	devpriod->show();
}
