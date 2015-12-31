#include "qmpinfowindow.hpp"
#include "ui_qmpinfowindow.h"
#include "qmpmainwindow.hpp"

const char* minors="abebbbf c g d a e b f#c#g#d#a#";
const char* majors="CbGbDbAbEbBbF C G D A E B F#C#";

qmpInfoWindow::qmpInfoWindow(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::qmpInfoWindow)
{
	ui->setupUi(this);
}

qmpInfoWindow::~qmpInfoWindow()
{
	delete ui;
}

void qmpInfoWindow::updateInfo()
{
	char str[256];
	CMidiPlayer* player=qmpMainWindow::getInstance()->getPlayer();
	ui->lbFileName->setText(QString("File name: ")+qmpMainWindow::getInstance()->getFileName());
	if(player->getTitle())ui->lbTitle->setText(QString("Title: ")+player->getTitle());
	if(player->getCopyright())ui->lbCopyright->setText(QString("Copyright: ")+player->getCopyright());
	ui->lbTempo->setText(QString("Tempo: ")+QString::number(player->getTempo(),'g',5));
	int t,r;player->getCurrentKeySignature(&t);r=(int8_t)((t>>8)&0xFF)+7;
	strncpy(str,t&0xFF?minors+2*r:majors+2*r,2);str[2]='\0';
	ui->lbKeySig->setText(QString("Key Sig.: ")+str);
	player->getCurrentTimeSignature(&t,&r);sprintf(str,"Time Sig.: %d/%d",t,r);
	ui->lbTimeSig->setText(str);
}
