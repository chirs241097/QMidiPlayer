#include "qmpmidiplay.hpp"
#include "qmpvisrendercore.hpp"
#include "qmpsettingsro.hpp"
#include "qmppluginapistub.hpp"

#include <QTextCodec>

qmpPluginAPIStub::qmpPluginAPIStub(qmpVisRenderCore *_core):
    core(_core)
{
}

qmpPluginAPIStub::~qmpPluginAPIStub()
{
    core = nullptr;
}

uint32_t qmpPluginAPIStub::getDivision()
{
    return core->player->getDivision();
}
uint32_t qmpPluginAPIStub::getRawTempo()
{
    return 0;
}
double qmpPluginAPIStub::getRealTempo()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getTimeSig()
{
    return 0;
}
int qmpPluginAPIStub::getKeySig()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getNoteCount()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getMaxTick()
{
    return core->player->getMaxTick();
}
uint32_t qmpPluginAPIStub::getCurrentPolyphone()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getMaxPolyphone()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getCurrentTimeStamp()
{
    return 0;
}
uint32_t qmpPluginAPIStub::getCurrentPlaybackPercentage()
{
    return 0;
}

PlaybackStatus qmpPluginAPIStub::getPlaybackStatus()
{
    return {false, 0, 0, 0, 0};
}
int qmpPluginAPIStub::getChannelCC(int ch, int cc)
{
    return 0;
}
int qmpPluginAPIStub::getChannelPreset(int ch)
{
    return 0;
}
void qmpPluginAPIStub::playerSeek(uint32_t percentage) {}
double qmpPluginAPIStub::getPitchBend(int ch)
{
    return 0;
}
void qmpPluginAPIStub::getPitchBendRaw(int ch, uint32_t *pb, uint32_t *pbr) {}
bool qmpPluginAPIStub::getChannelMask(int ch)
{
    return 0;
}
std::string qmpPluginAPIStub::getTitle()
{
    if (core->settings()->getOptionEnumIntOptName("Midi/TextEncoding") == "Unicode")
        return std::string(core->player->getTitle());
    return QTextCodec::codecForName(
            core->settings()->getOptionEnumIntOptName("Midi/TextEncoding").c_str())->
        toUnicode(core->player->getTitle()).toStdString();
}
std::wstring qmpPluginAPIStub::getWTitle()
{
    if (core->settings()->getOptionEnumIntOptName("Midi/TextEncoding") == "Unicode")
        return QString(core->player->getTitle()).toStdWString();
    return QTextCodec::codecForName(
            core->settings()->getOptionEnumIntOptName("Midi/TextEncoding").c_str())->
        toUnicode(core->player->getTitle()).toStdWString();
}

std::string qmpPluginAPIStub::getFilePath()
{
    return "";
}

std::wstring qmpPluginAPIStub::getWFilePath()
{
    return L"";
}
std::string qmpPluginAPIStub::getChannelPresetString(int ch)
{
    return std::string();
}
bool qmpPluginAPIStub::isDarkTheme()
{
    return false;
}
void *qmpPluginAPIStub::getMainWindow()
{
    return nullptr;
}

void qmpPluginAPIStub::playbackControl(PlaybackControlCommand cmd, void *data) {}
void qmpPluginAPIStub::discardCurrentEvent() {}
void qmpPluginAPIStub::commitEventChange(SEvent d) {}
void qmpPluginAPIStub::callEventReaderCB(SEvent d) {}
void qmpPluginAPIStub::setFuncState(std::string name, bool state) {}
void qmpPluginAPIStub::setFuncEnabled(std::string name, bool enable)
{}
void qmpPluginAPIStub::registerFunctionality(qmpFuncBaseIntf *i, std::string name, std::string desc, const char *icon, int iconlen, bool checkable)
{
    if (name == "Visualization")
        core->vf = i;
}
void qmpPluginAPIStub::unregisterFunctionality(std::string name)
{
    if (name == "Visualization")
        core->vf = nullptr;
}

int qmpPluginAPIStub::registerUIHook(std::string e, ICallBack *cb, void *userdat) {return -1;}
int qmpPluginAPIStub::registerUIHook(std::string e, callback_t cb, void *userdat)
{
    if (e == "main.start")
        core->startcb = cb;
    if (e == "main.reset")
        core->resetcb = cb;
    return 0;
}
void qmpPluginAPIStub::unregisterUIHook(std::string e, int hook)
{
    if (e == "main.start")
        core->startcb = nullptr;
    if (e == "main.reset")
        core->resetcb = nullptr;
}

void qmpPluginAPIStub::registerMidiOutDevice(qmpMidiOutDevice *dev, std::string name) {}
void qmpPluginAPIStub::unregisterMidiOutDevice(std::string name) {}

