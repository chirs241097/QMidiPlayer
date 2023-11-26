#include <cstdio>
#include "qmpmpris.hpp"
#include "qmpriswrapper.hpp"
#include "qmpmprisimpl.hpp"

qmpMPrisPlugin::qmpMPrisPlugin(qmpPluginAPI *_api)
{
    api = _api;
}
qmpMPrisPlugin::~qmpMPrisPlugin()
{
    api = nullptr;
}
void qmpMPrisPlugin::init()
{
    mw = QMPrisWrapper::create<QMPPlayer, QMPMediaPlayer2, QMPTrackList>("qmidiplayer", api);
}
void qmpMPrisPlugin::deinit()
{
    delete mw;
}
const char *qmpMPrisPlugin::pluginGetName()
{
    return "QMidiPlayer MPris Support";
}
const char *qmpMPrisPlugin::pluginGetVersion()
{
    return "0.8.8";
}
