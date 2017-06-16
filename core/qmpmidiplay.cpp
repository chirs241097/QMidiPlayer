#include <cstdio>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fluidsynth.h>
#include "qmpmidiplay.hpp"
#ifdef _WIN32
#include <windows.h>
uint64_t pf;
#endif
CMidiPlayer* CMidiPlayer::ref=NULL;
void CMidiPlayer::fluidPreInitialize()
{
	settings=new_fluid_settings();
}
void CMidiPlayer::fluidInitialize()
{
	synth=new_fluid_synth(settings);
	fluid_set_log_function(FLUID_DBG,NULL,NULL);
	fluid_set_log_function(FLUID_INFO,NULL,NULL);
	fluid_set_log_function(FLUID_WARN,NULL,NULL);
	fluid_set_log_function(FLUID_ERR,fluid_default_log_function,NULL);
	fluid_set_log_function(FLUID_PANIC,fluid_default_log_function,NULL);
	adriver=new_fluid_audio_driver(settings,synth);
	fluid_synth_set_chorus(synth,FLUID_CHORUS_DEFAULT_N,FLUID_CHORUS_DEFAULT_LEVEL,
						   FLUID_CHORUS_DEFAULT_SPEED,FLUID_CHORUS_DEFAULT_DEPTH,
						   FLUID_CHORUS_DEFAULT_TYPE);
#ifndef _WIN32
	if(!singleInstance)
	{
		if(midiFile->std==4)
		fluid_synth_set_channel_type(synth,9,CHANNEL_TYPE_MELODIC);
		else if(midiFile->std==1)
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
	SEventCallBackData cbd(e->type,e->p1,e->p2,tceptr);
	for(int i=0;i<16;++i)if(eventHandlerCB[i])
		eventHandlerCB[i]->callBack(&cbd,eventHandlerCBuserdata[i]);
	switch(e->type&0xF0)
	{
		case 0x80://Note off
			if(mappedoutput[e->type&0x0F])
				mapper->noteOff(mappedoutput[e->type&0x0F]-1,e->type&0x0F,e->p1);
			else
				fluid_synth_noteoff(synth,e->type&0x0F,e->p1);
		break;
		case 0x90://Note on
			if((mute>>(e->type&0x0F))&1)break;//muted
			if(solo&&!((solo>>(e->type&0x0F))&1))break;
			if(mappedoutput[e->type&0x0F])
				mapper->noteOn(mappedoutput[e->type&0x0F]-1,e->type&0x0F,e->p1,e->p2);
			else
				fluid_synth_noteon(synth,e->type&0x0F,e->p1,e->p2);
			chstate[e->type&0x0F]=1;
		break;
		case 0xB0://CC
			if(e->p1==100)rpnid[e->type&0x0F]=e->p2;
			if(e->p1==6)rpnval[e->type&0x0F]=e->p2;
			if(~rpnid[e->type&0x0F]&&~rpnval[e->type&0x0F])
			{
				if(rpnid[e->type&0x0F]==0)
				{
					fluid_synth_pitch_wheel_sens(synth,e->type&0x0F,rpnval[e->type&0x0F]);
					pbr[e->type&0x0F]=rpnval[e->type&0x0F];
				}
				rpnid[e->type&0x0F]=rpnval[e->type&0x0F]=-1;
			}
			chstatus[e->type&0x0F][e->p1]=e->p2;
			if(mappedoutput[e->type&0x0F])
				mapper->ctrlChange(mappedoutput[e->type&0x0F]-1,e->type&0x0F,e->p1,e->p2);
			else
				fluid_synth_cc(synth,e->type&0x0F,e->p1,e->p2);
		break;
		case 0xC0://PC
			chstatus[e->type&0x0F][128]=e->p1;
			if(mappedoutput[e->type&0x0F])
				mapper->progChange(mappedoutput[e->type&0x0F]-1,e->type&0x0F,e->p1);
			else
				fluid_synth_program_change(synth,e->type&0x0F,e->p1);
		break;
		case 0xE0://PW
			pbv[e->type&0x0F]=(e->p1|(e->p2<<7))&0x3FFF;;
			if(mappedoutput[e->type&0x0F])
				mapper->pitchBend(mappedoutput[e->type&0x0F]-1,e->type&0x0F,pbv[e->type&0x0F]);
			else
				fluid_synth_pitch_bend(synth,e->type&0x0F,pbv[e->type&0x0F]);
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
				if(sendSysEx)
				{
					for(int i=0;i<16;++i)if(deviceusage[i])mapper->sysEx(i,e->p1,e->str.c_str());
					fluid_synth_sysex(synth,e->str.c_str(),e->p1,NULL,&io,NULL,0);
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
			if(e->p1==100)rpnid[e->type&0x0F]=e->p2;
			if(e->p1==6)rpnval[e->type&0x0F]=e->p2;
			if(~rpnid[e->type&0x0F]&&~rpnval[e->type&0x0F])
			{
				if(rpnid[e->type&0x0F]==0)ccc[e->type&0x0F][134]=rpnval[e->type&0x0F];
				rpnid[e->type&0x0F]=rpnval[e->type&0x0F]=-1;
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
			ccc[e->type&0x0F][130]=(e->p1|(e->p2<<7))&0x3FFF;
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
SEvent* CMidiPlayer::getEvent(int id)
{
	size_t t=eorder[id].first,e=eorder[id].second;
	return &midiFile->tracks[t].eventList[e];
}
void CMidiPlayer::prePlayInit()
{
	playerPanic(true);
	for(int i=0;i<16;++i)if(deviceusage[i])
	for(int j=0;j<16;++j)mapper->reset(i,j);
}
void CMidiPlayer::playEvents()
{
	for(ct=getEvent(0)->time;tceptr<ecnt;)
	{
		while(tcpaused)std::this_thread::sleep_for(std::chrono::milliseconds(100));
		using namespace std::chrono;
		high_resolution_clock::time_point b=high_resolution_clock::now();
		while(!tcstop&&midiReaders&&tceptr<ecnt&&ct==getEvent(tceptr)->time)
			processEvent(getEvent(tceptr++));
		if(tcstop||!midiReaders||tceptr>=ecnt)break;
		high_resolution_clock::time_point a=high_resolution_clock::now();
		auto sendtime=a-b;
		if(resumed)resumed=false;
		else
		if(sendtime.count()<(getEvent(tceptr)->time-ct)*dpt)
#ifdef _WIN32
		w32usleep(((getEvent(tceptr)->time-ct)*dpt-sendtime.count())/1000);
#else
		std::this_thread::sleep_for(std::chrono::nanoseconds((getEvent(tceptr)->time-ct)*dpt-sendtime.count()));
#endif
		if(tcstop||!midiReaders)break;
		ct=getEvent(tceptr)->time;
	}
	while(!tcstop&&synth&&(waitvoice&&fluid_synth_get_active_voice_count(synth)>0))std::this_thread::sleep_for(std::chrono::milliseconds(2));
	finished=1;
}
void CMidiPlayer::fileTimer1Pass()
{
	ftime=.0;ctempo=0x7A120;dpt=ctempo*1000/divs;
	for(uint32_t eptr=0,ct=getEvent(0)->time;eptr<ecnt;)
	{
		while(eptr<ecnt&&ct==getEvent(eptr)->time)
			processEventStub(getEvent(eptr++));
		if(eptr>=ecnt)break;
		ftime+=(getEvent(eptr)->time-ct)*dpt/1e9;
		ct=getEvent(eptr)->time;
	}
}
void CMidiPlayer::fileTimer2Pass()
{
	double ctime=.0;uint32_t c=1;ctempo=0x7A120;dpt=ctempo*1000/divs;
	memset(stamps,0,sizeof(stamps));memset(ccstamps,0,sizeof(ccstamps));
	memset(ccc,0,sizeof(ccc));memset(rpnid,0xFF,sizeof(rpnid));memset(rpnval,0xFF,sizeof(rpnval));
	for(int i=0;i<16;++i)
	{
		ccc[i][7]=100;ccc[i][10]=64;ccc[i][11]=127;
		ccc[i][11]=127;ccc[i][71]=64;ccc[i][72]=64;
		ccc[i][73]=64;ccc[i][74]=64;ccc[i][75]=64;
		ccc[i][76]=64;ccc[i][77]=64;ccc[i][78]=64;
		ccc[i][131]=dpt;ccc[i][132]=0x04021808;
		ccc[i][133]=0;ccc[i][134]=2;
	}if(midiFile->std!=4)ccc[9][0]=128;
	for(int i=0;i<16;++i)for(int j=0;j<135;++j)
		ccstamps[0][i][j]=ccc[i][j];
	for(uint32_t eptr=0,ct=getEvent(0)->time;eptr<ecnt;)
	{
		while(eptr<ecnt&&ct==getEvent(eptr)->time)
			processEventStub(getEvent(eptr++));
		if(eptr>=ecnt)break;
		ctime+=(getEvent(eptr)->time-ct)*dpt/1e9;
		while(ctime>ftime*c/100.)
		{
			for(int i=0;i<16;++i)for(int j=0;j<135;++j)
				ccstamps[c][i][j]=ccc[i][j];
			stamps[c++]=eptr;
			if(c>100)break;
		}
		ct=getEvent(eptr)->time;
	}
	while(c<101)
	{
		for(int i=0;i<16;++i)for(int j=0;j<135;++j)
			ccstamps[c][i][j]=ccc[i][j];
		stamps[c++]=ecnt;
	}
}
CMidiPlayer::CMidiPlayer(bool singleInst)
{
	midiReaders=new CMidiFileReaderCollection();
	resumed=false;singleInstance=singleInst;midiFile=NULL;
	settings=NULL;synth=NULL;adriver=NULL;waitvoice=true;
	memset(eventHandlerCB,0,sizeof(eventHandlerCB));
	memset(eventHandlerCBuserdata,0,sizeof(eventHandlerCBuserdata));
	memset(eventReaderCB,0,sizeof(eventReaderCB));
	memset(eventReaderCBuserdata,0,sizeof(eventReaderCBuserdata));
	memset(fileReadFinishCB,0,sizeof(fileReadFinishCB));
	memset(fileReadFinishCBuserdata,0,sizeof(fileReadFinishCBuserdata));
	memset(mappedoutput,0,sizeof(mappedoutput));
	memset(deviceusage,0,sizeof(deviceusage));
	mapper=new qmpMidiMapperRtMidi();
	memset(chstatus,0,sizeof(chstatus));
	for(int i=0;i<16;++i)
		chstatus[i][7]=100,chstatus[i][11]=127,
		chstatus[i][10]=chstatus[i][71]=chstatus[i][72]=
		chstatus[i][73]=chstatus[i][74]=chstatus[i][75]=
		chstatus[i][76]=chstatus[i][77]=chstatus[i][78]=64;
#ifdef _WIN32
	QueryPerformanceFrequency((LARGE_INTEGER*)&pf);
#endif
	ref=this;
}
CMidiPlayer::~CMidiPlayer()
{
	if(singleInstance||settings||synth||adriver)fluidDeinitialize();
	if(midiFile)delete midiFile;delete midiReaders;delete mapper;
}
void CMidiPlayer::playerPanic(bool reset)
{
	for(int i=0;i<16;++i)
	{
		if(synth){
			if(reset){
				fluid_synth_cc(synth,i,0,0);
				fluid_synth_cc(synth,i,7,100);
				fluid_synth_cc(synth,i,10,64);
				fluid_synth_cc(synth,i,11,127);
				fluid_synth_cc(synth,i,32,0);
				fluid_synth_pitch_wheel_sens(synth,i,2);
			}
			fluid_synth_cc(synth,i,64,0);
			fluid_synth_pitch_bend(synth,i,8192);
			//all sounds off causes the minus polyphone bug...
			fluid_synth_all_notes_off(synth,i);
		}
		if(deviceusage[i])for(int j=0;j<16;++j)
		reset?mapper->reset(i,j):mapper->panic(i,j);
	}
}
bool CMidiPlayer::playerLoadFile(const char* fn)
{
	notes=0;if(midiFile)delete midiFile;
	midiFile=midiReaders->readFile(fn);
	if(!midiFile->valid)return false;
	divs=midiFile->divs;maxtk=ecnt=0;
	for(CMidiTrack& i:midiFile->tracks)
	{
		ecnt+=i.eventList.size();
		maxtk=std::max(maxtk,i.eventList.back().time);
	}
	for(int i=0;i<16;++i)if(fileReadFinishCB[i])
		fileReadFinishCB[i]->callBack(NULL,fileReadFinishCBuserdata[i]);
	eorder.clear();
	for(size_t i=0;i<midiFile->tracks.size();++i)
	for(size_t j=0;j<midiFile->tracks[i].eventList.size();++j)
	eorder.push_back(std::make_pair(i,j));
	std::sort(eorder.begin(),eorder.end(),
		[this](std::pair<size_t,size_t> &a,std::pair<size_t,size_t> &b)->bool{
			return midiFile->tracks[a.first].eventList[a.second]<
				   midiFile->tracks[b.first].eventList[b.second];
		}
	);
	fileTimer1Pass();
	fileTimer2Pass();
	return true;
}
void CMidiPlayer::playerInit()
{
	ctempo=0x7A120;ctsn=4;ctsd=4;cks=0;dpt=ctempo*1000/divs;
	tceptr=0;tcstop=0;tcpaused=0;finished=0;mute=solo=0;
	for(int i=0;i<16;++i)pbr[i]=2,pbv[i]=8192;
	sendSysEx=true;memset(rpnid,0xFF,sizeof(rpnid));memset(rpnval,0xFF,sizeof(rpnval));
	memset(chstatus,0,sizeof(chstatus));for(int i=0;i<16;++i)
		chstatus[i][7]=100,chstatus[i][11]=127,
		chstatus[i][10]=chstatus[i][71]=chstatus[i][72]=
		chstatus[i][73]=chstatus[i][74]=chstatus[i][75]=
		chstatus[i][76]=chstatus[i][77]=chstatus[i][78]=64;
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
	prePlayInit();
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
	if(ep==ecnt)tcstop=1;else tceptr=ep;
	for(int i=0;i<16;++i)
	{
		for(int j=0;j<120;++j)fluid_synth_cc(synth,i,j,ccstamps[st][i][j]);
		fluid_synth_program_change(synth,i,ccstamps[st][i][128]);
		//fluid_synth_pitch_bend(synth,i,ccstamps[st][i][130]);
		fluid_synth_pitch_wheel_sens(synth,i,ccstamps[st][i][134]);
		pbr[i]=ccstamps[st][i][134];
		dpt=ccstamps[st][0][131];ctempo=dpt*divs/1000;
		ctsn=ccstamps[st][0][132]>>24;ctsd=1<<((ccstamps[st][0][132]>>16)&0xFF);
		cks=ccstamps[st][0][133];
	}
}
double CMidiPlayer::getFtime(){return ftime;}
void CMidiPlayer::getCurrentTimeSignature(int *n,int *d){*n=ctsn;*d=ctsd;}
int CMidiPlayer::getCurrentKeySignature(){return cks;}
uint32_t CMidiPlayer::getFileNoteCount(){return notes;}
uint32_t CMidiPlayer::getFileStandard(){return midiFile?midiFile->std:0;}
const char* CMidiPlayer::getTitle(){return midiFile?midiFile->title:"";}
const char* CMidiPlayer::getCopyright(){return midiFile?midiFile->copyright:"";}
double CMidiPlayer::getTempo(){return 60./(ctempo/1e6);}
uint32_t CMidiPlayer::getTick(){return ct;}
uint32_t CMidiPlayer::getRawTempo(){return ctempo;}
uint32_t CMidiPlayer::getDivision(){return divs;}
uint32_t CMidiPlayer::getMaxTick(){return maxtk;}
double CMidiPlayer::getPitchBend(int ch){return((int)pbv[ch]-8192)/8192.*pbr[ch];}
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
	if(!synth)return(void)(*b=0,*p=0,strcpy(name,""));
	if(mappedoutput[ch])
	{
		*b=((int)chstatus[ch][0]<<7)|chstatus[ch][32];
		*p=chstatus[ch][128];
		strcpy(name,"");
	}
	else
	{
		fluid_synth_channel_info_t info;
		fluid_synth_get_channel_info(synth,ch,&info);
		*b=info.bank;*p=info.program;
		strcpy(name,info.name);
	}
}
void CMidiPlayer::setChannelPreset(int ch,int b,int p)
{
	if(!synth)return;
	chstatus[ch][128]=p;
	if(mappedoutput[ch])
	{
		//external device mode?
		chstatus[ch][0]=b>>7;chstatus[ch][32]=b&0x7F;
		mapper->ctrlChange(mappedoutput[ch]-1,ch,0,b>>7);
		mapper->ctrlChange(mappedoutput[ch]-1,ch,32,b&0x7F);
		mapper->progChange(mappedoutput[ch]-1,ch,p);
	}
	else
	{
		chstatus[ch][0]=b;//!!FIXME: This is not correct...
		fluid_synth_bank_select(synth,ch,b);
		fluid_synth_program_change(synth,ch,p);
	}
}
void CMidiPlayer::dumpFile()
{
	if(!midiFile)return;
	for(CMidiTrack &i:midiFile->tracks)
	for(SEvent &j:i.eventList)
	if(j.str.length())
		printf("type %x #%d @%d p1 %d p2 %d str %s\n",j.type,
		j.iid,j.time,j.p1,j.p2,j.str.c_str());
	else
		printf("type %x #%d @%d p1 %d p2 %d\n",j.type,j.iid,
		j.time,j.p1,j.p2);
}
//16MSB..LSB1
void CMidiPlayer::setBit(uint16_t &n, uint16_t bn, uint16_t b)
{n^=(((~b)+1)^n)&(1<<bn);}
void CMidiPlayer::setMute(int ch,bool m)
{setBit(mute,ch,m?1:0);}
void CMidiPlayer::setSolo(int ch,bool s)
{setBit(solo,ch,s?1:0);}
bool CMidiPlayer::getChannelMask(int ch)
{return((mute>>ch)&1)||(solo&&!((solo>>ch)&1));}
int CMidiPlayer::getCC(int ch,int id)
{
	int ret=0;
	if(mappedoutput[ch])
		ret=chstatus[ch][id];
	else
		if(synth)fluid_synth_get_cc(synth,ch,id,&ret);
	return ret;
}
void CMidiPlayer::setCC(int ch,int id,int val)
{
	if(!synth)return;
	chstatus[ch][id]=val;
	mappedoutput[ch]?mapper->ctrlChange(mappedoutput[ch]-1,ch,id,val):
					 (void)fluid_synth_cc(synth,ch,id,val);
}
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

qmpMidiMapperRtMidi* CMidiPlayer::getMidiMapper(){return mapper;}
int CMidiPlayer::getChannelOutput(int ch)
{
	return mappedoutput[ch];
}
void CMidiPlayer::setChannelOutput(int ch,int devid)
{
	int origoutput=mappedoutput[ch];
	int newoutput=0;
	if(devid>0)
	{
		if(!deviceusage[devid-1])deviceiid[devid]=newoutput=mapper->deviceInit(devid-1);
		++deviceusage[deviceiid[devid]];mapper->progChange(deviceiid[devid],ch,chstatus[ch][128]);
		for(int i=0;i<128;++i)if(i!=100&&i!=101)mapper->ctrlChange(deviceiid[devid],ch,i,chstatus[ch][i]);
	}
	else
	{
		fluid_synth_bank_select(synth,ch,chstatus[ch][0]);
		fluid_synth_program_change(synth,ch,chstatus[ch][128]);
		for(int i=0;i<128;++i)if(i!=100&&i!=101&&i!=0&&i!=32)
			fluid_synth_cc(synth,ch,i,chstatus[ch][i]);
	}
	mappedoutput[ch]=devid?deviceiid[devid]+1:0;
	if(origoutput>0)
	{
		--deviceusage[origoutput-1];mapper->panic(origoutput-1,ch);
		if(!deviceusage[origoutput-1])mapper->deviceDeinit(origoutput-1);
	}else if(synth)fluid_synth_all_notes_off(synth,ch);
}
uint8_t* CMidiPlayer::getChstates(){return chstate;}
int CMidiPlayer::setEventHandlerCB(IMidiCallBack *cb,void *userdata)
{
	for(int i=0;i<16;++i)
	{
		if(eventHandlerCB[i]==cb)return i;
		if(eventHandlerCB[i]==NULL)
		{
			eventHandlerCB[i]=cb;eventHandlerCBuserdata[i]=userdata;
			return i;
		}
	}
	return -1;
}
void CMidiPlayer::unsetEventHandlerCB(int id)
{eventHandlerCB[id]=NULL;eventHandlerCBuserdata[id]=NULL;}
int CMidiPlayer::setEventReaderCB(IMidiCallBack *cb,void *userdata)
{
	for(int i=0;i<16;++i)
	{
		if(eventReaderCB[i]==cb)return i;
		if(eventReaderCB[i]==NULL)
		{
			eventReaderCB[i]=cb;eventReaderCBuserdata[i]=userdata;
			return i;
		}
	}
	return -1;
}
void CMidiPlayer::unsetEventReaderCB(int id)
{eventReaderCB[id]=NULL;eventReaderCBuserdata[id]=NULL;}
int CMidiPlayer::setFileReadFinishedCB(IMidiCallBack *cb,void *userdata)
{
	for(int i=0;i<16;++i)
	{
		if(fileReadFinishCB[i]==cb)return i;
		if(fileReadFinishCB[i]==NULL)
		{
			fileReadFinishCB[i]=cb;fileReadFinishCBuserdata[i]=userdata;
			return i;
		}
	}
	return -1;
}
void CMidiPlayer::unsetFileReadFinishedCB(int id)
{fileReadFinishCB[id]=NULL;fileReadFinishCBuserdata[id]=NULL;}
void CMidiPlayer::registerReader(IMidiFileReader *reader,std::string name)
{midiReaders->registerReader(reader,name);}
void CMidiPlayer::unregisterReader(std::string name)
{midiReaders->unregisterReader(name);}
void CMidiPlayer::callEventReaderCB(SEventCallBackData d)
{
	if((d.type&0xF0)==0x90)++notes;
	for(int i=0;i<16;++i)if(eventReaderCB[i])
	eventReaderCB[i]->callBack(&d,eventReaderCBuserdata[i]);
}
void CMidiPlayer::discardCurrentEvent()
{
	if(midiReaders->getCurrentReader())
	midiReaders->getCurrentReader()->discardCurrentEvent();
}
void CMidiPlayer::commitEventChange(SEventCallBackData d)
{
	if(midiReaders->getCurrentReader())
	midiReaders->getCurrentReader()->commitEventChange(d);
}

CMidiPlayer* CMidiPlayer::getInstance(){return ref;}