int qmpPluginAPIStub::registerEventReaderIntf(ICallBack *cb, void *userdata) {return -1;}
void qmpPluginAPIStub::unregisterEventReaderIntf(int intfhandle) {}
int qmpPluginAPIStub::registerEventHandlerIntf(ICallBack *cb, void *userdata) {return -1;}
void qmpPluginAPIStub::unregisterEventHandlerIntf(int intfhandle) {}
int qmpPluginAPIStub::registerFileReadFinishedHandlerIntf(ICallBack *cb, void *userdata) {return -1;}
void qmpPluginAPIStub::unregisterFileReadFinishedHandlerIntf(int intfhandle) {}

int qmpPluginAPIStub::registerEventHandler(callback_t cb, void *userdata, bool post) {return -1;}
void qmpPluginAPIStub::unregisterEventHandler(int id) {}
int qmpPluginAPIStub::registerEventReadHandler(callback_t cb, void *userdata)
{
    return core->player->registerEventReadHandler(cb, userdata);
}
void qmpPluginAPIStub::unregisterEventReadHandler(int id)
{
    core->player->unregisterEventReadHandler(id);
}
int qmpPluginAPIStub::registerFileReadFinishHook(callback_t cb, void *userdata)
{
    return core->player->registerFileReadFinishHook(cb, userdata);
}
void qmpPluginAPIStub::unregisterFileReadFinishHook(int id)
{
    core->player->unregisterFileReadFinishHook(id);
}

void qmpPluginAPIStub::registerFileReader(qmpFileReader *reader, std::string name) {}
void qmpPluginAPIStub::unregisterFileReader(std::string name) {}

void qmpPluginAPIStub::registerOptionInt(std::string tab, std::string desc, std::string key, int min, int max, int defaultval)
{
    core->settings()->registerOptionInt(tab, desc, key, min, max, defaultval);
}
int qmpPluginAPIStub::getOptionInt(std::string key)
{
    return core->settings()->getOptionInt(key);
}
void qmpPluginAPIStub::setOptionInt(std::string key, int val)
{
    core->settings()->setOptionInt(key, val);
}

void qmpPluginAPIStub::registerOptionUint(std::string tab, std::string desc, std::string key, unsigned min, unsigned max, unsigned defaultval)
{
    core->settings()->registerOptionUint(tab, desc, key, min, max, defaultval);
}
unsigned qmpPluginAPIStub::getOptionUint(std::string key)
{
    return core->settings()->getOptionUint(key);
}
void qmpPluginAPIStub::setOptionUint(std::string key, unsigned val)
{
    return core->settings()->setOptionUint(key, val);
}

void qmpPluginAPIStub::registerOptionBool(std::string tab, std::string desc, std::string key, bool defaultval)
{
    core->settings()->registerOptionBool(tab, desc, key, defaultval);
}
bool qmpPluginAPIStub::getOptionBool(std::string key)
{
    return core->settings()->getOptionBool(key);
}
void qmpPluginAPIStub::setOptionBool(std::string key, bool val)
{
    core->settings()->setOptionBool(key, val);
}

void qmpPluginAPIStub::registerOptionDouble(std::string tab, std::string desc, std::string key, double min, double max, double defaultval)
{
    core->settings()->registerOptionDouble(tab, desc, key, min, max, defaultval);
}
double qmpPluginAPIStub::getOptionDouble(std::string key)
{
    return core->settings()->getOptionDouble(key);
}
void qmpPluginAPIStub::setOptionDouble(std::string key, double val)
{
    core->settings()->setOptionDouble(key, val);
}

void qmpPluginAPIStub::registerOptionString(std::string tab, std::string desc, std::string key, std::string defaultval, bool ispath)
{
    core->settings()->registerOptionString(tab, desc, key, defaultval, ispath);
}
std::string qmpPluginAPIStub::getOptionString(std::string key)
{
    return core->settings()->getOptionString(key);
}
void qmpPluginAPIStub::setOptionString(std::string key, std::string val)
{
    core->settings()->setOptionString(key, val);
}

void qmpPluginAPIStub::registerOptionEnumInt(std::string tab, std::string desc, std::string key, std::vector<std::string> options, int defaultval)
{
    core->settings()->registerOptionEnumInt(tab, desc, key, options, defaultval);
}
int qmpPluginAPIStub::getOptionEnumInt(std::string key)
{
    return core->settings()->getOptionEnumInt(key);
}
void qmpPluginAPIStub::setOptionEnumInt(std::string key, int val)
{
    core->settings()->setOptionEnumInt(key, val);
}
