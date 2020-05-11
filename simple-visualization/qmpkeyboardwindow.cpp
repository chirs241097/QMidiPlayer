#include <QVBoxLayout>
#include <QGridLayout>
#include <QCloseEvent>
#include <QLabel>
#include <QFontDatabase>
#include "qmppianowidget.hpp"
#include "qmpkeyboardwindow.hpp"

qmpKeyboardWindow::qmpKeyboardWindow(qmpPluginAPI *_api, QWidget *parent):
    QWidget(parent, Qt::Dialog), api(_api)
{
    setWindowTitle("Keyboard");
    QGridLayout *grid;
    setLayout(grid = new QGridLayout());
    for (int ch = 0; ch < 16; ++ch)
    {
        grid->addWidget(lb[ch] = new QLabel, ch, 0);
        grid->addWidget(pw[ch] = new qmpPianoWidget(this), ch, 1);
        lb[ch]->setFont(QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont));
        pw[ch]->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Preferred);
        QPalette p = palette();
        p.setColor(QPalette::ColorRole::Highlight, api->getOptionUint("Keyboard/acolor" + std::to_string(ch)));
        p.setColor(QPalette::ColorRole::Base, api->getOptionUint("Keyboard/bcolor" + std::to_string(ch)));
        pw[ch]->setPalette(p);
    }
    hide();
    eh = api->registerEventHandler(
            [this](const void *ee, void *)
    {
        const SEvent *e = (const SEvent *)ee;
        int ch = e->type & 0xF;
        if ((e->type & 0xF0) == 0x80 || ((e->type & 0xF0) == 0x90 && e->p2 == 0))
            emit keystateupdated(ch, e->p1, false);
        if ((e->type & 0xF0) == 0x90 && e->p2 > 0)
            emit keystateupdated(ch, e->p1, e->p2 > 0);
        if ((e->type & 0xF0) == 0xB0 || (e->type & 0xF0) == 0xC0)
            lb[ch]->setText(
                QString::fromStdString(api->getChannelPresetString(ch)) +
                QString("\nch:%1 v:0x%2 p:0x%3 e:0x%4")
                .arg(ch + 1, 2, 10, QChar('0'))
                .arg(api->getChannelCC(ch, 0x7), 2, 16, QChar('0'))
                .arg(api->getChannelCC(ch, 0xa), 2, 16, QChar('0'))
                .arg(api->getChannelCC(ch, 0xb), 2, 16, QChar('0')));
    }
    , nullptr, true);
    connect(this, &qmpKeyboardWindow::keystateupdated, this, &qmpKeyboardWindow::onkeystatesupdate);
}
qmpKeyboardWindow::~qmpKeyboardWindow()
{
    api->unregisterEventHandler(eh);
}
void qmpKeyboardWindow::closeEvent(QCloseEvent *event)
{
    api->setFuncState("Keyboard", false);
    event->accept();
}
void qmpKeyboardWindow::onkeystatesupdate(int ch, int key, bool state)
{
    pw[ch]->setKeyState(key, state);
}
void qmpKeyboardWindow::resetAll()
{
    for (int ch = 0; ch < 16; ++ch)
    {
        pw[ch]->reset();
        lb[ch]->setText(
            QString::fromStdString(api->getChannelPresetString(ch)) +
            QString("\nch:%1 v:0x%2 p:0x%3 e:0x%4")
            .arg(ch + 1, 2, 10, QChar('0'))
            .arg(api->getChannelCC(ch, 0x7), 2, 16, QChar('0'))
            .arg(api->getChannelCC(ch, 0xa), 2, 16, QChar('0'))
            .arg(api->getChannelCC(ch, 0xb), 2, 16, QChar('0')));
    }
}
