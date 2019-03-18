#include <cstdio>
#include <cstring>
#include <map>
#include "qmpmidioutfluid.hpp"
qmpMidiOutFluid::qmpMidiOutFluid()
{
	settings=new_fluid_settings();
	synth=NULL;adriver=NULL;
}
qmpMidiOutFluid::~qmpMidiOutFluid()
{
	delete_fluid_settings(settings);
	settings=NULL;
}
void qmpMidiOutFluid::deviceInit()
{
	synth=new_fluid_synth(settings);
	if(!synth){fputs("Error creating fluidsynth instance!",stderr);return;}
	fluid_set_log_function(FLUID_DBG,NULL,NULL);
	fluid_set_log_function(FLUID_INFO,NULL,NULL);
	fluid_set_log_function(FLUID_WARN,NULL,NULL);
	fluid_set_log_function(FLUID_ERR,fluid_default_log_function,NULL);
	fluid_set_log_function(FLUID_PANIC,fluid_default_log_function,NULL);
	adriver=new_fluid_audio_driver(settings,synth);
	if(!adriver)
	{
		fputs("Error creating fluidsynth audio driver!",stderr);
		delete_fluid_synth(synth);synth=NULL;
		return;
	}
	fluid_synth_set_chorus(synth,3,2.0,0.3,8.0,
						   FLUID_CHORUS_MOD_SINE);
}
void qmpMidiOutFluid::deviceDeinit(){deviceDeinit(false);}
void qmpMidiOutFluid::deviceDeinit(bool freshsettings)
{
	if(!synth||!adriver)return;
	delete_fluid_audio_driver(adriver);
	delete_fluid_synth(synth);
	synth=NULL;adriver=NULL;
	if(freshsettings)
	{
		delete_fluid_settings(settings);
		settings=new_fluid_settings();
	}
}
void qmpMidiOutFluid::basicMessage(uint8_t type,uint8_t p1,uint8_t p2)
{
	uint8_t chan=type&0x0F;
	switch(type&0xF0)
	{
		case 0x80:
			fluid_synth_noteoff(synth,chan,p1);
		break;
		case 0x90:
			if(p2)fluid_synth_noteon(synth,chan,p1,p2);
			else fluid_synth_noteoff(synth,chan,p1);
		break;
		case 0xB0:
			fluid_synth_cc(synth,chan,p1,p2);
		break;
		case 0xC0:
			fluid_synth_program_change(synth,chan,p1);
		break;
		case 0xE0:
			fluid_synth_pitch_bend(synth,chan,(p1|p2<<7)&0x3fff);
		break;
	}
}
void qmpMidiOutFluid::extendedMessage(uint8_t length,const char *data)
{
	int rlen=0;
	fluid_synth_sysex(synth,data,length,NULL,&rlen,NULL,0);
}
void qmpMidiOutFluid::rpnMessage(uint8_t ch,uint16_t type,uint16_t val)
{
	if(type==0)fluid_synth_pitch_wheel_sens(synth,ch,val>>7);
	else
	{
		fluid_synth_cc(synth,ch,0x64,type&0x7F);
		fluid_synth_cc(synth,ch,0x65,type>>7);
		fluid_synth_cc(synth,ch,0x06,val>>7);
		fluid_synth_cc(synth,ch,0x26,val&0x7F);
	}
}
void qmpMidiOutFluid::nrpnMessage(uint8_t ch,uint16_t type,uint16_t val)
{
	fluid_synth_cc(synth,ch,0x62,type&0x7F);
	fluid_synth_cc(synth,ch,0x63,type>>7);
	fluid_synth_cc(synth,ch,0x06,val>>7);
	fluid_synth_cc(synth,ch,0x26,val&0x7F);
}
void qmpMidiOutFluid::panic(uint8_t ch)
{
	fluid_synth_cc(synth,ch,64,0);
	fluid_synth_pitch_bend(synth,ch,8192);
	fluid_synth_all_notes_off(synth,ch);
}
void qmpMidiOutFluid::reset(uint8_t ch)
{
	this->panic(ch);
	for(int i=0;i<128;++i)
		fluid_synth_cc(synth,ch,i,0);
	if(ch==9)
		fluid_synth_cc(synth,ch,0,127);
	else
		fluid_synth_cc(synth,ch,0,0);
	fluid_synth_cc(synth,ch,7,100);
	fluid_synth_cc(synth,ch,8,64);
	fluid_synth_cc(synth,ch,10,64);
	fluid_synth_cc(synth,ch,11,127);
	fluid_synth_pitch_wheel_sens(synth,ch,2);
}
void qmpMidiOutFluid::onMapped(uint8_t,int)
{
}
void qmpMidiOutFluid::onUnmapped(uint8_t,int)
{
}
void qmpMidiOutFluid::setOptStr(const char *opt,const char *val)
{
	fluid_settings_setstr(settings,opt,val);
}
void qmpMidiOutFluid::setOptInt(const char *opt,int val)
{
	fluid_settings_setint(settings,opt,val);
}
void qmpMidiOutFluid::setOptNum(const char *opt,double val)
{
	fluid_settings_setnum(settings,opt,val);
}
void qmpMidiOutFluid::loadSFont(const char *path)
{
	if(synth)fluid_synth_sfload(synth,path,1);
}
int qmpMidiOutFluid::getSFCount()
{
	return synth?fluid_synth_sfcount(synth):0;
}
std::vector<std::pair<std::pair<int,int>,std::string>> qmpMidiOutFluid::listPresets()
{
	std::vector<std::pair<std::pair<int,int>,std::string>> ret;
	std::map<std::pair<int,int>,std::string> pmap;
	for(int i=getSFCount()-1;i>=0;--i)
	{
		fluid_sfont_t* psf=fluid_synth_get_sfont(synth,i);
		fluid_preset_t* preset;
		fluid_sfont_iteration_start(psf);
		while(preset=fluid_sfont_iteration_next(psf))
			pmap[std::make_pair(
					fluid_preset_get_banknum(preset),
					fluid_preset_get_num(preset)
			)]=fluid_preset_get_name(preset);
	}
	for(auto i=pmap.begin();i!=pmap.end();++i)
	ret.push_back(std::make_pair(i->first,i->second));
	return ret;
}
int qmpMidiOutFluid::getPolyphone()
{
	return synth?fluid_synth_get_active_voice_count(synth):0;
}
int qmpMidiOutFluid::getMaxPolyphone()
{
	return synth?fluid_synth_get_polyphony(synth):0;
}
void qmpMidiOutFluid::setGain(double gain)
{
	if(settings)fluid_settings_setnum(settings,"synth.gain",gain);
}
void qmpMidiOutFluid::getChannelInfo(int ch,int *b,int *p,char *s)
{
	if(!synth)return;
	fluid_preset_t* chpreset=fluid_synth_get_channel_preset(synth,ch);
	if(!chpreset)
	{
		*b=*p=-1;
		strcpy(s,"---");
		return;
	}
	*b=fluid_preset_get_banknum(chpreset);
	*p=fluid_preset_get_num(chpreset);
	strncpy(s,fluid_preset_get_name(chpreset),256);
}
void qmpMidiOutFluid::getReverbPara(double *r,double *d,double *w,double *l)
{
	if(!synth)return;
	*r=fluid_synth_get_reverb_roomsize(synth);
	*d=fluid_synth_get_reverb_damp(synth);
	*w=fluid_synth_get_reverb_width(synth);
	*l=fluid_synth_get_reverb_level(synth);
}
void qmpMidiOutFluid::setReverbPara(int e,double r,double d,double w,double l)
{
	if(!synth)return;
	fluid_synth_set_reverb_on(synth,e);
	fluid_synth_set_reverb(synth,r,d,w,l);
}
void qmpMidiOutFluid::getChorusPara(int *fb,double *l,double *r,double *d,int *type)
{
	if(!synth)return;
	*fb=fluid_synth_get_chorus_nr(synth);
	*l=fluid_synth_get_chorus_level(synth);
	*r=fluid_synth_get_chorus_speed(synth);
	*d=fluid_synth_get_chorus_depth(synth);
	*type=fluid_synth_get_chorus_type(synth);
}
void qmpMidiOutFluid::setChorusPara(int e,int fb,double l,double r,double d,int type)
{
	if(!synth)return;
	fluid_synth_set_chorus_on(synth,e);
	fluid_synth_set_chorus(synth,fb,l,r,d,type);
}

