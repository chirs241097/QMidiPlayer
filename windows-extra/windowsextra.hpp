#ifndef SAMPLEPLUGIN_H
#define SAMPLEPLUGIN_H

#include "../include/qmpcorepublic.hpp"

#include <QWinTaskbarButton>

class qmpWindowsExtraPlugin: public qmpPluginIntf
{
private:
    qmpPluginAPI *api;
public:
    qmpWindowsExtraPlugin(qmpPluginAPI *_api);
    ~qmpWindowsExtraPlugin();
    void init();
    void deinit();
    const char *pluginGetName();
    const char *pluginGetVersion();

private:
    QWinTaskbarButton * m_taskbarIcon = nullptr;
    QTimer * m_timer = nullptr;
    QMetaObject::Connection m_timerConnection;
    int ui_start, ui_stop, ui_pause, ui_reset;
};

extern "C" {
    EXPORTSYM qmpPluginIntf *qmpPluginGetInterface(qmpPluginAPI *api)
    {
        return new qmpWindowsExtraPlugin(api);
    }
    EXPORTSYM const char *qmpPluginGetAPIRev()
    {
        return QMP_PLUGIN_API_REV;
    }
}

#endif // SAMPLEPLUGIN_H
