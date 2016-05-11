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
	if(!plyr->getSFCount())return e->ignore();
	int sfc=plyr->getSFCount();
	for(int i=sfc-1;i>=0;--i)
	{
		fluid_sfont_t* psf=plyr->getSFPtr(i);
		fluid_preset_t preset;
		psf->iteration_start(psf);
		while(psf->iteration_next(psf,&preset))
			strcpy(presets[preset.get_banknum(&preset)][preset.get_num(&preset)],preset.get_name(&preset));
	}
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
	if(!plyr->getSFCount())return;
	ch=chid;int b=0,p=0,r;char name[64];
	sprintf(name,"Preset Selection - Channel #%d",ch+1);
	setWindowTitle(name);
	plyr->getChannelPreset(chid,&b,&p,name);
	for(int i=0;i<ui->lwBankSelect->count();++i)
	{
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

void qmpPresetSelector::on_pbCancel_clicked()
{
	close();
}

void qmpPresetSelector::on_pbOk_clicked()
{
	CMidiPlayer *plyr=qmpMainWindow::getInstance()->getPlayer();
	if(!ui->lwBankSelect->currentItem()||!ui->lwPresetSelect->currentItem())return (void)close();
	int b,p;sscanf(ui->lwBankSelect->currentItem()->text().toStdString().c_str(),"%d",&b);
	sscanf(ui->lwPresetSelect->currentItem()->text().toStdString().c_str(),"%d",&p);
	plyr->setChannelPreset(ch,b,p);
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
