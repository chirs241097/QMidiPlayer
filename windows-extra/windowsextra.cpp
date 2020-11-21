#include <cstdio>
#include "windowsextra.hpp"

#include <QMainWindow>
#include <QTimer>
#include <QWinTaskbarProgress>

qmpWindowsExtraPlugin::qmpWindowsExtraPlugin(qmpPluginAPI *_api)
{
    api = _api;
}
qmpWindowsExtraPlugin::~qmpWindowsExtraPlugin()
{
    api = nullptr;
}
void qmpWindowsExtraPlugin::init()
{
    QMainWindow * w = static_cast<QMainWindow *>(api->getMainWindow());
    m_timer = new QTimer();
    m_taskbarIcon = new QWinTaskbarButton();
    m_taskbarIcon->setWindow(w->windowHandle());

    m_timerConnection = QObject::connect(m_timer, &QTimer::timeout, [ = ](){
        m_taskbarIcon->progress()->setValue(api->getCurrentPlaybackPercentage());
    });

    ui_start = api->registerUIHook("main.start", [this](const void *, void *)
    {
        m_taskbarIcon->progress()->show();
        m_taskbarIcon->progress()->resume();
        m_timer->start(250);
    }, nullptr);
    ui_stop = api->registerUIHook("main.stop", [this](const void *, void *)
    {
        m_taskbarIcon->progress()->stop();
        m_taskbarIcon->progress()->hide();
    }, nullptr);
    ui_pause = api->registerUIHook("main.pause", [this](const void *, void *)
    {
        PlaybackStatus ps = api->getPlaybackStatus();
        if (ps.paused) {
            m_taskbarIcon->progress()->pause();
        } else {
            m_taskbarIcon->progress()->resume();
        }
    }, nullptr);
    ui_reset = api->registerUIHook("main.reset", [this](const void *, void *)
    {
        m_taskbarIcon->progress()->reset();
    }, nullptr);
}
void qmpWindowsExtraPlugin::deinit()
{
    QObject::disconnect(m_timerConnection);

    m_timer->deleteLater();
    m_taskbarIcon->deleteLater();

    api->unregisterUIHook("main.start", ui_start);
    api->unregisterUIHook("main.stop", ui_stop);
    api->unregisterUIHook("main.pause", ui_pause);
    api->unregisterUIHook("main.reset", ui_reset);
}
const char *qmpWindowsExtraPlugin::pluginGetName()
{
    return "Windows Extra Feature Plugin";
}
const char *qmpWindowsExtraPlugin::pluginGetVersion()
{
    return "0.0.0";
}
