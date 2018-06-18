#include <cstdio>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <thread>
#include <fluidsynth.h>
#include "qmpmidiplay.hpp"
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
uint64_t pf;
#endif
//debug purpose
uint32_t ttick;
std::chrono::high_resolution_clock::time_point ttime;
CMidiPlayer* CMidiPlayer::ref=NULL;
bool CMidiPlayer::processEvent(const SEvent *e)
{
	SEventCallBackData cbd(e->type,e->p1,e->p2,tceptr);
	for(int i=0;i<16;++i)if(eventHandlerCB[i])
		eventHandlerCB[i]->callBack(&cbd,eventHandlerCBuserdata[i]);
	uint8_t ch=e->type&0x0F;
	switch(e->type&0xF0)
	{
		case 0x80://Note off
		return true;
		case 0x90://Note on
			if((mute>>ch)&1&&e->p2)return false;//muted
			if(solo&&!((solo>>ch)&1)&&e->p2)return false;//excluded by solo flags
			if(e->p2)chstate[ch]=1;
		return true;
		case 0xB0://CC
			if(e->p1==100)rpnid[ch]=e->p2;
			if(e->p1==6)rpnval[ch]=e->p2;
			if(~rpnid[ch]&&~rpnval[ch])
			{
				if(rpnid[ch]==0)
				{
					internalFluid->rpnMessage(ch,0,rpnval[ch]<<7);
					mididev[mappedoutput[ch]].dev->rpnMessage(ch,0,rpnval[ch]<<7);
					pbr[ch]=rpnval[ch];
				}
				rpnid[ch]=rpnval[ch]=-1;
			}
			chstatus[ch][e->p1]=e->p2;
		return true;
		case 0xC0://PC
			chstatus[ch][128]=e->p1;
		return true;
		case 0xE0://PW
			pbv[ch]=(e->p1|(e->p2<<7))&0x3FFF;
		return true;
		case 0xF0://Meta/SysEx
			if((e->type&0x0F)==0x0F)
			{
				switch(e->p1)
				{
					case 0x51:
						ctempo=e->p2;dpt=ctempo*1000./divs;
						ttime=std::chrono::high_resolution_clock::now();
						ttick=getTick();
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
			if(((e->type&0x0F)==0x00||(e->type&0x0F)==07)&&sendSysEx)
				for(auto& i:mididev)
					if(i.refcnt)
						i.dev->extendedMessage(e->p1,e->str.c_str());
		return false;
	}
	return false;
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
						ctempo=e->p2;dpt=ctempo*1000./divs;
						ccc[0][131]=ctempo;
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
	for(size_t i=0;i<mididev.size();++i)
	if(mididev[i].refcnt)
	for(int j=0;j<16;++j)mididev[i].dev->reset(j);
}
void CMidiPlayer::playEvents()
{
	static uint32_t _lrtick;_lrtick=0;
	ttick=getEvent(0)->time;ttime=std::chrono::high_resolution_clock::now();
	for(ct=getEvent(0)->time;tceptr<ecnt;)
	{
		while(tcpaused)std::this_thread::sleep_for(std::chrono::milliseconds(100));
		using namespace std::chrono;
		high_resolution_clock::time_point b=high_resolution_clock::now();
		if(getTick()-_lrtick>divs){
			_lrtick=getTick();
			//double _dt=(getTick()-ttick)*dpt-duration_cast<nanoseconds>((b-ttime)).count();
			//<0: slower than expected, >0: faster than expected
			//fprintf(stderr,"@ tick %u, dtime %.6fus",getTick(),_dt/1000.);
		}
		for(;!tcstop&&midiReaders&&tceptr<ecnt&&ct==getEvent(tceptr)->time;++tceptr)
			if(processEvent(getEvent(tceptr)))
			{
				SEvent* e=getEvent(tceptr);
				mididev[mappedoutput[e->type&0x0F]].dev->basicMessage(e->type,e->p1,e->p2);
			}
		if(tcstop||!midiReaders||tceptr>=ecnt)break;
		high_resolution_clock::time_point a=high_resolution_clock::now();
		auto sendtime=a-b;
		if(resumed){resumed=false;
		ttick=getTick();ttime=high_resolution_clock::now();
		}
		else
		if(sendtime.count()<(getEvent(tceptr)->time-ct)*dpt)
		{
			double ns_sleep=(getEvent(tceptr)->time-ct)*dpt-sendtime.count();
			double correction=(getTick()-ttick)*dpt-(b-ttime).count();
#ifdef _WIN32
			w32usleep((uint64_t)(((getEvent(tceptr)->time-ct)*dpt-sendtime.count())/1000));
#else
			std::this_thread::sleep_for(std::chrono::nanoseconds((uint64_t)(ns_sleep+correction)));
#endif
		}
		if(tcstop||!midiReaders)break;
		ct=getEvent(tceptr)->time;
	}
	while(!tcstop&&(waitvoice&&internalFluid->getPolyphone()>0))std::this_thread::sleep_for(std::chrono::milliseconds(2));
	finished=1;
}
void CMidiPlayer::fileTimer1Pass()
{
	ftime=.0;ctempo=0x7A120;dpt=ctempo*1000./divs;
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
	double ctime=.0;uint32_t c=1;ctempo=0x7A120;dpt=ctempo*1000./divs;
	memset(stamps,0,sizeof(stamps));memset(ccstamps,0,sizeof(ccstamps));
	memset(ccc,0,sizeof(ccc));memset(rpnid,0xFF,sizeof(rpnid));memset(rpnval,0xFF,sizeof(rpnval));
	for(int i=0;i<16;++i)
	{
		ccc[i][7]=100;ccc[i][10]=64;ccc[i][11]=127;
		ccc[i][11]=127;ccc[i][71]=64;ccc[i][72]=64;
		ccc[i][73]=64;ccc[i][74]=64;ccc[i][75]=64;
		ccc[i][76]=64;ccc[i][77]=64;ccc[i][78]=64;
		ccc[i][131]=ctempo;ccc[i][132]=0x04021808;
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

CMidiPlayer::CMidiPlayer()
{
	midiReaders=new CMidiFileReaderCollection();
	resumed=false;midiFile=NULL;internalFluid=new qmpMidiOutFluid();
	registerMidiOutDevice(internalFluid,"Internal FluidSynth");
	waitvoice=true;
	memset(eventHandlerCB,0,sizeof(eventHandlerCB));
	memset(eventHandlerCBuserdata,0,sizeof(eventHandlerCBuserdata));
	memset(eventReaderCB,0,sizeof(eventReaderCB));
	memset(eventReaderCBuserdata,0,sizeof(eventReaderCBuserdata));
	memset(fileReadFinishCB,0,sizeof(fileReadFinishCB));
	memset(fileReadFinishCBuserdata,0,sizeof(fileReadFinishCBuserdata));
	memset(mappedoutput,0,sizeof(mappedoutput));
	mididev[0].refcnt=16;
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
	if(internalFluid)
	{
		internalFluid->deviceDeinit();
		delete internalFluid;
		internalFluid=NULL;
	}
	if(midiFile)delete midiFile;delete midiReaders;
}
void CMidiPlayer::playerPanic(bool reset)
{
	for(auto& i:mididev)
	if(i.refcnt)
	{
		for(uint8_t j=0;j<16;++j)
		reset?i.dev->reset(j):i.dev->panic(j);
	}
}
bool CMidiPlayer::playerLoadFile(const char* fn)
{
	notes=0;if(midiFile)delete midiFile;
	midiFile=midiReaders->readFile(fn);
	if(!midiFile||!midiFile->valid)return false;
	divs=midiFile->divs;maxtk=ecnt=0;
	for(CMidiTrack& i:midiFile->tracks)
	{
		ecnt+=i.eventList.size();
		if(i.eventList.size())
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
	ctempo=0x7A120;ctsn=4;ctsd=4;cks=0;dpt=ctempo*1000./divs;
	tceptr=0;tcstop=0;tcpaused=0;finished=0;mute=solo=0;
	for(int i=0;i<16;++i)pbr[i]=2,pbv[i]=8192;
	sendSysEx=true;memset(rpnid,0xFF,sizeof(rpnid));memset(rpnval,0xFF,sizeof(rpnval));
	memset(chstatus,0,sizeof(chstatus));for(int i=0;i<16;++i)
		chstatus[i][7]=100,chstatus[i][11]=127,
		chstatus[i][10]=chstatus[i][71]=chstatus[i][72]=
		chstatus[i][73]=chstatus[i][74]=chstatus[i][75]=
		chstatus[i][76]=chstatus[i][77]=chstatus[i][78]=64;
}
void CMidiPlayer::playerDeinit()
{
	tceptr=0;tcstop=1;tcpaused=0;
	delete midiFile;midiFile=NULL;
}
void CMidiPlayer::playerThread()
{
	prePlayInit();
	playEvents();
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
		qmpMidiOutDevice* dest=mididev[mappedoutput[i]].dev;
		for(int j=0;j<120;++j)
		{
			internalFluid->basicMessage(0xB0|i,j,ccstamps[st][i][j]);
			dest->basicMessage(0xB0|i,j,ccstamps[st][i][j]);
			chstatus[i][j]=ccstamps[st][i][j];
		}
		internalFluid->basicMessage(0xC0|i,ccstamps[st][i][128],0);
		dest->basicMessage(0xC0|i,ccstamps[st][i][128],0);
		chstatus[i][128]=ccstamps[st][i][128];
		//fluid_synth_pitch_bend(synth,i,ccstamps[st][i][130]);
		internalFluid->rpnMessage(i,0,ccstamps[st][i][134]<<7);
		dest->rpnMessage(i,0,ccstamps[st][i][134]<<7);
		pbr[i]=ccstamps[st][i][134];
		ctempo=ccstamps[st][0][131];dpt=ctempo*1000./divs;
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

void CMidiPlayer::getChannelPreset(int ch,int *b,int *p,char *name)
{
	if(mappedoutput[ch])
	{
		*b=((int)chstatus[ch][0]<<7)|chstatus[ch][32];
		*p=chstatus[ch][128];
		strcpy(name,"");
	}
	else
	{
		internalFluid->getChannelInfo(ch,b,p,name);
	}
}
void CMidiPlayer::setChannelPreset(int ch,int b,int p)
{
	chstatus[ch][128]=p;
	if(mappedoutput[ch])
	{
		//external device mode?
		chstatus[ch][0]=b>>7;chstatus[ch][32]=b&0x7F;
		qmpMidiOutDevice* d=mididev[mappedoutput[ch]].dev;
		d->basicMessage(0xB0|ch,0x00,b>>7);
		d->basicMessage(0xB0|ch,0x20,b&0x7F);
		d->basicMessage(0xC0|ch,p,0);
	}
	else
	{
		chstatus[ch][0]=b;//Assuming GS. !!FIXME: This is not correct...
		qmpMidiOutDevice* d=mididev[mappedoutput[ch]].dev;
		d->basicMessage(0xB0|ch,0x00,b);
		d->basicMessage(0xC0|ch,p,0);
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
	return chstatus[ch][id];
}
void CMidiPlayer::setCC(int ch,int id,int val)
{
	chstatus[ch][id]=val;
	mididev[mappedoutput[ch]].dev->basicMessage(0xB0|ch,id,val);
}

qmpMidiOutFluid* CMidiPlayer::fluid(){return internalFluid;}

void CMidiPlayer::registerMidiOutDevice(qmpMidiOutDevice* dev,std::string name)
{
	SMidiDev d;
	d.dev=dev;d.name=name;
	d.refcnt=0;
	mididev.push_back(d);
}
void CMidiPlayer::unregisterMidiOutDevice(std::string name)
{
	for(auto i=mididev.begin();i!=mididev.end();++i)
	if(i->name==name)
	{
		i->dev->deviceDeinit();
		mididev.erase(i);
		break;
	}
}
std::vector<std::string> CMidiPlayer::getMidiOutDevices()
{
	std::vector<std::string> ret;
	for(auto &i:mididev)
	ret.push_back(i.name);
	return ret;
}
int CMidiPlayer::getChannelOutput(int ch)
{
	return mappedoutput[ch];
}
void CMidiPlayer::setChannelOutput(int ch,int outid)
{
	int origoutput=mappedoutput[ch];
	SMidiDev& dnew=mididev[outid];
	dnew.dev->onMapped(ch,++dnew.refcnt);
	for(int i=0;i<128;++i)
	if(i!=6&&i!=38&&i!=100&&i!=101)//avoid sending RPN/NRPN
	dnew.dev->basicMessage(0xB0|ch,i,chstatus[ch][i]);
	dnew.dev->basicMessage(0xC0|ch,chstatus[ch][128],0);
	mappedoutput[ch]=outid;
	if(~origoutput)
	{
		SMidiDev& dold=mididev[origoutput];
		dold.dev->onUnmapped(ch,--dold.refcnt);
	}
}
uint8_t* CMidiPlayer::getChstates(){return chstate;}
int CMidiPlayer::setEventHandlerCB(ICallBack *cb,void *userdata)
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
int CMidiPlayer::setEventReaderCB(ICallBack *cb,void *userdata)
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
int CMidiPlayer::setFileReadFinishedCB(ICallBack *cb,void *userdata)
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
void CMidiPlayer::registerReader(qmpFileReader *reader,std::string name)
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
