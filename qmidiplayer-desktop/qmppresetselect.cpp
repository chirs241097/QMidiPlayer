#include <cstdio>
#include "qmppresetselect.hpp"
#include "ui_qmppresetselect.h"
#include "qmpmainwindow.hpp"

qmpPresetSelector::qmpPresetSelector(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpPresetSelector)
{
	ui->setupUi(this);
	int w=size().width(),h=size().height();w=w*(logicalDpiX()/96.);h=h*(logicalDpiY()/96.);
	setMaximumWidth(w);setMaximumHeight(h);setMinimumWidth(w);setMinimumHeight(h);
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
	std::vector<std::pair<std::pair<int,int>,std::string>>
		presetlist=plyr->fluid()->listPresets();
	for(auto &i:presetlist)
		strcpy(presets[i.first.first][i.first.second],i.second.c_str());
	ui->lwBankSelect->clear();
	ui->lwPresetSelect->clear();
	for(int i=0;i<=128;++i)
	{
		int b=0;
		for(int j=0;j<128;++j)if(strlen(presets[i][j])){b=1;break;}
		if(b)ui->lwBankSelect->addItem(QString::number(i));
	}
	e->accept();
}
void qmpPresetSelector::setupWindow(int chid)
{
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	if(!plyr->fluid()->getSFCount())return;
	ch=chid;int b=0,p=0,r;char name[256];
	sprintf(name,"Preset Selection - Channel #%d",ch+1);
	setWindowTitle(name);
	plyr->getChannelPreset(chid,&b,&p,name);
	if(plyr->getChannelOutput(chid)){
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
		for(int i=0;i<ui->lwBankSelect->count();++i){
			sscanf(ui->lwBankSelect->item(i)->text().toStdString().c_str(),"%3d",&r);
			if(r==b){ui->lwBankSelect->setCurrentRow(i);break;}
		}
		r=0;
		ui->lwPresetSelect->clear();
		for(int i=0,cr=0;i<128;++i)
		if(strlen(presets[b][i]))
		{
			sprintf(name,"%03d %s",i,presets[b][i]);
			if(i==p)r=cr;
			ui->lwPresetSelect->addItem(name);
			cr++;
		}
		ui->lwPresetSelect->setCurrentRow(r);
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
		plyr->setChannelPreset(ch,(ui->spCustomMSB->value()<<7)|ui->spCustomLSB->value(),ui->spCustomPC->value());
		//plyr->setCC(ch,0,ui->spCustomMSB->value());
		//plyr->setCC(ch,32,ui->spCustomLSB->value());
	}
	else{
		if(!ui->lwBankSelect->currentItem()||!ui->lwPresetSelect->currentItem())return (void)close();
		int b,p;sscanf(ui->lwBankSelect->currentItem()->text().toStdString().c_str(),"%d",&b);
		sscanf(ui->lwPresetSelect->currentItem()->text().toStdString().c_str(),"%d",&p);
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
	char name[30];int b;
	sscanf(ui->lwBankSelect->currentItem()->text().toStdString().c_str(),"%d",&b);
	for(int i=0;i<128;++i)
	if(strlen(presets[b][i]))
	{
		sprintf(name,"%03d %s",i,presets[b][i]);
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
