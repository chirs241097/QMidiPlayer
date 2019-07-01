#include <cstdio>
#include "qmppresetselect.hpp"
#include "ui_qmppresetselect.h"
#include "qmpmainwindow.hpp"

qmpPresetSelector::qmpPresetSelector(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpPresetSelector)
{
	ui->setupUi(this);
}

qmpPresetSelector::~qmpPresetSelector()
{
	delete ui;
}

void qmpPresetSelector::showEvent(QShowEvent *e)
{
	memset(presets,0,sizeof(presets));
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	if(!plyr->fluid()->getSFCount())return e->ignore();
	ui->lwBankSelect->clear();
	ui->lwPresetSelect->clear();
	e->accept();
}
void qmpPresetSelector::setupWindow(int chid)
{
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	if(!plyr->fluid()->getSFCount())return;
	ch=chid;int r;char name[256];
	uint16_t b;uint8_t p;
	std::string pstname;
	sprintf(name,"Preset Selection - Channel #%d",ch+1);
	setWindowTitle(name);
	r=plyr->getChannelOutputDevice(ch)->getChannelPreset(ch,&b,&p,pstname);
	if(!r){b=plyr->getCC(ch,0)<<7|plyr->getCC(ch,32);p=plyr->getCC(ch,128);}
	ui->lwBankSelect->blockSignals(true);
	ui->lwBankSelect->clear();
	ui->lwPresetSelect->clear();
	ui->lwBankSelect->blockSignals(false);
	if(plyr->getChannelOutputDevice(ch)->getBankList().empty()){
		ui->lwPresetSelect->setEnabled(false);
		ui->lwBankSelect->setEnabled(false);
		ui->spCustomLSB->setEnabled(true);
		ui->spCustomMSB->setEnabled(true);
		ui->spCustomPC->setEnabled(true);
		ui->spCustomMSB->setValue(plyr->getCC(chid,0));
		ui->spCustomLSB->setValue(plyr->getCC(chid,32));
		ui->spCustomPC->setValue(p);
	}
	else{
		ui->lwPresetSelect->setEnabled(true);
		ui->lwBankSelect->setEnabled(true);
		ui->spCustomLSB->setEnabled(false);
		ui->spCustomMSB->setEnabled(false);
		ui->spCustomPC->setEnabled(false);
		ui->lwBankSelect->blockSignals(true);
		for(auto&i:plyr->getChannelOutputDevice(ch)->getBankList())
		{
			snprintf(name,256,"%03d %s",i.first,i.second.c_str());
			ui->lwBankSelect->addItem(name);
			if(i.first==b)ui->lwBankSelect->setCurrentRow(ui->lwBankSelect->count()-1);
		}
		ui->lwBankSelect->blockSignals(false);
		for(auto&i:plyr->getChannelOutputDevice(ch)->getPresets(b))
		{
			snprintf(name,256,"%03d %s",i.first,i.second.c_str());
			ui->lwPresetSelect->addItem(name);
			if(i.first==p)ui->lwPresetSelect->setCurrentRow(ui->lwPresetSelect->count()-1);
		}
	}
}

void qmpPresetSelector::on_pbCancel_clicked()
{
	close();
}

void qmpPresetSelector::on_pbOk_clicked()
{
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	if(plyr->getChannelOutput(ch)){
		if(ui->spCustomMSB->isEnabled())
		plyr->setChannelPreset(ch,(ui->spCustomMSB->value()<<7)|ui->spCustomLSB->value(),ui->spCustomPC->value());
		else
		{
			int b=ui->lwBankSelect->currentItem()->text().split(' ').first().toInt();
			int p=ui->lwPresetSelect->currentItem()->text().split(' ').first().toInt();
			plyr->setChannelPreset(ch,b,p);
		}
	}
	else{
		if(!ui->lwBankSelect->currentItem()||!ui->lwPresetSelect->currentItem())return (void)close();
		int b,p;
		b=ui->lwBankSelect->currentItem()->text().toInt();
		p=ui->lwPresetSelect->currentItem()->text().split(' ').first().toInt();
		QString s=qmpSettingsWindow::getSettingsIntf()->value("Audio/BankSelect","CC#0").toString();
		if(s=="CC#32"){
			if(b==128)b=127<<7;
		}
		else if(s=="CC#0")b<<=7;
		plyr->setChannelPreset(ch,b,p);
	}
	close();
}

void qmpPresetSelector::on_lwPresetSelect_itemDoubleClicked()
{
	on_pbOk_clicked();
}

void qmpPresetSelector::on_lwBankSelect_currentRowChanged()
{
	ui->lwPresetSelect->clear();
	if(!ui->lwBankSelect->currentItem())return;
	char name[256];int b;
	sscanf(ui->lwBankSelect->currentItem()->text().toStdString().c_str(),"%d",&b);
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	for(auto&i:plyr->getChannelOutputDevice(ch)->getPresets(b))
	{
		snprintf(name,256,"%03d %s",i.first,i.second.c_str());
		ui->lwPresetSelect->addItem(name);
	}
}

void qmpPresetSelector::on_buttonBox_accepted()
{
	on_pbOk_clicked();
}

void qmpPresetSelector::on_buttonBox_rejected()
{
	on_pbCancel_clicked();
}
