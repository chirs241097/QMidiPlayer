#ifndef QMPPLUGIN_H
#define QMPPLUGIN_H
#define QMP_MAIN
#include <string>
#include <vector>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
struct qmpPlugin
{
    std::string name, version, path;
    qmpPluginIntf *pinterface;
    bool enabled, initialized;
    qmpPlugin(std::string _n, std::string _v, std::string _p, qmpPluginIntf *_i)
    {
        name = _n;
        version = _v;
        path = _p;
        pinterface = _i;
        enabled = initialized = false;
    }
};
class qmpMainWindow;
class qmpSettings;
class qmpPluginAPIImpl: public qmpPluginAPI
{
public:
    qmpPluginAPIImpl();
    ~qmpPluginAPIImpl();
    uint32_t getDivision();
    uint32_t getRawTempo();
    double getRealTempo();
    uint32_t getTimeSig();
    int getKeySig();
    uint32_t getNoteCount();
    uint32_t getMaxTick();
    uint32_t getCurrentPolyphone();
    uint32_t getMaxPolyphone();
    uint32_t getCurrentTimeStamp();
    uint32_t getCurrentPlaybackPercentage();
    PlaybackStatus getPlaybackStatus();
    int getChannelCC(int ch, int cc);
    int getChannelPreset(int ch);
    void playerSeek(uint32_t percentage);
    double getPitchBend(int ch);
    void getPitchBendRaw(int ch, uint32_t *pb, uint32_t *pbr);
    bool getChannelMask(int ch);
    std::string getTitle();
    std::wstring getWTitle();
    std::string getChannelPresetString(int ch);
    bool isDarkTheme();
    void *getMainWindow();

    void discardCurrentEvent();
    void commitEventChange(SEvent d);
    void callEventReaderCB(SEvent d);
    void setFuncState(std::string name, bool state);
    void setFuncEnabled(std::string name, bool enable);

    void registerFunctionality(qmpFuncBaseIntf *i, std::string name, std::string desc, const char *icon, int iconlen, bool checkable);
    void unregisterFunctionality(std::string name);
    int registerUIHook(std::string e, ICallBack *cb, void *userdat);
    int registerUIHook(std::string e, callback_t cb, void *userdat);
    void unregisterUIHook(std::string e, int hook);
    void registerMidiOutDevice(qmpMidiOutDevice *dev, std::string name);
    void unregisterMidiOutDevice(std::string name);
    int registerEventReaderIntf(ICallBack *cb, void *userdata);
    void unregisterEventReaderIntf(int intfhandle);
    int registerEventHandlerIntf(ICallBack *cb, void *userdata);
    void unregisterEventHandlerIntf(int intfhandle);
    int registerFileReadFinishedHandlerIntf(ICallBack *cb, void *userdata);
    void unregisterFileReadFinishedHandlerIntf(int intfhandle);
    int registerEventHandler(callback_t cb, void *userdata, bool post = false);
    void unregisterEventHandler(int id);
    int registerEventReadHandler(callback_t cb, void *userdata);
    void unregisterEventReadHandler(int id);
    int registerFileReadFinishHook(callback_t cb, void *userdata);
    void unregisterFileReadFinishHook(int id);
    void registerFileReader(qmpFileReader *reader, std::string name);
    void unregisterFileReader(std::string name);

    void registerOptionInt(std::string tab, std::string desc, std::string key, int min, int max, int defaultval);
    int getOptionInt(std::string key);
    void setOptionInt(std::string key, int val);
    void registerOptionUint(std::string tab, std::string desc, std::string key, unsigned min, unsigned max, unsigned defaultval);
    unsigned getOptionUint(std::string key);
    void setOptionUint(std::string key, unsigned val);
    void registerOptionBool(std::string tab, std::string desc, std::string key, bool defaultval);
    bool getOptionBool(std::string key);
    void setOptionBool(std::string key, bool val);
    void registerOptionDouble(std::string tab, std::string desc, std::string key, double min, double max, double defaultval);
    double getOptionDouble(std::string key);
    void setOptionDouble(std::string key, double val);
    void registerOptionString(std::string tab, std::string desc, std::string key, std::string defaultval, bool ispath = false);
    std::string getOptionString(std::string key);
    void setOptionString(std::string key, std::string val);
    void registerOptionEnumInt(std::string tab, std::string desc, std::string key, std::vector<std::string> options, int defaultval);
    int getOptionEnumInt(std::string key);
    void setOptionEnumInt(std::string key, int val);
};
class qmpPluginManager
{
private:
    std::vector<qmpPlugin> plugins;
    static qmpPluginAPIImpl *pluginAPI;
    static qmpMainWindow *mainwindow;
    static qmpSettings *settings;
public:
    qmpPluginManager();
    ~qmpPluginManager();
    std::vector<qmpPlugin> *getPlugins();
    void scanPlugins(const std::vector<std::string> &pp);
    void initPlugins();
    void deinitPlugins();
    friend class qmpPluginAPIImpl;
    friend class qmpMainWindow;
};
#endif // QMPPLUGIN_H
