#include <cmath>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include "qmpmidioutfluid.hpp"
qmpMidiOutFluid::qmpMidiOutFluid()
{
    settings = new_fluid_settings();
    synth = nullptr;
    adriver = nullptr;
}
qmpMidiOutFluid::~qmpMidiOutFluid()
{
    delete_fluid_settings(settings);
    settings = nullptr;
}

void qmpMidiOutFluid::registerOptions(qmpPluginAPI *coreapi)
{
    default_driver = -1;
    fluid_settings_t *fsettings = new_fluid_settings();
    auto insert_driver = [](void *d, const char *, const char *driver)->void
    {
        qmpMidiOutFluid *me = static_cast<qmpMidiOutFluid *>(d);
        me->drivers.push_back(std::string(driver));
#ifdef WIN32
        if (std::string(driver) == "waveout")
#else
        if (std::string(driver) == "pulseaudio")
            me->default_driver = static_cast<int>(me->drivers.size() - 1);
#endif
        };
    fluid_settings_foreach_option(fsettings, "audio.driver", this, insert_driver);
    delete_fluid_settings(fsettings);
    coreapi->registerOptionEnumInt("Audio", "Audio Driver", "FluidSynth/AudioDriver", drivers, default_driver);
    coreapi->registerOptionInt("Audio", "Audio Buffer Size", "FluidSynth/BufSize", 64, 8192,
#ifdef WIN32
        512
#else
        64
#endif
    );
    coreapi->registerOptionInt("Audio", "Audio Buffer Count", "FluidSynth/BufCnt", 2, 64,
#ifdef WIN32
        8
#else
        16
#endif
    );
    coreapi->registerOptionEnumInt("Audio", "Sample Format", "FluidSynth/SampleFormat", {"16bits", "float"}, 0);
    coreapi->registerOptionInt("Audio", "Sample Rate", "FluidSynth/SampleRate", 8000, 96000, 48000);
    coreapi->registerOptionInt("Audio", "Max Polyphony", "FluidSynth/Polyphony", 1, 65535, 256);
    coreapi->registerOptionInt("Audio", "CPU Cores", "FluidSynth/Threads", 1, 256, 1);
    coreapi->registerOptionBool("Audio", "Auto Bank Select Mode", "FluidSynth/AutoBS", true);
    coreapi->registerOptionEnumInt("Audio", "Bank Select Mode", "FluidSynth/BankSelect", {"GM", "GS", "XG", "MMA"}, 1);
}
void qmpMidiOutFluid::deviceInit()
{
    synth = new_fluid_synth(settings);
    if (!synth)
    {
        fputs("Error creating fluidsynth instance!", stderr);
        return;
    }
    fluid_set_log_function(FLUID_DBG, nullptr, nullptr);
    fluid_set_log_function(FLUID_INFO, nullptr, nullptr);
    fluid_set_log_function(FLUID_WARN, nullptr, nullptr);
    fluid_set_log_function(FLUID_ERR, fluid_default_log_function, nullptr);
    fluid_set_log_function(FLUID_PANIC, fluid_default_log_function, nullptr);
    adriver = new_fluid_audio_driver2(settings,
        [](void *t, int l, int nfx, float *fx[], int nout, float *out[])->int
        {
            qmpMidiOutFluid *self = static_cast<qmpMidiOutFluid*>(t);
            float *fxb[4] = {out[0], out[1], out[0], out[1]};
            int ret = fluid_synth_process(self->synth, l, nout * 2, fxb, nout, out);
            double s = 0;
            for (int i = 0; i < nout; ++i)
            {
                for (int j = 0; j < l; ++j)
                {
                    s += out[i][j] * out[i][j] / l;
                }
            }
            self->output_level = 20 * log10(sqrt(s));
            self->voice_count = fluid_synth_get_active_voice_count(self->synth);
            return ret;
        }
    , this);
    if (!adriver)
    {
        adriver = new_fluid_audio_driver(settings, synth);
        output_level = 1e9 + 7;
        voice_count = -1;
    }
    if (!adriver)
    {
        fputs("Error creating fluidsynth audio driver!", stderr);
        delete_fluid_synth(synth);
        synth = nullptr;
        return;
    }
    fluid_synth_set_chorus(synth, 3, 2.0, 0.3, 8.0, FLUID_CHORUS_MOD_SINE);
    bnk.clear();
    pst.clear();
}
void qmpMidiOutFluid::deviceDeinit()
{
    deviceDeinit(false);
}
void qmpMidiOutFluid::deviceDeinit(bool freshsettings)
{
    if (!synth || !adriver)
        return;
    delete_fluid_audio_driver(adriver);
    delete_fluid_synth(synth);
    synth = nullptr;
    adriver = nullptr;
    bnk.clear();
    pst.clear();
    if (freshsettings)
    {
        delete_fluid_settings(settings);
        settings = new_fluid_settings();
    }
}
void qmpMidiOutFluid::basicMessage(uint8_t type, uint8_t p1, uint8_t p2)
{
    uint8_t chan = type & 0x0F;
    switch (type & 0xF0)
    {
        case 0x80:
            fluid_synth_noteoff(synth, chan, p1);
            break;
        case 0x90:
            if (p2)
                fluid_synth_noteon(synth, chan, p1, p2);
            else
                fluid_synth_noteoff(synth, chan, p1);
            break;
        case 0xB0:
            fluid_synth_cc(synth, chan, p1, p2);
            break;
        case 0xC0:
            fluid_synth_program_change(synth, chan, p1);
            break;
        case 0xE0:
            fluid_synth_pitch_bend(synth, chan, (p1 | p2 << 7) & 0x3fff);
            break;
    }
}
void qmpMidiOutFluid::extendedMessage(uint32_t length, const char *data)
{
    int rlen = 0;
    fluid_synth_sysex(synth, data + 1, int(length) - 2, nullptr, &rlen, nullptr, 0);
}
void qmpMidiOutFluid::rpnMessage(uint8_t ch, uint16_t type, uint16_t val)
{
    if (type == 0)
        fluid_synth_pitch_wheel_sens(synth, ch, val >> 7);
    else
    {
        fluid_synth_cc(synth, ch, 0x64, type & 0x7F);
        fluid_synth_cc(synth, ch, 0x65, type >> 7);
        fluid_synth_cc(synth, ch, 0x06, val >> 7);
        fluid_synth_cc(synth, ch, 0x26, val & 0x7F);
    }
}
void qmpMidiOutFluid::nrpnMessage(uint8_t ch, uint16_t type, uint16_t val)
{
    fluid_synth_cc(synth, ch, 0x62, type & 0x7F);
    fluid_synth_cc(synth, ch, 0x63, type >> 7);
    fluid_synth_cc(synth, ch, 0x06, val >> 7);
    fluid_synth_cc(synth, ch, 0x26, val & 0x7F);
}
void qmpMidiOutFluid::panic(uint8_t ch)
{
    fluid_synth_cc(synth, ch, 64, 0);
    fluid_synth_pitch_bend(synth, ch, 8192);
    fluid_synth_all_notes_off(synth, ch);
}
void qmpMidiOutFluid::reset(uint8_t ch)
{
    if (!~ch)
    {
        fluid_synth_system_reset(synth);
        return;
    }
    this->panic(ch);

    for (int i = 0; i < 128; ++i)
        fluid_synth_cc(synth, ch, i, 0);

    if (ch == 9)
        fluid_synth_cc(synth, ch, 0, 127);
    else
        fluid_synth_cc(synth, ch, 0, 0);

    fluid_synth_cc(synth, ch, 7, 100);
    fluid_synth_cc(synth, ch, 8, 64);
    fluid_synth_cc(synth, ch, 10, 64);
    fluid_synth_cc(synth, ch, 11, 127);
    fluid_synth_pitch_wheel_sens(synth, ch, 2);
}
void qmpMidiOutFluid::onMapped(uint8_t, int)
{
}
void qmpMidiOutFluid::onUnmapped(uint8_t, int)
{
}
std::vector<std::pair<uint16_t, std::string>> qmpMidiOutFluid::getBankList()
{
    return bnk;
}
std::vector<std::pair<uint8_t, std::string>> qmpMidiOutFluid::getPresets(uint16_t bank)
{
    std::vector<std::pair<uint8_t, std::string>> ret;
    if (pst.find(bank) == pst.end())
        return ret;
    for (uint8_t i = 0; i < 128; ++i)
    {
        if (pst[bank][i].length())
            ret.emplace_back(i, pst[bank][i]);
    }
    return ret;
}
std::string qmpMidiOutFluid::getPresetName(uint16_t bank, uint8_t preset)
{
    if (pst.find(bank) == pst.end())
        return "";
    //should consult fluidsynth instead? (4 bank mapping methods)
    return pst[bank][preset];
}
bool qmpMidiOutFluid::getChannelPreset(int ch, uint16_t *bank, uint8_t *preset, std::string &presetname)
{
    if (!synth)
        return false;
    fluid_preset_t *chpreset = fluid_synth_get_channel_preset(synth, ch);
    if (!chpreset)
    {
        *bank = *preset = -1;
        presetname = "---";
        return true;
    }
    *bank = fluid_preset_get_banknum(chpreset);
    *preset = fluid_preset_get_num(chpreset);
    presetname = fluid_preset_get_name(chpreset);
    return true;
}
uint8_t qmpMidiOutFluid::getInitialCCValue(uint8_t cc, uint8_t)
{
    switch (cc)
    {
        case 11:
            return 127;
        case 7:
            return 100;
        case 8:
        case 10:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
            return 64;
        case 129:
            return 2;
        default:
            return 0;
    }
}
void qmpMidiOutFluid::setOptStr(const char *opt, const char *val)
{
    fluid_settings_setstr(settings, opt, val);
}
void qmpMidiOutFluid::setOptInt(const char *opt, int val)
{
    fluid_settings_setint(settings, opt, val);
}
void qmpMidiOutFluid::setOptNum(const char *opt, double val)
{
    fluid_settings_setnum(settings, opt, val);
}
void qmpMidiOutFluid::loadSFont(const char *path)
{
    if (synth)
        fluid_synth_sfload(synth, path, 1);
    update_preset_list();
}
int qmpMidiOutFluid::getSFCount()
{
    return synth ? fluid_synth_sfcount(synth) : 0;
}
void qmpMidiOutFluid::update_preset_list()
{
    bnk.clear();
    pst.clear();
    for (int i = getSFCount() - 1; i >= 0; --i)
    {
        fluid_sfont_t *psf = fluid_synth_get_sfont(synth, i);
        fluid_preset_t *preset;
        fluid_sfont_iteration_start(psf);
        while ((preset = fluid_sfont_iteration_next(psf)))
        {
            uint16_t b = fluid_preset_get_banknum(preset);
            uint8_t p = fluid_preset_get_num(preset);
            if (bnk.empty() || bnk.back().first != b)
                bnk.emplace_back(b, "");
            if (pst[b].empty())
                pst[b].resize(128);
            pst[b][p] = std::string(fluid_preset_get_name(preset));
            if (!pst[b][p].length())
                pst[b][p] = " ";
        }
    }
    std::sort(bnk.begin(), bnk.end());
    bnk.erase(std::unique(bnk.begin(), bnk.end()), bnk.end());
}
int qmpMidiOutFluid::getPolyphone()
{
    if (~voice_count)
    {
        return voice_count;
    }
    return synth ? fluid_synth_get_active_voice_count(synth) : 0;
}
int qmpMidiOutFluid::getMaxPolyphone()
{
    return synth ? fluid_synth_get_polyphony(synth) : 0;
}

