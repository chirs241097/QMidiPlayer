#include <cstdio>
#include <chrono>
#include <thread>
#include <fluidsynth.h>
#include "qmpmidiplay.hpp"
#ifdef _WIN32
#include <windows.h>
uint64_t pf;
#endif
void CMidiPlayer::fluidPreInitialize()
{
	settings=new_fluid_settings();
}
void CMidiPlayer::fluidInitialize()
{
	synth=new_fluid_synth(settings);
	adriver=new_fluid_audio_driver(settings,synth);
	fluid_synth_set_chorus(synth,FLUID_CHORUS_DEFAULT_N,FLUID_CHORUS_DEFAULT_LEVEL,
						   FLUID_CHORUS_DEFAULT_SPEED,FLUID_CHORUS_DEFAULT_DEPTH,
						   FLUID_CHORUS_DEFAULT_TYPE);
#ifndef _WIN32
	if(!singleInstance)
	{
		if(midiFile->getStandard()==4)
		fluid_synth_set_channel_type(synth,9,CHANNEL_TYPE_MELODIC);
		else if(midiFile->getStandard()==1)
			fluid_synth_set_channel_type(synth,9,CHANNEL_TYPE_DRUM);
		else
		{
			fluid_synth_set_channel_type(synth,9,CHANNEL_TYPE_DRUM);
			fluid_synth_bank_select(synth,9,128);
		}
	}
#endif
}
void CMidiPlayer::fluidDeinitialize()
{
	if(!synth||!adriver||!settings)return;
	delete_fluid_settings(settings);
	delete_fluid_audio_driver(adriver);
	delete_fluid_synth(synth);
	settings=NULL;synth=NULL;adriver=NULL;
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
			if(e->p1==100)rpnid=e->p2;
			if(e->p1==6)rpnval=e->p2;
			if(~rpnid&&~rpnval)
			{
				if(rpnid==0)fluid_synth_pitch_wheel_sens(synth,e->type&0x0F,rpnval);
				rpnid=rpnval=-1;
			}
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
						ctsn=e->p2>>24;
						ctsd=1<<((e->p2>>16)&0xFF);
					break;
					case 0x59:
						cks=e->p2;
					break;
					case 0x01:case 0x02:case 0x03:
					case 0x04:case 0x05:case 0x06:
					case 0x07:
						//if(e->str)puts(e->str);
					break;
				}
			}
			if((e->type&0x0F)==0x00||(e->type&0x0F)==07)
			{
				int io=0;
				if(sendSysEx)fluid_synth_sysex(synth,e->str,e->p1,NULL,&io,NULL,0);
			}
		break;
	}
}
void CMidiPlayer::processEventStub(const SEvent *e)
{
	switch(e->type&0xF0)
	{
		case 0xB0://CC
			if(e->p1==100)rpnid=e->p2;
			if(e->p1==6)rpnval=e->p2;
			if(~rpnid&&~rpnval)
			{
				if(rpnid==0)ccc[e->type&0x0F][134]=rpnval;
				rpnid=rpnval=-1;
			}
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
					case 0x58:
						ccc[0][132]=e->p2;
					break;
					case 0x59:
						ccc[0][133]=e->p2;
					break;
				}
			}
		break;
	}
}
#ifdef _WIN32
void w32usleep(uint64_t t)
{
	uint64_t st=0,ct=0;
	timeBeginPeriod(1);
	QueryPerformanceCounter((LARGE_INTEGER*)&st);
	do{
		if(t>10000+(ct-st)*1000000/pf)Sleep((t-(ct-st)*1000000/pf)/2000);
		else if(t>5000+(ct-st)*1000000/pf)Sleep(1);
		else std::this_thread::yield();
		QueryPerformanceCounter((LARGE_INTEGER*)&ct);
	}while((ct-st)*1000000<t*pf);
	timeEndPeriod(1);
}
#endif
void CMidiPlayer::playEvents()
{
	for(uint32_t ct=midiFile->getEvent(0)->time;tceptr<midiFile->getEventCount();)
	{
		while(tcpaused)std::this_thread::sleep_for(std::chrono::milliseconds(100));
		while(!tcstop&&midiFile&&tceptr<midiFile->getEventCount()&&ct==midiFile->getEvent(tceptr)->time)
			processEvent(midiFile->getEvent(tceptr++));
		if(tcstop||!midiFile||tceptr>=midiFile->getEventCount())break;
		if(resumed)resumed=false;
		else
#if 0
		w32usleep((midiFile->getEvent(tceptr)->time-ct)*(dpt/1000));
#else
		std::this_thread::sleep_for(std::chrono::nanoseconds(midiFile->getEvent(tceptr)->time-ct)*dpt);
#endif
		if(tcstop||!midiFile)break;
		ct=midiFile->getEvent(tceptr)->time;
	}
	while(!tcstop&&synth&&(waitvoice&&fluid_synth_get_active_voice_count(synth)>0))std::this_thread::sleep_for(std::chrono::milliseconds(2));
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
	memset(ccc,0,sizeof(ccc));rpnid=rpnval=-1;for(int i=0;i<16;++i)
	{
		ccc[i][7]=100;ccc[i][10]=64;ccc[i][11]=127;
		ccc[i][11]=127;ccc[i][71]=64;ccc[i][72]=64;
		ccc[i][73]=64;ccc[i][74]=64;ccc[i][75]=64;
		ccc[i][76]=64;ccc[i][77]=64;ccc[i][78]=64;
		ccc[0][131]=dpt;ccc[0][132]=0x04021808;
		ccc[0][133]=0;ccc[0][134]=2;
	}if(midiFile->getStandard()!=4)ccc[9][0]=128;
	for(int i=0;i<16;++i)for(int j=0;j<135;++j)
		ccstamps[0][i][j]=ccc[i][j];
	for(uint32_t eptr=0,ct=midiFile->getEvent(0)->time;eptr<midiFile->getEventCount();)
	{
		while(eptr<midiFile->getEventCount()&&ct==midiFile->getEvent(eptr)->time)
			processEventStub(midiFile->getEvent(eptr++));
		if(eptr>=midiFile->getEventCount())break;
		ctime+=(midiFile->getEvent(eptr)->time-ct)*dpt/1e9;
		while(ctime>ftime*c/100.)
		{
			for(int i=0;i<16;++i)for(int j=0;j<135;++j)
				ccstamps[c][i][j]=ccc[i][j];
			stamps[c++]=eptr;
			if(c>100)break;
		}
		ct=midiFile->getEvent(eptr)->time;
	}
	while(c<101)
	{
		for(int i=0;i<16;++i)for(int j=0;j<135;++j)
			ccstamps[c][i][j]=ccc[i][j];
		stamps[c++]=midiFile->getEventCount();
	}
}
CMidiPlayer::CMidiPlayer(bool singleInst)
{
	midiFile=NULL;resumed=false;singleInstance=singleInst;
	settings=NULL;synth=NULL;adriver=NULL;waitvoice=true;
#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&pf);
#endif
}
CMidiPlayer::~CMidiPlayer()
{
	if(singleInstance)fluidDeinitialize();
}
void CMidiPlayer::playerPanic(bool reset)
{
	if(reset)for(int i=0;i<16;++i)
	{
		fluid_synth_pitch_bend(synth,i,8192);
		fluid_synth_cc(synth,i,7,100);
		fluid_synth_cc(synth,i,10,64);
		fluid_synth_cc(synth,i,11,127);
	}
	for(int i=0;i<16;++i)fluid_synth_all_notes_off(synth,i);
}
bool CMidiPlayer::playerLoadFile(const char* fn)
{
	midiFile=new CMidiFile(fn);
	if(!midiFile->isValid())return false;
	divs=midiFile->getDivision();
	fileTimer1Pass();
	fileTimer2Pass();
	return true;
}
void CMidiPlayer::playerInit()
{
	ctempo=0x7A120;ctsn=4;ctsd=4;cks=0;dpt=ctempo*1000/divs;
	tceptr=0;tcstop=0;tcpaused=0;finished=0;mute=solo=0;
	sendSysEx=true;rpnid=rpnval=-1;
	if(!singleInstance)fluidPreInitialize();
}
void CMidiPlayer::playerDeinit()
{
	tceptr=0;tcstop=1;tcpaused=0;
	delete midiFile;midiFile=NULL;
	if(!singleInstance)fluidDeinitialize();
}
void CMidiPlayer::playerThread()
{
	playEvents();
}

