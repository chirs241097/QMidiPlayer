#include <cstdio>
#include <cmath>
#include <functional>
#include <QUrl>
#include <QFileInfo>
#include <QMimeData>
#include <QFont>
#include <QTextCodec>
#include <QDirIterator>
#include <QMessageBox>
#include <QCheckBox>
#include "qmpmidioutfluid.hpp"
#include "qmpmidiplay.hpp"
#include "qmpmainwindow.hpp"
#include "ui_qmpmainwindow.h"
#define setButtonHeight(x,h) {x->setMaximumHeight(h*(logicalDpiY()/96.));x->setMinimumHeight(h*(logicalDpiY()/96.));}
#define setButtonWidth(x,h) {x->setMaximumWidth(h*(logicalDpiY()/96.));x->setMinimumWidth(h*(logicalDpiY()/96.));}
#ifdef _WIN32
#include <windows.h>
char *wcsto8bit(const wchar_t *s)
{
    int size = WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, s, -1, 0, 0, 0, 0);
    char *c = (char *)calloc(size, sizeof(char));
    WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, s, -1, c, size, 0, 0);
    return c;
}
#endif
#define UPDATE_INTERVAL 66

qmpMainWindow *qmpMainWindow::ref = nullptr;

qmpMainWindow::qmpMainWindow(QCommandLineParser *_clp, QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::qmpMainWindow),
    clp(_clp)
{
    ui->setupUi(this);
    ui->lbCurPoly->setText("00000");
    ui->lbMaxPoly->setText("00000");
    ui->lbFileName->setText("");
    ref = this;
    ui->verticalLayout->setAlignment(ui->pushButton, Qt::AlignRight);
    setButtonHeight(ui->pbNext, 36);
    setButtonHeight(ui->pbPlayPause, 36);
    setButtonHeight(ui->pbAdd, 36);
    setButtonHeight(ui->pbPrev, 36);
    setButtonHeight(ui->pbSettings, 36);
    setButtonHeight(ui->pbStop, 36);
    playing = false;
    stopped = true;
    dragging = false;
    fin = false;
    settings.reset(new qmpSettings());
    settingsw = new qmpSettingsWindow(settings.get(), this);
    player = nullptr;
    timer = nullptr;
    fluidrenderer = nullptr;
}

qmpMainWindow::~qmpMainWindow()
{
    QList<QAction *>a = ui->lbFileName->actions();
    for (unsigned i = 0; i < a.size(); ++i)
    {
        ui->lbFileName->removeAction(a[i]);
        delete a[i];
    }
    pmgr->deinitPlugins();
    auto rtdev = qmpRtMidiManager::getDevices();
    for (auto &i : rtdev)
        player->unregisterMidiOutDevice(i.second);
    delete pmgr;
    if (timer)
        delete timer;
    delete helpw;
    helpw = nullptr;
    delete efxw;
    efxw = nullptr;
    delete chnlw;
    chnlw = nullptr;
    delete plistw;
    plistw = nullptr;
    delete infow;
    infow = nullptr;
    delete settingsw;
    settingsw = nullptr;
    delete panicf;
    panicf = nullptr;
    delete renderf;
    renderf = nullptr;
    delete reloadsynf;
    reloadsynf = nullptr;
    if (player)
        delete player;
    internalfluid->deviceDeinit();
    delete internalfluid;
    delete ui;
}