qmpFileRendererFluid::qmpFileRendererFluid(const char *_fn,const char *_ofn)
{
	settings=new_fluid_settings();
	fluid_settings_setstr(settings,"audio.file.name",_ofn);
	fn=std::string(_fn);
	finished=false;
}
qmpFileRendererFluid::~qmpFileRendererFluid()
{
	if(player||synth||settings)renderDeinit();
}
void qmpFileRendererFluid::renderInit()
{
	synth=new_fluid_synth(settings);
	player=new_fluid_player(synth);
	fluid_player_add(player,fn.c_str());
}
void qmpFileRendererFluid::renderDeinit()
{
	delete_fluid_player(player);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
	player=NULL;synth=NULL;settings=NULL;
}
void qmpFileRendererFluid::renderWorker()
{
	fluid_file_renderer_t* renderer=new_fluid_file_renderer(synth);
	fluid_player_play(player);
	while(fluid_player_get_status(player)==FLUID_PLAYER_PLAYING)
		if(fluid_file_renderer_process_block(renderer)!=FLUID_OK)break;
	delete_fluid_file_renderer(renderer);
	finished=true;
}
void qmpFileRendererFluid::setOptStr(const char *opt,const char *val)
{
	fluid_settings_setstr(settings,opt,val);
}
void qmpFileRendererFluid::setOptInt(const char *opt,int val)
{
	fluid_settings_setint(settings,opt,val);
}
void qmpFileRendererFluid::setOptNum(const char *opt,double val)
{
	fluid_settings_setnum(settings,opt,val);
}
void qmpFileRendererFluid::loadSFont(const char *path)
{
	if(synth)fluid_synth_sfload(synth,path,1);
}
bool qmpFileRendererFluid::isFinished()
{
	return finished;
}
void qmpFileRendererFluid::setGain(double gain)
{
	if(settings)fluid_settings_setnum(settings,"synth.gain",gain);
}
void qmpFileRendererFluid::setReverbPara(int e,double r,double d,double w,double l)
{
	if(!synth)return;
	fluid_synth_set_reverb_on(synth,e);
	fluid_synth_set_reverb(synth,r,d,w,l);
}
void qmpFileRendererFluid::setChorusPara(int e,int fb,double l,double r,double d,int type)
{
	if(!synth)return;
	fluid_synth_set_chorus_on(synth,e);
	fluid_synth_set_chorus(synth,fb,l,r,d,type);
}