void CMidiPlayer::rendererLoadFile(const char* ofn)
{
	settings=new_fluid_settings();
	fluid_settings_setstr(settings,"audio.file.name",ofn);
}
void CMidiPlayer::rendererInit(const char* fn)
{
	finished=0;
	synth=new_fluid_synth(settings);
	player=new_fluid_player(synth);
	fluid_player_add(player,fn);
}
void CMidiPlayer::rendererThread()
{
	fluid_file_renderer_t* renderer=new_fluid_file_renderer(synth);
	fluid_player_play(player);
	while(fluid_player_get_status(player)==FLUID_PLAYER_PLAYING)
		if(fluid_file_renderer_process_block(renderer)!=FLUID_OK)break;
	delete_fluid_file_renderer(renderer);
	finished=1;
}
void CMidiPlayer::rendererDeinit()
{
	delete_fluid_player(player);
	delete_fluid_synth(synth);
	delete_fluid_settings(settings);
	player=NULL;synth=NULL;settings=NULL;
}

void CMidiPlayer::sendSysX(bool send){sendSysEx=send;}
uint32_t CMidiPlayer::getStamp(int id){return stamps[id];}
uint32_t CMidiPlayer::getTCeptr(){return tceptr;}
void CMidiPlayer::setTCeptr(uint32_t ep,uint32_t st)
{
	resumed=true;
	if(ep==midiFile->getEventCount())tcstop=1;else tceptr=ep;
	for(int i=0;i<16;++i)
	{
		for(int j=0;j<120;++j)fluid_synth_cc(synth,i,j,ccstamps[st][i][j]);
		fluid_synth_program_change(synth,i,ccstamps[st][i][128]);
		//fluid_synth_pitch_bend(synth,i,ccstamps[st][i][130]);
		dpt=ccstamps[st][0][131];ctempo=dpt*divs/1000;
		ctsn=ccstamps[st][0][132]>>24;ctsd=1<<((ccstamps[st][0][132]>>16)&0xFF);
		cks=ccstamps[st][0][133];
	}
}
double CMidiPlayer::getFtime(){return ftime;}
void CMidiPlayer::getCurrentTimeSignature(int *n,int *d){*n=ctsn;*d=ctsd;}
void CMidiPlayer::getCurrentKeySignature(int *ks){*ks=cks;}
uint32_t CMidiPlayer::getFileNoteCount(){return midiFile?midiFile->getNoteCount():0;}
uint32_t CMidiPlayer::getFileStandard(){return midiFile?midiFile->getStandard():0;}
const char* CMidiPlayer::getTitle(){return midiFile?midiFile->getTitle():"";}
const char* CMidiPlayer::getCopyright(){return midiFile?midiFile->getCopyright():"";}
double CMidiPlayer::getTempo(){return 60./(ctempo/1e6)/**ctsd/4.*/;}
uint32_t CMidiPlayer::getTCpaused(){return tcpaused;}
void CMidiPlayer::setTCpaused(uint32_t ps){tcpaused=ps;}
uint32_t CMidiPlayer::isFinished(){return finished;}
void CMidiPlayer::setResumed(){resumed=true;}
void CMidiPlayer::setWaitVoice(bool wv){waitvoice=wv;}
void CMidiPlayer::setGain(double gain){if(settings)fluid_settings_setnum(settings,"synth.gain",gain);}
int CMidiPlayer::getPolyphone(){return synth?fluid_synth_get_active_voice_count(synth):0;}
int CMidiPlayer::getMaxPolyphone(){return synth?fluid_synth_get_polyphony(synth):0;}
void CMidiPlayer::setMaxPolyphone(int p){if(synth)fluid_synth_set_polyphony(synth,p);}

