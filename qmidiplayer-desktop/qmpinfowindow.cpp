#include "qmpinfowindow.hpp"
#include "ui_qmpinfowindow.h"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"

const char *minors = "abebbbf c g d a e b f#c#g#d#a#";
const char *majors = "CbGbDbAbEbBbF C G D A E B F#C#";
const char *standards = "?  GM GM2GS XG ";

qmpInfoWindow::qmpInfoWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::qmpInfoWindow)
{
    ui->setupUi(this);
    qmpMainWindow::getInstance()->registerFunctionality(
        infof = new qmpInfoFunc(this),
        std::string("FileInfo"),
        tr("File Information").toStdString(),
        getThemedIconc(":/img/info.svg"),
        0,
        true
    );
}

qmpInfoWindow::~qmpInfoWindow()
{
    qmpMainWindow::getInstance()->unregisterFunctionality("FileInfo");
    delete infof;
    delete ui;
}

void qmpInfoWindow::closeEvent(QCloseEvent *e)
{
    setVisible(false);
    qmpMainWindow::getInstance()->setFuncState("FileInfo", false);
    e->accept();
}
void qmpInfoWindow::hideEvent(QHideEvent *e)
{
    qmpMainWindow::getInstance()->setFuncState("FileInfo", false);
    e->accept();
}


void qmpInfoWindow::updateInfo()
{
    char str[256];
    CMidiPlayer *player = qmpMainWindow::getInstance()->getPlayer();
    std::string textencoding = qmpMainWindow::getInstance()->getSettings()->getOptionEnumIntOptName("Midi/TextEncoding");
    ui->lbFileName->setText(QString("File name: ") + qmpMainWindow::getInstance()->getFileName());
    if (player->getTitle())
    {
        ui->lbTitle->setText(QString("Title: %1").arg(qmpMainWindow::decodeString(player->getTitle())));
    }
    else ui->lbTitle->setText(QString("Title: "));
    if (player->getCopyright())
    {
        ui->lbCopyright->setText(QString("Copyright: %1").arg(qmpMainWindow::decodeString(player->getCopyright())));
    }
    else ui->lbCopyright->setText(QString("Copyright: "));
    ui->lbTempo->setText(QString("Tempo: ") + QString::number(player->getTempo(), 'g', 5));
    int t, r;
    t = player->getCurrentKeySignature();
    r = (int8_t)((t >> 8) & 0xFF) + 7;
    strncpy(str, t & 0xFF ? minors + 2 * r : majors + 2 * r, 2);
    str[2] = '\0';
    ui->lbKeySig->setText(QString("Key Sig.: ") + str);
    player->getCurrentTimeSignature(&t, &r);
    sprintf(str, "Time Sig.: %d/%d", t, r);
    ui->lbTimeSig->setText(str);
    sprintf(str, "Note count: %u", player->getFileNoteCount());
    ui->lbNoteCount->setText(str);
    strncpy(str, standards + player->getFileStandard() * 3, 3);
    str[3] = '\0';
    ui->lbFileStandard->setText(QString("File standard: ") + str);
}

qmpInfoFunc::qmpInfoFunc(qmpInfoWindow *par)
{
    p = par;
}
void qmpInfoFunc::show()
{
    p->show();
}
void qmpInfoFunc::close()
{
    p->close();
}