void qmpMainWindow::init()
{
    show();
    ui->centralWidget->setEnabled(false);

    pmgr = new qmpPluginManager();
    registerMidiOptions();

    std::future<void> f = std::async(std::launch::async, [this]
    {
        player = new CMidiPlayer();
        internalfluid = new qmpMidiOutFluid();
        player->registerMidiOutDevice(internalfluid, "Internal FluidSynth");
        reloadsynf = new qmpReloadSynthFunc(this);

        internalfluid->registerOptions(pmgr->pluginAPI);
        playerSetup(internalfluid);
        internalfluid->deviceInit();
        loadSoundFont(internalfluid);
        for (int i = 0; i < 16; ++i)
            player->setChannelOutput(i, 0);

        auto rtdev = qmpRtMidiManager::getDevices();
        for (auto &i : rtdev)
            player->registerMidiOutDevice(i.first, i.second);
    });
    while (f.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
        QApplication::processEvents();
    ui->centralWidget->setEnabled(true);

    settingsw->registerSoundFontOption();
    registerBehaviorOptions();
    settingsw->registerCustomizeWidgetOptions();

    plistw = new qmpPlistWindow(this);
    chnlw = new qmpChannelsWindow(this);
    efxw = new qmpEfxWindow(this);
    infow = new qmpInfoWindow(this);
    helpw = new qmpHelpWindow(this);
    timer = new QTimer(this);
    renderf = new qmpRenderFunc(this);
    panicf = new qmpPanicFunc(this);
    if (argfiles.size())
    {
        plistw->emptyList();
        for (auto &i : argfiles)
            plistw->insertItem(i);
    }

    if (settings->getOptionBool("Behavior/DialogStatus"))
    {
        QRect g = settings->getOptionRaw("DialogStatus/MainW", QRect()).toRect();
        if (!g.isNull())
            setGeometry(g);
    }

    registerFunctionality(renderf, "Render", tr("Render to wave").toStdString(), getThemedIconc(":/img/render.svg"), 0, false);
    registerFunctionality(panicf, "Panic", tr("Panic").toStdString(), getThemedIconc(":/img/panic.svg"), 0, false);
    registerFunctionality(reloadsynf, "ReloadSynth", tr("Restart fluidsynth").toStdString(), getThemedIconc(":/img/repeat-base.svg"), 0, false);
    const QStringList &qpp = clp->values("plugin");
    std::vector<std::string> pp;
    for (auto &s : qpp)
        pp.push_back(s.toStdString());
    pmgr->scanPlugins(pp);
    settingsw->registerPluginOption(pmgr);
    settingsw->updatePluginList(pmgr);
    pmgr->initPlugins();

    settingsw->registerExtraMidiOptions();

    QVariant *dinif_v = static_cast<QVariant *>(settings->getOptionCustom("Midi/DeviceInitializationFiles"));
    QList<QVariant> devinif_list = dinif_v->toList();
    delete dinif_v;
    QMap<QString, QString> devinif_map;
    for (auto &i : devinif_list)
    {
        QPair<QString, QString> p = i.value<QPair<QString, QString>>();
        devinif_map[p.first] = p.second;
    }
    auto rtdev = qmpRtMidiManager::getDevices();
    for (auto &i : rtdev)
    {
        if (devinif_map.contains(QString(i.second.c_str())))
            i.first->setInitializerFile(devinif_map[QString(i.second.c_str())].toStdString().c_str());
    }
    chnlw->selectDefaultDevice();

    ui->vsMasterVol->setValue(settings->getOptionRaw("FluidSynth/Gain", 50).toInt());
    connect(timer, &QTimer::timeout, this, &qmpMainWindow::updateWidgets);
    connect(timer, &QTimer::timeout, infow, &qmpInfoWindow::updateInfo);
    ui->pbNext->setIcon(QIcon(getThemedIcon(":/img/next.svg")));
    ui->pbPrev->setIcon(QIcon(getThemedIcon(":/img/prev.svg")));
    ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
    ui->pbStop->setIcon(QIcon(getThemedIcon(":/img/stop.svg")));
    ui->pbSettings->setIcon(QIcon(getThemedIcon(":/img/settings.svg")));
    ui->pbAdd->setIcon(QIcon(getThemedIcon(":/img/open.svg")));
    if (argfiles.size())
        on_pbPlayPause_clicked();
    setupWidget();
    settingsw->postInit();
}

int qmpMainWindow::parseArgs()
{
    bool loadfolder = clp->isSet("load-all-files");
    const QStringList &args = clp->positionalArguments();
    for (int i = 0; i < args.size(); ++i)
    {
        if (QFileInfo(args.at(i)).exists())
        {
            if (loadfolder || settings->getOptionBool("Behavior/LoadFolder"))
            {
                QDirIterator di(QFileInfo(args.at(i)).absolutePath());
                while (di.hasNext())
                {
                    QString c = di.next();
                    argfiles.push_back(c);
                }
            }
            else
                argfiles.push_back(args.at(i));
        }
    }
    return 0;
}

void qmpMainWindow::closeEvent(QCloseEvent *event)
{
    if (settings->getOptionBool("Behavior/DialogStatus"))
    {
        settings->setOptionRaw("DialogStatus/MainW", geometry());
    }
    on_pbStop_clicked();
    fin = true;
    for (auto i = mfunc.begin(); i != mfunc.end(); ++i)
    {
        i->second.i()->close();
        i->second.setAssignedControl((QReflectiveAction *)nullptr),
        i->second.setAssignedControl((QReflectivePushButton *)nullptr);
    }
    efxw->close();
    chnlw->close();
    plistw->close();
    infow->close();
    settingsw->close();
    event->accept();
}

void qmpMainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> l = event->mimeData()->urls();
    QStringList sl;
    for (int i = 0; i < l.size(); ++i)
        sl.push_back(l.at(i).toLocalFile());
    plistw->insertItems(sl);
    switchTrack(plistw->getLastItem());
}
void qmpMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //if(event->mimeData()->hasFormat("application/x-midi"))
    event->acceptProposedAction();
}

