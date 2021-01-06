#ifndef QMPCOREPUBLIC_HPP
#define QMPCOREPUBLIC_HPP
#include <cstdint>
#include <functional>
#include <vector>
#include <string>
#ifdef _WIN32
#define EXPORTSYM __declspec(dllexport)
#else
#define EXPORTSYM __attribute__ ((visibility ("default")))
#endif
#define QMP_PLUGIN_API_REV "1+indev8"
//MIDI Event structure
struct SEvent
{
    uint32_t iid, time;
    uint8_t type, p1, p2;
    uint8_t flags;
    std::string str;
    SEvent()
    {
        time = iid = 0;
        type = p1 = p2 = 0;
        flags = 0;
        str = "";
    }
    SEvent(uint32_t _iid, uint32_t _t, uint8_t _tp, uint8_t _p1, uint8_t _p2, const char *s = nullptr)
    {
        iid = _iid;
        time = _t;
        type = _tp;
        p1 = _p1;
        p2 = _p2;
        flags = 0;
        if (s)str = std::string(s);
        else str = "";
    }
    SEvent(uint32_t _iid, uint32_t _t, uint8_t _tp, uint8_t _p1, uint8_t _p2, std::string s):
        iid(_iid), time(_t), type(_tp), p1(_p1), p2(_p2), str(s) {}
    friend bool operator <(const SEvent &a, const SEvent &b)
    {
        if (a.time != b.time)
            return a.time < b.time;
        else if ((a.type == 0xF0) ^ (b.type == 0xF0))
            return (a.type == 0xF0) > (b.type == 0xF0);
        else return a.iid < b.iid;
    }
};
//MIDI Track class
class CMidiTrack
{
public:
    std::vector<SEvent> eventList;
    void appendEvent(SEvent e)
    {
        eventList.push_back(e);
    }
    SEvent &operator[](size_t sub)
    {
        return eventList[sub];
    }
};
//MIDI File class
class CMidiFile
{
public:
    bool valid;
    char *title, *copyright;
    std::vector<CMidiTrack> tracks;
    uint32_t std, divs;
    ~CMidiFile()
    {
        if (title)delete[] title;
        if (copyright)delete[] copyright;
    }
};
struct PlaybackStatus
{
    bool paused;
    uint64_t curtime_ms;
    uint64_t maxtime_ms;
    uint64_t curtick;
    uint64_t maxtick;
};
//Generic callback function that can be used for hooking the core.
//"userdata" is set when you register the callback function.
//Deprecated. Removing in 0.9.x.
class ICallBack
{
public:
    ICallBack() {}
    virtual void callBack(const void *callerdata, void *userdata) = 0;
    virtual ~ICallBack() {}
};
//alternative callback function type
typedef std::function<void(const void *, void *)> callback_t;
//MIDI file reader interface. Use this to implement your file importer.
class qmpFileReader
{
public:
    qmpFileReader() {}
    virtual ~qmpFileReader() {}
    virtual CMidiFile *readFile(const char *fn) = 0;
    virtual void discardCurrentEvent() = 0;
    virtual void commitEventChange(SEvent d) = 0;
};
//Functionality interface.
class qmpFuncBaseIntf
{
public:
    qmpFuncBaseIntf() {}
    virtual void show() = 0;
    virtual void close() = 0;
    virtual ~qmpFuncBaseIntf() {}
};
//Midi mapper plugin interface.
class qmpMidiOutDevice
{
public:
    qmpMidiOutDevice() {}
    virtual void deviceInit() = 0;
    virtual void deviceDeinit() = 0;
    virtual void basicMessage(uint8_t type, uint8_t p1, uint8_t p2) = 0;
    virtual void extendedMessage(uint32_t length, const char *data) = 0;
    virtual void rpnMessage(uint8_t ch, uint16_t type, uint16_t val) = 0;
    virtual void nrpnMessage(uint8_t ch, uint16_t type, uint16_t val) = 0;
    virtual void panic(uint8_t ch) = 0;
    virtual void reset(uint8_t ch) = 0;
    virtual void onMapped(uint8_t ch, int refcnt) = 0;
    virtual void onUnmapped(uint8_t ch, int refcnt) = 0;
    virtual std::vector<std::pair<uint16_t, std::string>> getBankList() = 0;
    virtual std::vector<std::pair<uint8_t, std::string>> getPresets(uint16_t bank) = 0;
    virtual std::string getPresetName(uint16_t bank, uint8_t preset) = 0;
    virtual bool getChannelPreset(int ch, uint16_t *bank, uint8_t *preset, std::string &presetname) = 0;
    virtual uint8_t getInitialCCValue(uint8_t cc, uint8_t ch) = 0;
    virtual ~qmpMidiOutDevice() {}
};
//Main plugin interface.
class qmpPluginIntf
{
public:
    qmpPluginIntf() {}
    virtual ~qmpPluginIntf() {}
    virtual void init() {}
    virtual void deinit() {}
    virtual const char *pluginGetName()
    {
        return "";
    }
    virtual const char *pluginGetVersion()
    {
        return "";
    }
};
#ifdef QMP_MAIN
extern "C" {
#endif
//The API class provided by the core. Plugins use this class to interact with
//the core.
    class qmpPluginAPI
    {
    public:
        virtual ~qmpPluginAPI() {}
        virtual uint32_t getDivision() = 0;
        virtual uint32_t getRawTempo() = 0;
        virtual double getRealTempo() = 0;
        virtual uint32_t getTimeSig() = 0;
        virtual int getKeySig() = 0;
        virtual uint32_t getNoteCount() = 0;
        virtual uint32_t getMaxTick() = 0;
        virtual uint32_t getCurrentPolyphone() = 0;
        virtual uint32_t getMaxPolyphone() = 0;
        virtual uint32_t getCurrentTimeStamp() = 0;
        virtual uint32_t getCurrentPlaybackPercentage() = 0;
        virtual PlaybackStatus getPlaybackStatus() = 0;
        virtual int getChannelCC(int ch, int cc) = 0;
        virtual int getChannelPreset(int ch) = 0;
        virtual void playerSeek(uint32_t percentage) = 0;
        virtual double getPitchBend(int ch) = 0;
        virtual void getPitchBendRaw(int ch, uint32_t *pb, uint32_t *pbr) = 0;
        virtual bool getChannelMask(int ch) = 0;
        virtual std::string getTitle() = 0;
        virtual std::wstring getWTitle() = 0;
        virtual std::string getChannelPresetString(int ch) = 0;
        virtual bool isDarkTheme() = 0;
        virtual void *getMainWindow() = 0;

        //WARNING!!: This function should be called from event reader callbacks only and
        //it is somehow dangerous -- other plugins might be unaware of the removal of the
        //event. The design might be modified afterward.
        virtual void discardCurrentEvent() = 0;
        //WARNING!!: This function should be called from event reader callbacks only and
        //it is somehow dangerous -- other plugins might be unaware of the event change.
        //The design might be modified afterward.
        virtual void commitEventChange(SEvent d) = 0;
        //This function should be called from a file reader when it has read a new event
        virtual void callEventReaderCB(SEvent d) = 0;
        virtual void setFuncState(std::string name, bool state) = 0;
        virtual void setFuncEnabled(std::string name, bool enable) = 0;

        virtual void registerFunctionality(qmpFuncBaseIntf *i, std::string name, std::string desc, const char *icon, int iconlen, bool checkable) = 0;
        virtual void unregisterFunctionality(std::string name) = 0;
        virtual int registerUIHook(std::string e, ICallBack *cb, void *userdat) = 0;
        virtual int registerUIHook(std::string e, callback_t cb, void *userdat) = 0;
        virtual void unregisterUIHook(std::string e, int hook) = 0;
        virtual void registerMidiOutDevice(qmpMidiOutDevice *dev, std::string name) = 0;
        virtual void unregisterMidiOutDevice(std::string name) = 0;
        virtual int registerEventReaderIntf(ICallBack *cb, void *userdata) = 0;
        virtual void unregisterEventReaderIntf(int intfhandle) = 0;
        virtual int registerEventHandlerIntf(ICallBack *cb, void *userdata) = 0;
        virtual void unregisterEventHandlerIntf(int intfhandle) = 0;
        virtual int registerFileReadFinishedHandlerIntf(ICallBack *cb, void *userdata) = 0;
        virtual void unregisterFileReadFinishedHandlerIntf(int intfhandle) = 0;
        virtual int registerEventHandler(callback_t cb, void *userdata, bool post = false) = 0;
        virtual void unregisterEventHandler(int id) = 0;
        virtual int registerEventReadHandler(callback_t cb, void *userdata) = 0;
        virtual void unregisterEventReadHandler(int id) = 0;
        virtual int registerFileReadFinishHook(callback_t cb, void *userdata) = 0;
        virtual void unregisterFileReadFinishHook(int id) = 0;
        virtual void registerFileReader(qmpFileReader *reader, std::string name) = 0;
        virtual void unregisterFileReader(std::string name) = 0;

        //if desc=="", the option won't be visible in the settings form.
        //it will only show up in the configuration file.
        virtual void registerOptionInt(std::string tab, std::string desc, std::string key, int min, int max, int defaultval) = 0;
        virtual int getOptionInt(std::string key) = 0;
        virtual void setOptionInt(std::string key, int val) = 0;
        virtual void registerOptionUint(std::string tab, std::string desc, std::string key, unsigned min, unsigned max, unsigned defaultval) = 0;
        virtual unsigned getOptionUint(std::string key) = 0;
        virtual void setOptionUint(std::string key, unsigned val) = 0;
        virtual void registerOptionBool(std::string tab, std::string desc, std::string key, bool defaultval) = 0;
        virtual bool getOptionBool(std::string key) = 0;
        virtual void setOptionBool(std::string key, bool val) = 0;
        virtual void registerOptionDouble(std::string tab, std::string desc, std::string key, double min, double max, double defaultval) = 0;
        virtual double getOptionDouble(std::string key) = 0;
        virtual void setOptionDouble(std::string key, double val) = 0;
        virtual void registerOptionString(std::string tab, std::string desc, std::string key, std::string defaultval, bool ispath = false) = 0;
        virtual std::string getOptionString(std::string key) = 0;
        virtual void setOptionString(std::string key, std::string val) = 0;
        virtual void registerOptionEnumInt(std::string tab, std::string desc, std::string key, std::vector<std::string> options, int defaultval) = 0;
        virtual int getOptionEnumInt(std::string key) = 0;
        virtual void setOptionEnumInt(std::string key, int val) = 0;
    };
#ifdef QMP_MAIN
}
#endif
//The entry type for the plugin. Your plugin should implement
//qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
//as its entry point. A pointer to the core API is also passed to the plugin
//through the parameter. This function should return a pointer to a class
//that implementes the plugin pinterface (qmpPluginIntf).
typedef qmpPluginIntf *(*qmpPluginEntry)(qmpPluginAPI *);
//The following symbol only presents in plugins. Its purpose is to help the core reject incompatible plugins.
typedef const char *(*qmpPluginAPIRevEntry)();
#endif // QMPCOREPUBLIC_HPP
