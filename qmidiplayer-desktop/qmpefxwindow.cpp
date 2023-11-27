#include <cmath>
#include "qmpefxwindow.hpp"
#include "ui_qmpefxwindow.h"
#include "qmpmainwindow.hpp"
#include "qmpmidioutfluid.hpp"

qmpEfxWindow::qmpEfxWindow(QWidget *parent) :
    QWidget(parent, Qt::Dialog),
    ui(new Ui::qmpEfxWindow)
{
    ui->setupUi(this);
    initialized = false;
    styl = new QDialSkulptureStyle();
    QList<QDial *> dials = findChildren<QDial *>();
    for (int i = 0; i < dials.count(); ++i)
        dials.at(i)->setStyle(styl);
    qmpSettings *settings = qmpMainWindow::getInstance()->getSettings();
    ui->cbEnabledC->setChecked(settings->getOptionRaw("Effects/ChorusEnabled", 1).toInt());
    ui->cbEnabledR->setChecked(settings->getOptionRaw("Effects/ReverbEnabled", 1).toInt());
    rr = settings->getOptionRaw("Effects/ReverbRoom", 0.2).toDouble();
    rd = settings->getOptionRaw("Effects/ReverbDamp", 0.0).toDouble();
    rw = settings->getOptionRaw("Effects/ReverbWidth", 0.5).toDouble();
    rl = settings->getOptionRaw("Effects/ReverbLevel", 0.9).toDouble();

    cfb = settings->getOptionRaw("Effects/ChorusFeedbk", 3).toInt();
    cl = settings->getOptionRaw("Effects/ChorusLevel", 2.0).toDouble();
    cr = settings->getOptionRaw("Effects/ChorusRate", 0.3).toDouble();
    cd = settings->getOptionRaw("Effects/ChorusDepth", 8.0).toDouble();
    ct = settings->getOptionRaw("Effects/ChorusType", FLUID_CHORUS_MOD_SINE).toInt();
    qmpMainWindow::getInstance()->registerFunctionality(
        efxf = new qmpEfxFunc(this),
        std::string("Effects"),
        tr("Effects").toStdString(),
        getThemedIconc(":/img/effects.svg"),
        0,
        true
    );
    connect(ui->sbRoom, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbDamp, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbWidth, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbLevelR, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbFeedBack, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbRate, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbDepth, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    connect(ui->sbLevelC, qOverload<int>(&QSpinBox::valueChanged), this, &qmpEfxWindow::spinValueChange);
    if (!settings->getOptionRaw("DialogStatus/EfxW", QRect()).toRect().isNull())
        setGeometry(settings->getOptionRaw("DialogStatus/EfxW", QRect()).toRect());
    if (settings->getOptionRaw("DialogStatus/EfxWShown", 0).toInt())
    {
        show();
        qmpMainWindow::getInstance()->setFuncState("Effects", true);
    }
}

qmpEfxWindow::~qmpEfxWindow()
{
    qmpMainWindow::getInstance()->unregisterFunctionality("Effects");
    delete efxf;
    delete styl;
    delete ui;
}

void qmpEfxWindow::closeEvent(QCloseEvent *event)
{
    qmpSettings *settings = qmpMainWindow::getInstance()->getSettings();
    if (settings->getOptionBool("Behavior/DialogStatus"))
    {
        settings->setOptionRaw("DialogStatus/EfxW", geometry());
    }
    setVisible(false);
    if (!qmpMainWindow::getInstance()->isFinalizing() && settings->getOptionBool("Behavior/DialogStatus"))
    {
        settings->setOptionRaw("DialogStatus/EfxWShown", 0);
    }
    qmpMainWindow::getInstance()->setFuncState("Effects", false);
    event->accept();
}

void qmpEfxWindow::showEvent(QShowEvent *event)
{
    //These parameters will never be modified outside this window...
    /*if(initialized)
    {
        player->getReverbPara(&rr,&rd,&rw,&rl);
        player->getChorusPara(&cfb,&cl,&cr,&cd,&ct);
    }*/
    ui->sbRoom->setValue((int)round(rr * 100));
    ui->dRoom->setValue((int)round(rr * 100));
    ui->sbDamp->setValue((int)round(rd * 100));
    ui->dDamp->setValue((int)round(rd * 100));
    ui->sbWidth->setValue((int)round(rw * 100));
    ui->dWidth->setValue((int)round(rw * 100));
    ui->sbLevelR->setValue((int)round(rl * 100));
    ui->dLevelR->setValue((int)round(rl * 100));

    ui->sbFeedBack->setValue(cfb);
    ui->dFeedBack->setValue(cfb);
    ui->sbRate->setValue(cr);
    ui->dRate->setValue((int)round(cr * 100));
    ui->sbDepth->setValue(cd);
    ui->dDepth->setValue((int)round(cd * 10));
    ui->sbLevelC->setValue((int)round(cl * 100));
    ui->dLevelC->setValue((int)round(cl * 100));
    if (ct == FLUID_CHORUS_MOD_SINE)
    {
        ui->rbSine->setChecked(true);
        ui->rbTriangle->setChecked(false);
    }
    if (ct == FLUID_CHORUS_MOD_TRIANGLE)
    {
        ui->rbSine->setChecked(false);
        ui->rbTriangle->setChecked(true);
    }
    initialized = true;
    qmpSettings *settings = qmpMainWindow::getInstance()->getSettings();
    if (!settings->getOptionRaw("DialogStatus/EfxW", QRect()).toRect().isNull())
        setGeometry(settings->getOptionRaw("DialogStatus/EfxW", QRect()).toRect());
    if (settings->getOptionBool("Behavior/DialogStatus"))
    {
        settings->setOptionRaw("DialogStatus/EfxWShown", 1);
    }
    event->accept();
}

void qmpEfxWindow::sendEfxChange(void *_fs)
{
    if (!qmpMainWindow::getInstance() || !initialized)
        return;
    rr = ui->sbRoom->value() / 100.;
    rd = ui->sbDamp->value() / 100.;
    rw = ui->sbWidth->value() / 100.;
    rl = ui->sbLevelR->value() / 100.;
    ct = ui->rbSine->isChecked() ? FLUID_CHORUS_MOD_SINE : FLUID_CHORUS_MOD_TRIANGLE;
    cfb = ui->sbFeedBack->value();
    cl = ui->sbLevelC->value() / 100.;
    cr = ui->sbRate->value();
    cd = ui->sbDepth->value();
    IFluidSettings *fs = (IFluidSettings *)_fs;
    if (!_fs)
        fs = qmpMainWindow::getInstance()->getFluid();
    fs->setReverbPara(ui->cbEnabledR->isChecked() ? 1 : 0, rr, rd, rw, rl);
    fs->setChorusPara(ui->cbEnabledC->isChecked() ? 1 : 0, cfb, cl, cr, cd, ct);

    qmpSettings *settings = qmpMainWindow::getInstance()->getSettings();
    settings->setOptionRaw("Effects/ChorusEnabled", ui->cbEnabledC->isChecked() ? 1 : 0);
    settings->setOptionRaw("Effects/ReverbEnabled", ui->cbEnabledR->isChecked() ? 1 : 0);
    settings->setOptionRaw("Effects/ReverbRoom", rr);
    settings->setOptionRaw("Effects/ReverbDamp", rd);
    settings->setOptionRaw("Effects/ReverbWidth", rw);
    settings->setOptionRaw("Effects/ReverbLevel", rl);

    settings->setOptionRaw("Effects/ChorusFeedbk", cfb);
    settings->setOptionRaw("Effects/ChorusLevel", cl);
    settings->setOptionRaw("Effects/ChorusRate", cr);
    settings->setOptionRaw("Effects/ChorusDepth", cd);
    settings->setOptionRaw("Effects/ChorusType", ct);
}

void qmpEfxWindow::dailValueChange()
{
    if (!initialized)
        return;
    ui->sbRoom->setValue(ui->dRoom->value());
    ui->sbDamp->setValue(ui->dDamp->value());
    ui->sbWidth->setValue(ui->dWidth->value());
    ui->sbLevelR->setValue(ui->dLevelR->value());
    ui->sbFeedBack->setValue(ui->dFeedBack->value());
    ui->sbRate->setValue(ui->dRate->value() / 100.);
    ui->sbDepth->setValue(ui->dDepth->value() / 10.);
    ui->sbLevelC->setValue(ui->dLevelC->value());
    sendEfxChange();
}

void qmpEfxWindow::spinValueChange()
{
    if (!initialized)
        return;
    ui->dRoom->setValue(ui->sbRoom->value());
    ui->dDamp->setValue(ui->sbDamp->value());
    ui->dWidth->setValue(ui->sbWidth->value());
    ui->dLevelR->setValue(ui->sbLevelR->value());
    ui->dFeedBack->setValue(ui->sbFeedBack->value());
    ui->dRate->setValue((int)(ui->sbRate->value() * 100));
    ui->dDepth->setValue((int)(ui->sbDepth->value() * 10));
    ui->dLevelC->setValue(ui->sbLevelC->value());
    sendEfxChange();
}

void qmpEfxWindow::on_dRoom_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dDamp_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dWidth_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dLevelR_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dFeedBack_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dRate_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dDepth_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_dLevelC_valueChanged()
{
    dailValueChange();
}

void qmpEfxWindow::on_cbEnabledC_stateChanged()
{
    sendEfxChange();
}

void qmpEfxWindow::on_cbEnabledR_stateChanged()
{
    sendEfxChange();
}

void qmpEfxWindow::on_rbSine_toggled()
{
    sendEfxChange();
}

void qmpEfxWindow::on_rbTriangle_toggled()
{
    sendEfxChange();
}

qmpEfxFunc::qmpEfxFunc(qmpEfxWindow *par)
{
    p = par;
}
void qmpEfxFunc::show()
{
    p->show();
}
void qmpEfxFunc::close()
{
    p->close();
}