void CMidiPlayer::getChannelPreset(int ch,int *b,int *p,char *name)
{
	if(!synth)return(void)(b=0,p=0,strcpy(name,""));
	fluid_synth_channel_info_t info;
	fluid_synth_get_channel_info(synth,ch,&info);
	*b=info.bank;*p=info.program;
	strcpy(name,info.name);
}
void CMidiPlayer::setChannelPreset(int ch,int b,int p)
{
	if(!synth)return;
	fluid_synth_bank_select(synth,ch,b);
	fluid_synth_program_change(synth,ch,p);
}
//16MSB..LSB1
void CMidiPlayer::setBit(uint16_t &n, uint16_t bn, uint16_t b)
{n^=(-b^n)&(1<<bn);}
void CMidiPlayer::setMute(int ch,bool m)
{setBit(mute,ch,m?1:0);}
void CMidiPlayer::setSolo(int ch,bool s)
{setBit(solo,ch,s?1:0);}
int CMidiPlayer::getCC(int ch, int id)
{int ret=0;synth?fluid_synth_get_cc(synth,ch,id,&ret):0;return ret;}
void CMidiPlayer::setCC(int ch, int id, int val)
{synth?fluid_synth_cc(synth,ch,id,val):0;}
void CMidiPlayer::getReverbPara(double *r,double *d,double *w,double *l)
{
	if(!synth)return;
	*r=fluid_synth_get_reverb_roomsize(synth);
	*d=fluid_synth_get_reverb_damp(synth);
	*w=fluid_synth_get_reverb_width(synth);
	*l=fluid_synth_get_reverb_level(synth);
}
void CMidiPlayer::setReverbPara(int e,double r,double d,double w,double l)
{
	if(!synth)return;
	fluid_synth_set_reverb_on(synth,e);
	fluid_synth_set_reverb(synth,r,d,w,l);
}
void CMidiPlayer::getChorusPara(int *fb,double *l,double *r,double *d,int *type)
{
	if(!synth)return;
	*fb=fluid_synth_get_chorus_nr(synth);
	*l=fluid_synth_get_chorus_level(synth);
	*r=fluid_synth_get_chorus_speed_Hz(synth);
	*d=fluid_synth_get_chorus_depth_ms(synth);
	*type=fluid_synth_get_chorus_type(synth);
}
void CMidiPlayer::setChorusPara(int e,int fb,double l,double r,double d,int type)
{
	if(!synth)return;
	fluid_synth_set_chorus_on(synth,e);
	fluid_synth_set_chorus(synth,fb,l,r,d,type);
}
fluid_settings_t* CMidiPlayer::getFluidSettings(){return settings;}
void CMidiPlayer::pushSoundFont(const char *sf)
{fluid_synth_sfload(synth,sf,1);}
int CMidiPlayer::getSFCount()
{return synth?fluid_synth_sfcount(synth):0;}
fluid_sfont_t* CMidiPlayer::getSFPtr(int sfid)
{return synth&&sfid<getSFCount()?fluid_synth_get_sfont(synth,sfid):NULL;}