double qmpMidiOutFluid::getOutputLevel()
{
    return output_level;
}
void qmpMidiOutFluid::setGain(double gain)
{
    if (settings)
        fluid_settings_setnum(settings, "synth.gain", gain);
}
void qmpMidiOutFluid::getChannelInfo(int ch, int *b, int *p, char *s)
{
    if (!synth)
        return;
    fluid_preset_t *chpreset = fluid_synth_get_channel_preset(synth, ch);
    if (!chpreset)
    {
        *b = *p = -1;
        strcpy(s, "---");
        return;
    }
    *b = fluid_preset_get_banknum(chpreset);
    *p = fluid_preset_get_num(chpreset);
    strncpy(s, fluid_preset_get_name(chpreset), 256);
}
void qmpMidiOutFluid::getReverbPara(double *r, double *d, double *w, double *l)
{
    if (!synth)
        return;
    *r = fluid_synth_get_reverb_roomsize(synth);
    *d = fluid_synth_get_reverb_damp(synth);
    *w = fluid_synth_get_reverb_width(synth);
    *l = fluid_synth_get_reverb_level(synth);
}
void qmpMidiOutFluid::setReverbPara(int e, double r, double d, double w, double l)
{
    if (!synth)
        return;
    fluid_synth_set_reverb_on(synth, e);
    fluid_synth_set_reverb(synth, r, d, w, l);
}
void qmpMidiOutFluid::getChorusPara(int *fb, double *l, double *r, double *d, int *type)
{
    if (!synth)
        return;
    *fb = fluid_synth_get_chorus_nr(synth);
    *l = fluid_synth_get_chorus_level(synth);
    *r = fluid_synth_get_chorus_speed(synth);
    *d = fluid_synth_get_chorus_depth(synth);
    *type = fluid_synth_get_chorus_type(synth);
}
void qmpMidiOutFluid::setChorusPara(int e, int fb, double l, double r, double d, int type)
{
    if (!synth)
        return;
    fluid_synth_set_chorus_on(synth, e);
    fluid_synth_set_chorus(synth, fb, l, r, d, type);
}

