#ifndef QMPMPRIS_HPP
#define QMPMPRIS_HPP

#include "../include/qmpcorepublic.hpp"

class QMPrisWrapper;
class qmpMPrisPlugin: public qmpPluginIntf
{
private:
    qmpPluginAPI *api;
    QMPrisWrapper *mw = nullptr;
public:
    qmpMPrisPlugin(qmpPluginAPI *_api);
    ~qmpMPrisPlugin();
    void init();
    void deinit();
    const char *pluginGetName();
    const char *pluginGetVersion();
};

extern "C" {
    EXPORTSYM qmpPluginIntf *qmpPluginGetInterface(qmpPluginAPI *api)
    {
        return new qmpMPrisPlugin(api);
    }
    EXPORTSYM const char *qmpPluginGetAPIRev()
    {
        return QMP_PLUGIN_API_REV;
    }
}

#endif
