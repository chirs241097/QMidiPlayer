#ifndef QMPMIDIOUTFLUID_H
#define QMPMIDIOUTFLUID_H
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include "../include/qmpcorepublic.hpp"
#include <fluidsynth.h>
class IFluidSettings
{
public:
    virtual void setOptStr(const char *opt, const char *val) = 0;
    virtual void setOptInt(const char *opt, int val) = 0;
    virtual void setOptNum(const char *opt, double val) = 0;
    virtual void loadSFont(const char *path) = 0;
    virtual void setGain(double gain) = 0;
    virtual void setReverbPara(int e, double r, double d, double w, double l) = 0;
    virtual void setChorusPara(int e, int fb, double l, double r, double d, int type) = 0;
};
class qmpMidiOutFluid: public qmpMidiOutDevice, public IFluidSettings
{
private:
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_audio_driver_t *adriver;
    std::vector<std::pair<uint16_t, std::string>> bnk;
    std::unordered_map<uint16_t, std::vector<std::string>> pst;
    qmpPluginAPI *coreapi;
    std::vector<std::string> drivers;
    int default_driver = -1;
    void update_preset_list();
    double output_level;
    int voice_count;
public:
    qmpMidiOutFluid();
    ~qmpMidiOutFluid();
    void registerOptions(qmpPluginAPI *coreapi);
    void deviceInit();
    void deviceDeinit();
    void deviceDeinit(bool freshsettings);
    void basicMessage(uint8_t type, uint8_t p1, uint8_t p2);
    void extendedMessage(uint32_t length, const char *data);
    void rpnMessage(uint8_t ch, uint16_t type, uint16_t val);
    void nrpnMessage(uint8_t ch, uint16_t type, uint16_t val);
    void panic(uint8_t ch);
    void reset(uint8_t ch);
    void onMapped(uint8_t ch, int refcnt);
    void onUnmapped(uint8_t ch, int refcnt);
    std::vector<std::pair<uint16_t, std::string>> getBankList();
    std::vector<std::pair<uint8_t, std::string>> getPresets(uint16_t bank);
    std::string getPresetName(uint16_t bank, uint8_t preset);
    bool getChannelPreset(int ch, uint16_t *bank, uint8_t *preset, std::string &presetname);
    uint8_t getInitialCCValue(uint8_t cc, uint8_t ch);
    //FluidSynth specific stuff
    void setOptStr(const char *opt, const char *val);
    void setOptInt(const char *opt, int val);
    void setOptNum(const char *opt, double val);
    void loadSFont(const char *path);
    int getSFCount();

    int getPolyphone();
    int getMaxPolyphone();
    double getOutputLevel();
    void setGain(double gain);
    void getChannelInfo(int ch, int *b, int *p, char *s);
    void getReverbPara(double *r, double *d, double *w, double *l);
    void setReverbPara(int e, double r, double d, double w, double l);
    void getChorusPara(int *fb, double *l, double *r, double *d, int *type);
    void setChorusPara(int e, int fb, double l, double r, double d, int type);
};
class qmpFileRendererFluid: public IFluidSettings
{
private:
    fluid_settings_t *settings;
    fluid_synth_t *synth;
    fluid_player_t *player;
    bool finished;
    std::string fn;
public:
    qmpFileRendererFluid(const char *_fn, const char *_ofn);
    ~qmpFileRendererFluid();
    void renderInit();
    void renderDeinit();
    void renderWorker();
    void setOptStr(const char *opt, const char *val);
    void setOptInt(const char *opt, int val);
    void setOptNum(const char *opt, double val);
    void loadSFont(const char *path);
    bool isFinished();
    void setGain(double gain);
    void setReverbPara(int e, double r, double d, double w, double l);
    void setChorusPara(int e, int fb, double l, double r, double d, int type);

};
#endif // QMPMIDIOUTFLUID_H