qmpFileRendererFluid::qmpFileRendererFluid(const char *_fn, const char *_ofn)
{
    settings = new_fluid_settings();
    fluid_settings_setstr(settings, "audio.file.name", _ofn);
    fn = std::string(_fn);
    finished = false;
}
qmpFileRendererFluid::~qmpFileRendererFluid()
{
    if (player || synth || settings)
        renderDeinit();
}
void qmpFileRendererFluid::renderInit()
{
    synth = new_fluid_synth(settings);
    player = new_fluid_player(synth);
    fluid_player_add(player, fn.c_str());
}
void qmpFileRendererFluid::renderDeinit()
{
    delete_fluid_player(player);
    delete_fluid_synth(synth);
    delete_fluid_settings(settings);
    player = nullptr;
    synth = nullptr;
    settings = nullptr;
}
void qmpFileRendererFluid::renderWorker()
{
    fluid_file_renderer_t *renderer = new_fluid_file_renderer(synth);
    fluid_player_play(player);
    while (fluid_player_get_status(player) == FLUID_PLAYER_PLAYING)
        if (fluid_file_renderer_process_block(renderer) != FLUID_OK)
            break;
    delete_fluid_file_renderer(renderer);
    finished = true;
}
void qmpFileRendererFluid::setOptStr(const char *opt, const char *val)
{
    fluid_settings_setstr(settings, opt, val);
}
void qmpFileRendererFluid::setOptInt(const char *opt, int val)
{
    fluid_settings_setint(settings, opt, val);
}
void qmpFileRendererFluid::setOptNum(const char *opt, double val)
{
    fluid_settings_setnum(settings, opt, val);
}
void qmpFileRendererFluid::loadSFont(const char *path)
{
    if (synth)
        fluid_synth_sfload(synth, path, 1);
}
bool qmpFileRendererFluid::isFinished()
{
    return finished;
}
void qmpFileRendererFluid::setGain(double gain)
{
    if (settings)
        fluid_settings_setnum(settings, "synth.gain", gain);
}
void qmpFileRendererFluid::setReverbPara(int e, double r, double d, double w, double l)
{
    if (!synth)
        return;
    fluid_synth_set_reverb_on(synth, e);
    fluid_synth_set_reverb(synth, r, d, w, l);
}
void qmpFileRendererFluid::setChorusPara(int e, int fb, double l, double r, double d, int type)
{
    if (!synth)
        return;
    fluid_synth_set_chorus_on(synth, e);
    fluid_synth_set_chorus(synth, fb, l, r, d, type);
}
