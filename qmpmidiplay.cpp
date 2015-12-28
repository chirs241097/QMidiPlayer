#include <cstdio>
#include <chrono>
#include <thread>
#include <fluidsynth.h>
#include "qmpmidiplay.hpp"
void CMidiPlayer::fluidInitialize(const char* sf)
{
	settings=new_fluid_settings();
	fluid_settings_setstr(settings,"audio.driver","pulseaudio");
	fluid_settings_setint(settings,"synth.cpu-cores",4);
	fluid_settings_setint(settings,"synth.min-note-length",0);
	fluid_settings_setint(settings,"synth.polyphony",256);
	synth=new_fluid_synth(settings);
	adriver=new_fluid_audio_driver(settings,synth);
	fluid_synth_sfload(synth,sf,1);
}
void CMidiPlayer::fluidDeinitialize()
{
	if(!synth||!adriver||!settings)return;
	delete_fluid_audio_driver(adriver);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
	synth=NULL;settings=NULL;adriver=NULL;
}
void CMidiPlayer::processEvent(const SEvent *e)
{
	switch(e->type&0xF0)
	{
		case 0x80://Note off
			fluid_synth_noteoff(synth,e->type&0x0F,e->p1);
		break;
		case 0x90://Note on
			if((mute>>(e->type&0x0F))&1)break;//muted
			if(solo&&!((solo>>(e->type&0x0F))&1))break;
			fluid_synth_noteon(synth,e->type&0x0F,e->p1,e->p2);
		break;
		case 0xB0://CC
			fluid_synth_cc(synth,e->type&0x0F,e->p1,e->p2);
		break;
		case 0xC0://PC
			fluid_synth_program_change(synth,e->type&0x0F,e->p1);
		break;
		case 0xE0://PW
			fluid_synth_pitch_bend(synth,e->type&0x0F,e->p1);
		break;
		case 0xF0://Meta/SysEx
			if((e->type&0x0F)==0x0F)
			{
				switch(e->p1)
				{
					case 0x51:
						ctempo=e->p2;dpt=ctempo*1000/divs;
					break;
					case 0x58:
					break;
					case 0x59:
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:
						if(e->str)puts(e->str);
					break;
				}
			}
		break;
	}
}
void CMidiPlayer::processEventStub(const SEvent *e)
{
	switch(e->type&0xF0)
	{
		case 0xB0://CC
			ccc[e->type&0x0F][e->p1]=e->p2;
		break;
		case 0xC0://PC
			ccc[e->type&0x0F][128]=e->p1;
		break;
		case 0xD0://CP
			ccc[e->type&0x0F][129]=e->p1;
		break;
		case 0xE0://PW
			ccc[e->type&0x0F][130]=e->p1;
		break;
		case 0xF0://Meta/SysEx
			if((e->type&0x0F)==0x0F)
			{
				switch(e->p1)
				{
					case 0x51:
						ctempo=e->p2;dpt=ctempo*1000/divs;
						ccc[0][131]=dpt;
					break;
				}
			}
		break;
	}
}
void CMidiPlayer::playEvents()
{
	for(uint32_t ct=midiFile->getEvent(0)->time;tceptr<midiFile->getEventCount();)
	{
		while(tcpaused)std::this_thread::sleep_for(std::chrono::milliseconds(100));
		while(!tcstop&&tceptr<midiFile->getEventCount()&&ct==midiFile->getEvent(tceptr)->time)
			processEvent(midiFile->getEvent(tceptr++));
		if(tcstop||tceptr>=midiFile->getEventCount())break;
		if(resumed)resumed=false;
		else
		std::this_thread::sleep_for(std::chrono::nanoseconds(midiFile->getEvent(tceptr)->time-ct)*dpt);
		ct=midiFile->getEvent(tceptr)->time;
	}
	while(!tcstop&&synth&&fluid_synth_get_active_voice_count(synth)>0)std::this_thread::sleep_for(std::chrono::microseconds(100));
	finished=1;
}
void CMidiPlayer::fileTimer1Pass()
{
	ftime=.0;ctempo=0x7A120;dpt=ctempo*1000/divs;
	for(uint32_t eptr=0,ct=midiFile->getEvent(0)->time;eptr<midiFile->getEventCount();)
	{
		while(eptr<midiFile->getEventCount()&&ct==midiFile->getEvent(eptr)->time)
			processEventStub(midiFile->getEvent(eptr++));
		if(eptr>=midiFile->getEventCount())break;
		ftime+=(midiFile->getEvent(eptr)->time-ct)*dpt/1e9;
		ct=midiFile->getEvent(eptr)->time;
	}
}
void CMidiPlayer::fileTimer2Pass()
{
	double ctime=.0;uint32_t c=1;ctempo=0x7A120;dpt=ctempo*1000/divs;
	memset(stamps,0,sizeof(stamps));memset(ccstamps,0,sizeof(ccstamps));
	memset(ccc,0,sizeof(ccc));for(int i=0;i<16;++i)
	{
		ccc[i][7]=100;ccc[i][10]=64;ccc[i][11]=127;
		ccc[i][11]=127;ccc[i][71]=64;ccc[i][72]=64;
		ccc[i][73]=64;ccc[i][74]=64;ccc[i][75]=64;
		ccc[i][76]=64;ccc[i][77]=64;ccc[i][78]=64;
		ccc[0][131]=dpt;
	}
	for(uint32_t eptr=0,ct=midiFile->getEvent(0)->time;eptr<midiFile->getEventCount();)
	{
		while(eptr<midiFile->getEventCount()&&ct==midiFile->getEvent(eptr)->time)
			processEventStub(midiFile->getEvent(eptr++));
		if(eptr>=midiFile->getEventCount())break;
		ctime+=(midiFile->getEvent(eptr)->time-ct)*dpt/1e9;
		while(ctime>ftime*c/100.)
		{
			for(int i=0;i<16;++i)for(int j=0;j<132;++j)
				ccstamps[c][i][j]=ccc[i][j];
			stamps[c++]=eptr;
			if(c>100)throw;
		}
		ct=midiFile->getEvent(eptr)->time;
	}
	while(c<101)
	{
		for(int i=0;i<16;++i)for(int j=0;j<132;++j)
			ccstamps[c][i][j]=ccc[i][j];
		stamps[c++]=midiFile->getEventCount();
	}
}
CMidiPlayer::CMidiPlayer()
{
	midiFile=NULL;resumed=false;
	settings=NULL;synth=NULL;adriver=NULL;
}
void CMidiPlayer::playerPanic()
{
	for(int i=0;i<16;++i)fluid_synth_all_notes_off(synth,i);
	//for(int i=0;i<16;++i)for(int j=0;j<128;++j)fluid_synth_noteoff(synth,i,j);
}
void CMidiPlayer::playerLoadFile(const char* fn)
{
	midiFile=new CMidiFile(fn);
	divs=midiFile->getDivision();
	fileTimer1Pass();
	fileTimer2Pass();
}
void CMidiPlayer::playerInit()
{
	ctempo=0x7A120;ctsn=4;ctsd=2;dpt=ctempo*1000/divs;
	tceptr=0;tcstop=0;tcpaused=0;finished=0;mute=solo=0;
	fluidInitialize("/media/Files/FluidR3_Ext.sf2");
}
void CMidiPlayer::playerDeinit()
{
	tceptr=0;tcstop=1;tcpaused=0;
	fluidDeinitialize();
}
void CMidiPlayer::playerThread()
{
	playEvents();
}
uint32_t CMidiPlayer::getStamp(int id){return stamps[id];}
uint32_t CMidiPlayer::getTCeptr(){return tceptr;}
void CMidiPlayer::setTCeptr(uint32_t ep,uint32_t st)
{
	if(ep==midiFile->getEventCount())tcstop=1;else tceptr=ep;
	for(int i=0;i<16;++i)
	{
		for(int j=0;j<120;++j)fluid_synth_cc(synth,i,j,ccstamps[st][i][j]);
		fluid_synth_program_change(synth,i,ccstamps[st][i][128]);
		//fluid_synth_pitch_bend(synth,i,ccstamps[st][i][130]);
		dpt=ccstamps[st][0][131];
	}
}
double CMidiPlayer::getFtime(){return ftime;}
uint32_t CMidiPlayer::getTCpaused(){return tcpaused;}
void CMidiPlayer::setTCpaused(uint32_t ps){tcpaused=ps;}
uint32_t CMidiPlayer::isFinished(){return finished;}
void CMidiPlayer::setResumed(){resumed=true;}
void CMidiPlayer::setGain(double gain){if(settings)fluid_settings_setnum(settings,"synth.gain",gain);}
int CMidiPlayer::getPolyphone(){return synth?fluid_synth_get_active_voice_count(synth):0;}
int CMidiPlayer::getMaxPolyphone(){return synth?fluid_synth_get_polyphony(synth):0;}
void CMidiPlayer::setMaxPolyphone(int p){if(synth)fluid_synth_set_polyphony(synth,p);}

void CMidiPlayer::getChannelPreset(int ch,int *b,int *p,char *name)
{
	if(!synth)return;
	fluid_synth_channel_info_t info;
	fluid_synth_get_channel_info(synth,ch,&info);
	*b=info.bank;*p=info.program;
	strcpy(name,info.name);
}
//16MSB..LSB1
void CMidiPlayer::setBit(uint16_t &n, uint16_t bn, uint16_t b)
{n^=(-b^n)&(1<<bn);}
void CMidiPlayer::setMute(int ch,bool m)
{
	setBit(mute,ch,m?1:0);
}
void CMidiPlayer::setSolo(int ch,bool s)
{
	setBit(solo,ch,s?1:0);
}