void qmpMainWindow::updateWidgets()
{
    setFuncEnabled("Render", stopped);
    setFuncEnabled("ReloadSynth", stopped);
    if (player->isFinished() && playerTh)
    {
        if (!plistw->getRepeat())
        {
            timer->stop();
            stopped = true;
            playing = false;
            invokeCallback("main.stop", nullptr);
            setFuncEnabled("Render", stopped);
            setFuncEnabled("ReloadSynth", stopped);
            player->playerDeinit();
            auto f = std::async([this] {playerTh->join();});
            while (f.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
            {
                QApplication::processEvents();
                ui->lbCurPoly->setText(QString("%1").arg(internalfluid->getPolyphone(), 5, 10, QChar('0')));
                ui->lbMaxPoly->setText(QString("%1").arg(internalfluid->getMaxPolyphone(), 5, 10, QChar('0')));
            }
            delete playerTh;
            playerTh = nullptr;
            player->playerPanic(true);
            chnlw->on_pbUnmute_clicked();
            chnlw->on_pbUnsolo_clicked();
            ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
            ui->hsTimer->setValue(0);
            ui->lbCurPoly->setText("00000");
            ui->lbMaxPoly->setText("00000");
            ui->lbCurTime->setText("00:00");
        }
        else
            switchTrack(plistw->getNextItem(), false);
    }
    if (renderTh)
    {
        if (fluidrenderer->isFinished())
        {
            renderTh->join();
            timer->stop();
            ui->centralWidget->setEnabled(true);
            delete renderTh;
            renderTh = nullptr;
            fluidrenderer->renderDeinit();
            delete fluidrenderer;
            fluidrenderer = nullptr;
        }
    }
    while (!player->isFinished() && player->getTCeptr() > player->getStamp(ui->hsTimer->value())
        && ui->hsTimer->value() < 100 && !dragging)
        ui->hsTimer->setValue(ui->hsTimer->value() + 1);
    if (playing)
    {
        std::chrono::duration<double> elapsed =
            std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - st);
        char ts[100];
        sprintf(ts, "%02d:%02d", (int)(elapsed.count() + offset) / 60, (int)(elapsed.count() + offset) % 60);
        ui->lbCurTime->setText(ts);
        ui->lbCurPoly->setText(QString("%1").arg(internalfluid->getPolyphone(), 5, 10, QChar('0')));
        ui->lbMaxPoly->setText(QString("%1").arg(internalfluid->getMaxPolyphone(), 5, 10, QChar('0')));
    }
}

