#ifndef MIDIFMTPLUGIN_HPP
#define MIDIFMTPLUGIN_HPP

#include <cstdio>
#include "../include/qmpcorepublic.hpp"

class CMidiStreamReader: public qmpFileReader
{
private:
    CMidiFile *ret;
    FILE *f;
    int eventdiscarded, fmt;
    uint32_t readDWLE();
    bool RIFFHeaderReader();
    bool midsBodyReader();
public:
    CMidiStreamReader();
    ~CMidiStreamReader();
    CMidiFile *readFile(const char *fn);
    void discardCurrentEvent();
    void commitEventChange(SEvent d);
};

class qmpMidiFmtPlugin: public qmpPluginIntf
{
private:
    CMidiStreamReader *mdsreader;
public:
    static qmpPluginAPI *api;
    qmpMidiFmtPlugin(qmpPluginAPI *_api);
    ~qmpMidiFmtPlugin();
    void init();
    void deinit();
    const char *pluginGetName();
    const char *pluginGetVersion();
};

extern "C" {
    EXPORTSYM qmpPluginIntf *qmpPluginGetInterface(qmpPluginAPI *api)
    {
        return new qmpMidiFmtPlugin(api);
    }
    EXPORTSYM const char *qmpPluginGetAPIRev()
    {
        return QMP_PLUGIN_API_REV;
    }
}

#endif // MIDIFMTPLUGIN_HPP