QString qmpMainWindow::getFileName()
{
    return ui->lbFileName->text();
}
void qmpMainWindow::switchTrack(QString s, bool interrupt)
{
    stopped = false;
    playing = true;
    setFuncEnabled("Render", stopped);
    setFuncEnabled("ReloadSynth", stopped);
    ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/pause.svg")));
    if (interrupt)
    {
        player->playerDeinit();
        player->playerPanic();
    }
    invokeCallback("main.stop", nullptr);
    if (playerTh)
    {
        auto f = std::async([this] {playerTh->join();});
        while (f.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
        {
            QApplication::processEvents();
            ui->lbCurPoly->setText(QString("%1").arg(internalfluid->getPolyphone(), 5, 10, QChar('0')));
            ui->lbMaxPoly->setText(QString("%1").arg(internalfluid->getMaxPolyphone(), 5, 10, QChar('0')));
        }
        delete playerTh;
        playerTh = nullptr;
    }
    timer->stop();
    player->playerPanic(true);
    ui->hsTimer->setValue(0);
    chnlw->on_pbUnmute_clicked();
    chnlw->on_pbUnsolo_clicked();
    QString fns = s;
    setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')) + " - QMidiPlayer");
    ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
    if (plistw->getCurrentItem() != fns)
        plistw->setCurrentItem(fns);
    onfnChanged();
    if (!loadFile(fns))
        return;
    char ts[100];
    sprintf(ts, "%02d:%02d", (int)player->getFtime() / 60, (int)player->getFtime() % 60);
    ui->lbFinTime->setText(ts);
    player->playerInit();
    PlaybackStatus ps = getPlaybackStatus();
    invokeCallback("main.start", &ps);
    internalfluid->setGain(ui->vsMasterVol->value() / 250.);
    efxw->sendEfxChange();
    playerTh = new std::thread([this]
    {
        player->playerThread();
        if (settings->getOptionBool("Midi/WaitVoice") && player->isFinished())
            while (internalfluid->getOutputLevel() > -100 && internalfluid->getPolyphone() > 0)
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
#ifdef _WIN32
    SetThreadPriority((void *)playerTh->native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
    st = std::chrono::steady_clock::now();
    offset = 0;
    timer->start(UPDATE_INTERVAL);
}
std::string qmpMainWindow::getTitle()
{
    if (settings->getOptionEnumIntOptName("Midi/TextEncoding") == "Unicode")
        return std::string(player->getTitle());
    return QTextCodec::codecForName(
        settings->getOptionEnumIntOptName("Midi/TextEncoding").c_str())->
        toUnicode(player->getTitle()).toStdString();
}
std::wstring qmpMainWindow::getWTitle()
{
    if (settings->getOptionEnumIntOptName("Midi/TextEncoding") == "Unicode")
        return QString(player->getTitle()).toStdWString();
    return QTextCodec::codecForName(
        settings->getOptionEnumIntOptName("Midi/TextEncoding").c_str())->
        toUnicode(player->getTitle()).toStdWString();
}

void qmpMainWindow::playerSetup(IFluidSettings *fs)
{
    fs->setOptStr("audio.driver", settings->getOptionEnumIntOptName("FluidSynth/AudioDriver").c_str());
    fs->setOptInt("audio.period-size", settings->getOptionInt("FluidSynth/BufSize"));
    fs->setOptInt("audio.periods", settings->getOptionInt("FluidSynth/BufCnt"));
    fs->setOptStr("audio.sample-format", settings->getOptionEnumIntOptName("FluidSynth/SampleFormat").c_str());
    fs->setOptNum("synth.sample-rate", settings->getOptionInt("FluidSynth/SampleRate"));
    fs->setOptInt("synth.polyphony", settings->getOptionInt("FluidSynth/Polyphony"));
    fs->setOptInt("synth.cpu-cores", settings->getOptionInt("FluidSynth/Threads"));
    std::string bsmode;
    if (settings->getOptionBool("FluidSynth/AutoBS") && player->getFileStandard())
        switch (player->getFileStandard())
        {
            case 1:
                bsmode = "gm";
                break;
            case 2:
                bsmode = "mma";
                break;
            case 3:
                bsmode = "gs";
                break;
            case 4:
                bsmode = "xg";
                break;
        }
    else
    {
        bsmode = settings->getOptionEnumIntOptName("FluidSynth/BankSelect");
        std::transform(bsmode.begin(), bsmode.end(), bsmode.begin(), [](char i)
        {
            return tolower(i);
        });
    }
    fs->setOptStr("synth.midi-bank-select", bsmode.c_str());
    player->sendSysX(settings->getOptionBool("Midi/SendSysEx"));
}
void qmpMainWindow::loadSoundFont(IFluidSettings *fs)
{
    QVariant *data = static_cast<QVariant *>(settings->getOptionCustom("FluidSynth/SoundFonts"));
    QList<QVariant> sflist = data->toList();
    for (auto i = sflist.rbegin(); i != sflist.rend(); ++i)
    {
        if (i->toString().startsWith('#'))
            continue;
        QString sf = i->toString();
#ifdef _WIN32
        char *c = wcsto8bit(sf.toStdWString().c_str());
        fs->loadSFont(c);
        free(c);
#else
        fs->loadSFont(sf.toStdString().c_str());
#endif
    }
    delete data;
}
int qmpMainWindow::loadFile(QString fns)
{
#ifdef _WIN32
    char *c = wcsto8bit(fns.toStdWString().c_str());
#else
    std::string s = fns.toStdString();
    const char *c = s.c_str();
#endif
    int ret = 1;
    invokeCallback("main.reset", nullptr);
    if (!player->playerLoadFile(c))
    {
        QMessageBox::critical(this, tr("Error"), tr("%1 is not a valid midi file.").arg(fns));
        ret = 0;
    }
#ifdef _WIN32
    free(c);
#endif
    return ret;
}

void qmpMainWindow::registerMidiOptions()
{
    settings->registerOptionBool("MIDI", "Disable MIDI Mapping", "Midi/DisableMapping", false);
    settings->registerOptionBool("MIDI", "Send system exclusive messages", "Midi/SendSysEx", true);
    settings->registerOptionBool("MIDI", "Wait for remaining voice before stopping", "Midi/WaitVoice", true);
    settings->registerOptionEnumInt("MIDI", "Text encoding", "Midi/TextEncoding", {"Unicode", "Big5", "Big5-HKSCS", "CP949", "EUC-JP", "EUC-KR", "GB18030", "KOI8-R", "KOI8-U", "Macintosh", "Shift-JIS"}, 0);
}

void qmpMainWindow::registerBehaviorOptions()
{
    settings->registerOptionBool("Behavior", "Restore last playlist on startup", "Behavior/RestorePlaylist", false);
    settings->registerOptionBool("Behavior", "Add files in the same folder to playlist", "Behavior/LoadFolder", false);
    settings->registerOptionBool("Behavior", "Save dialog status", "Behavior/DialogStatus", false);
    settings->registerOptionBool("Behavior", "Show labels beside icon in toolbar buttons", "Behavior/ShowButtonLabel", false);
    settings->registerOptionEnumInt("Behavior", "Icon Theme", "Behavior/IconTheme", {"Auto", "Dark", "Light"}, 0);
}

void qmpMainWindow::on_pbPlayPause_clicked()
{
    playing = !playing;
    if (stopped)
    {
        QString fns = plistw->getFirstItem();
        if (!fns.length())
        {
            if (!plistw->on_pbAdd_clicked())
            {
                playing = false;
                return;
            }
            fns = plistw->getFirstItem();
            if (!fns.length())
                return (void)(playing = false);
        }
        setWindowTitle(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')) + " - QMidiPlayer");
        ui->lbFileName->setText(QUrl::fromLocalFile(fns).fileName().left(QUrl::fromLocalFile(fns).fileName().lastIndexOf('.')));
        onfnChanged();
        if (!loadFile(fns))
            return;
        char ts[100];
        sprintf(ts, "%02d:%02d", (int)player->getFtime() / 60, (int)player->getFtime() % 60);
        ui->lbFinTime->setText(ts);
        player->playerInit();
        PlaybackStatus ps = getPlaybackStatus();
        invokeCallback("main.start", &ps);
        internalfluid->setGain(ui->vsMasterVol->value() / 250.);
        efxw->sendEfxChange();
        playerTh = new std::thread([this]
        {
            player->playerThread();
            if (settings->getOptionBool("Midi/WaitVoice") && player->isFinished())
            while (internalfluid->getOutputLevel() > -100 && internalfluid->getPolyphone() > 0)
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
        });
#ifdef _WIN32
        SetThreadPriority((void *)playerTh->native_handle(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
        st = std::chrono::steady_clock::now();
        offset = 0;
        timer->start(UPDATE_INTERVAL);
        stopped = false;
    }
    else
    {
        if (!playing)
        {
            player->playerPanic();
            offset = ui->hsTimer->value() / 100.*player->getFtime();
        }
        else
        {
            st = std::chrono::steady_clock::now();
            player->setResumed();
        }
        player->setTCpaused(!playing);
        PlaybackStatus ps = getPlaybackStatus();
        invokeCallback("main.pause", &ps);
    }
    ui->pbPlayPause->setIcon(QIcon(getThemedIcon(playing ? ":/img/pause.svg" : ":/img/play.svg")));
}

void qmpMainWindow::on_hsTimer_sliderPressed()
{
    dragging = true;
}

void qmpMainWindow::on_hsTimer_sliderReleased()
{
    dragging = false;
    if (playing)
    {
        if (ui->hsTimer->value() == 100)
        {
            on_pbNext_clicked();
            return;
        }
        player->playerPanic();
        player->setTCeptr(player->getStamp(ui->hsTimer->value()), ui->hsTimer->value());
        offset = ui->hsTimer->value() / 100.*player->getFtime();
        st = std::chrono::steady_clock::now();
    }
    else
    {
        if (stopped)
        {
            ui->hsTimer->setValue(0);
            return;
        }
        player->setTCeptr(player->getStamp(ui->hsTimer->value()), ui->hsTimer->value());
        offset = ui->hsTimer->value() / 100.*player->getFtime();
        char ts[100];
        sprintf(ts, "%02d:%02d", (int)(offset) / 60, (int)(offset) % 60);
        ui->lbCurTime->setText(ts);
    }
    PlaybackStatus ps = getPlaybackStatus();
    invokeCallback("main.seek", &ps);
}

uint32_t qmpMainWindow::getPlaybackPercentage()
{
    return ui->hsTimer->value();
}
void qmpMainWindow::playerSeek(uint32_t percentage)
{
    if (percentage > 100)
        percentage = 100;
    if (percentage < 0)
        percentage = 0;
    if (playing)
    {
        if (percentage == 100)
        {
            on_pbNext_clicked();
            return;
        }
        player->playerPanic();
        ui->hsTimer->setValue(percentage);
        player->setTCeptr(player->getStamp(percentage), percentage);
        offset = percentage / 100.*player->getFtime();
        st = std::chrono::steady_clock::now();
    }
    else
    {
        if (stopped)
        {
            ui->hsTimer->setValue(0);
            return;
        }
        player->setTCeptr(player->getStamp(percentage), percentage);
        offset = percentage / 100.*player->getFtime();
        ui->hsTimer->setValue(percentage);
        char ts[100];
        sprintf(ts, "%02d:%02d", (int)(offset) / 60, (int)(offset) % 60);
        ui->lbCurTime->setText(ts);
    }
    PlaybackStatus ps = getPlaybackStatus();
    invokeCallback("main.seek", &ps);
}

PlaybackStatus qmpMainWindow::getPlaybackStatus()
{
    std::chrono::duration<double> elapsed =
        std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - st);
    return {!playing, uint64_t((elapsed.count() + offset) * 1000), uint64_t(player->getFtime() * 1000), player->getTick(), player->getMaxTick()};
}

void qmpMainWindow::on_vsMasterVol_valueChanged()
{
    if (!stopped)
        internalfluid->setGain(ui->vsMasterVol->value() / 250.);
    settings->setOptionRaw("FluidSynth/Gain", ui->vsMasterVol->value());
}

void qmpMainWindow::on_pbStop_clicked()
{
    if (!stopped)
    {
        timer->stop();
        stopped = true;
        playing = false;
        invokeCallback("main.stop", nullptr);
        player->playerDeinit();
        setFuncEnabled("Render", stopped);
        setFuncEnabled("ReloadSynth", stopped);
        player->playerPanic(true);
        if (playerTh)
        {
            playerTh->join();
            delete playerTh;
            playerTh = nullptr;
        }
        chnlw->on_pbUnmute_clicked();
        chnlw->on_pbUnsolo_clicked();
        ui->pbPlayPause->setIcon(QIcon(getThemedIcon(":/img/play.svg")));
        ui->hsTimer->setValue(0);
        ui->lbCurPoly->setText("00000");
        ui->lbMaxPoly->setText("00000");
        ui->lbCurTime->setText("00:00");
    }
}

void qmpMainWindow::dialogClosed()
{
    if (!settingsw->isVisible())
        ui->pbSettings->setChecked(false);
}

void qmpMainWindow::on_pbPrev_clicked()
{
    switchTrack(plistw->getPrevItem());
}

void qmpMainWindow::on_pbNext_clicked()
{
    switchTrack(plistw->getNextItem());
}

void qmpMainWindow::selectionChanged()
{
    switchTrack(plistw->getCurrentItem());
}

void qmpMainWindow::on_lbFileName_customContextMenuRequested(const QPoint &pos)
{
    QMenu menu(ui->lbFileName);
    menu.addActions(ui->lbFileName->actions());
    menu.exec(this->pos() + ui->lbFileName->pos() + pos);
}

void qmpMainWindow::onfnChanged()
{
    if (!ui->lbFileName->text().length())
        return;
    QFont f = ui->lbFileName->font();
    f.setPointSize(18);
    QFontMetrics fm(f);
    QSize size = fm.size(0, ui->lbFileName->text());
    double fw = ui->lbFileName->width() / (double)size.width();
    double fh = ui->lbFileName->height() / (double)size.height();
    double ps = floor(f.pointSizeF() * (fw < fh ? fw : fh));
    if (ps < 6)
        ps = 6;
    f.setPointSizeF(ps > 18 ? 18 : ps);
    ui->lbFileName->setFont(f);
}

int qmpMainWindow::registerUIHook(std::string e, ICallBack *callback, void *userdat)
{
    std::map<int, std::pair<qmpCallBack, void *>> &m = muicb[e];
    int id = 0;
    if (m.size())
        id = m.rbegin()->first + 1;
    m[id] = std::make_pair(qmpCallBack(callback), userdat);
    return id;
}
int qmpMainWindow::registerUIHook(std::string e, callback_t callback, void *userdat)
{
    std::map<int, std::pair<qmpCallBack, void *>> &m = muicb[e];
    int id = 0;
    if (m.size())
        id = m.rbegin()->first + 1;
    m[id] = std::make_pair(qmpCallBack(callback), userdat);
    return id;
}
void qmpMainWindow::unregisterUIHook(std::string e, int hook)
{
    std::map<int, std::pair<qmpCallBack, void *>> &m = muicb[e];
    m.erase(hook);
}

void qmpMainWindow::registerFunctionality(qmpFuncBaseIntf *i, std::string name, std::string desc, const char *icon, int iconlen, bool checkable)
{
    if (mfunc.find(name) != mfunc.end())
        return;
    mfunc[name] = qmpFuncPrivate(i, desc, icon, iconlen, checkable);
}

void qmpMainWindow::unregisterFunctionality(std::string name)
{
    mfunc.erase(name);
    for (auto i = enabled_actions.begin(); i != enabled_actions.end(); ++i)
        if (*i == name)
        {
            enabled_actions.erase(i);
            break;
        }
    for (auto i = enabled_buttons.begin(); i != enabled_buttons.end(); ++i)
        if (*i == name)
        {
            enabled_buttons.erase(i);
            break;
        }
    setupWidget();
}

void qmpMainWindow::setFuncState(std::string name, bool state)
{
    mfunc[name].setChecked(state);
}
void qmpMainWindow::setFuncEnabled(std::string name, bool enable)
{
    mfunc[name].setEnabled(enable);
}

bool qmpMainWindow::isDarkTheme()
{
    if (!settings->getOptionEnumInt("Behavior/IconTheme"))
    {
        return ui->centralWidget->palette().color(QPalette::Window).lightness() < 128;
    }
    else return 2 - settings->getOptionEnumInt("Behavior/IconTheme");
}

void qmpMainWindow::startRender()
{
#ifdef _WIN32
    char *ofstr = wcsto8bit((plistw->getCurrentItem() + QString(".wav")).toStdWString().c_str());
    char *ifstr = wcsto8bit(plistw->getCurrentItem().toStdWString().c_str());
    fluidrenderer = new qmpFileRendererFluid(ifstr, ofstr);
    playerSetup(fluidrenderer);
    fluidrenderer->renderInit();
    free(ofstr);
    free(ifstr);
#else
    fluidrenderer = new qmpFileRendererFluid(
        plistw->getCurrentItem().toStdString().c_str(),
        (plistw->getCurrentItem() + QString(".wav")).toStdString().c_str()
    );
    playerSetup(fluidrenderer);
    fluidrenderer->renderInit();
#endif
    loadSoundFont(fluidrenderer);
    ui->centralWidget->setEnabled(false);
    fluidrenderer->setGain(ui->vsMasterVol->value() / 250.);
    efxw->sendEfxChange(fluidrenderer);
    timer->start(UPDATE_INTERVAL);
    renderTh = new std::thread(&qmpFileRendererFluid::renderWorker, fluidrenderer);
}

void qmpMainWindow::reloadSynth()
{
    ui->centralWidget->setEnabled(false);
    std::future<void> f = std::async(std::launch::async,
            [this]
    {
        internalfluid->deviceDeinit(true);
        playerSetup(internalfluid);
        internalfluid->deviceInit();
        loadSoundFont(internalfluid);
    }
        );
    while (f.wait_for(std::chrono::milliseconds(100)) == std::future_status::timeout)
        QApplication::processEvents();
    ui->centralWidget->setEnabled(true);
}

std::map<std::string, qmpFuncPrivate> &qmpMainWindow::getFunc()
{
    return mfunc;
}

void qmpMainWindow::setupWidget()
{
    for (auto i = mfunc.begin(); i != mfunc.end(); ++i)
    {
        i->second.setAssignedControl(static_cast<QReflectiveAction *>(nullptr));
        i->second.setAssignedControl(static_cast<QReflectivePushButton *>(nullptr));
    }
    QVariant *v = static_cast<QVariant *>(settings->getOptionCustom("Behavior/Toolbar"));
    enabled_buttons.clear();
    for (auto i : v->toList())
        enabled_buttons.push_back(i.toString().toStdString());
    delete v;
    v = static_cast<QVariant *>(settings->getOptionCustom("Behavior/Actions"));
    enabled_actions.clear();
    for (auto i : v->toList())
        enabled_actions.push_back(i.toString().toStdString());
    delete v;
    QList<QWidget *>w = ui->buttonwidget->findChildren<QWidget *>("", Qt::FindDirectChildrenOnly);
    qDeleteAll(w);
    QList<QAction *>a = ui->lbFileName->actions();
    for (int i = 0; i < a.size(); ++i)
    {
        ui->lbFileName->removeAction(a[i]);
        delete a[i];
    }
    for (unsigned i = 0; i < enabled_buttons.size(); ++i)
    {
        if (mfunc.find(enabled_buttons[i]) == mfunc.end())
            continue;
        QReflectivePushButton *pb = new QReflectivePushButton(
            mfunc[enabled_buttons[i]].icon(),
            tr(mfunc[enabled_buttons[i]].desc().c_str()),
            enabled_buttons[i]
        );
        setButtonHeight(pb, 32);
        //!!TODO
        if (settings->getOptionBool("Behavior/ShowButtonLabel"))
        {
            pb->setText(tr(mfunc[enabled_buttons[i]].desc().c_str()));
            pb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        }
        else
            setButtonWidth(pb, 32);
        pb->setIconSize(QSize(16, 16));
        ui->buttonwidget->layout()->addWidget(pb);
        mfunc[enabled_buttons[i]].setAssignedControl(pb);
        connect(pb, &QReflectivePushButton::onClick, this, &qmpMainWindow::funcReflector);
    }
    for (unsigned i = 0; i < enabled_actions.size(); ++i)
    {
        if (mfunc.find(enabled_actions[i]) == mfunc.end())
            continue;
        QReflectiveAction *a = new QReflectiveAction(
            mfunc[enabled_actions[i]].icon(),
            tr(mfunc[enabled_actions[i]].desc().c_str()),
            enabled_actions[i]
        );
        ui->lbFileName->addAction(a);
        mfunc[enabled_actions[i]].setAssignedControl(a);
        connect(a, &QReflectiveAction::onClick, this, &qmpMainWindow::funcReflector);
    }
    ui->buttonwidget->layout()->setAlignment(Qt::AlignLeft);
}

void qmpMainWindow::invokeCallback(std::string cat, void *callerdat)
{
    std::map<int, std::pair<qmpCallBack, void *>> *mp;
    mp = &muicb[cat];
    for (auto &i : *mp)
        i.second.first(callerdat, i.second.second);
}

void qmpMainWindow::on_pbSettings_clicked()
{
    if (ui->pbSettings->isChecked())
        settingsw->show();
    else settingsw->close();
}

void qmpMainWindow::funcReflector(std::string reflt)
{
    if (mfunc[reflt].isCheckable())
    {
        mfunc[reflt].setChecked(!mfunc[reflt].isChecked());
        if (mfunc[reflt].isChecked())
            mfunc[reflt].i()->show();
        else
            mfunc[reflt].i()->close();
    }
    else mfunc[reflt].i()->show();
}

void qmpMainWindow::on_pushButton_clicked()
{
    helpw->show();
}

qmpFuncPrivate::qmpFuncPrivate(qmpFuncBaseIntf *i, std::string _desc, const char *icon, int iconlen, bool checkable):
    _i(i), des(_desc), _checkable(checkable)
{
    if (icon)
    {
        QImage img;
        if (icon[0] == ':' && icon[1] == '/' || icon[0] == 'q' && icon[1] == 'r' && icon[2] == 'c')
            img = QImage(QString(icon));
        else
            img.loadFromData((uchar *)icon, iconlen);
        QPixmap pixm;
        pixm.convertFromImage(img);
        _icon = QIcon(pixm);
    }
    else _icon = QIcon();
    checked = false;
    asgna = nullptr;
    asgnb = nullptr;
}

void qmpMainWindow::on_pbAdd_clicked()
{
    if (plistw->on_pbAdd_clicked())
        switchTrack(plistw->getLastItem());
}
