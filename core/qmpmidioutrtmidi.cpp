#include <cctype>
#include <cstdio>
#include <cstring>
#include <algorithm>
#include <deque>
#include <vector>
#include "RtMidi.h"
#include "qmpmidioutrtmidi.hpp"

void split(std::string s,char c,std::deque<std::string>& v)
{
	v.clear();
	for(size_t anch=0;;)
	{
		std::string sec;
		if(s.find(c,anch)==std::string::npos)
		sec=s.substr(anch);
		else sec=s.substr(anch,s.find(c,anch)-anch);
		if(!sec.empty())v.push_back(sec);
		if(s.find(c,anch)==std::string::npos)break;
		anch=s.find(c,anch)+1;
	}
}

qmpDeviceInitializer* qmpDeviceInitializer::parse(const char* path)
{
	qmpDeviceInitializer *ret=new qmpDeviceInitializer();
	ret->initseq.eventList.clear();
	memset(ret->sinitv,0xFF,sizeof(ret->sinitv));

	bool st_inmapping=false;
	char buf[1024];
	int ln=0;
	int cmsb=-1,clsb=-1;

#define err(e) {delete ret;return fprintf(stderr,"line %d: %s",ln,e),nullptr;}
	FILE* f=fopen(path,"r");
	if(!f)err("file not found")

	//writing such a bad parser makes me want my money for
	//the credits from "compiler principles" back...
	auto h2d=[](char c)->char{return 'F'>=c&&c>='A'?c-'A'+10:'9'>=c&&c>='0'?c-'0':-1;};
	auto hh2d=[](const char *c)->int
	{
		if(!c||!*c||strlen(c)>2)return -1;
		int x=-1,r;
		r=sscanf(c,"%x",&x);
		(x<0||x>0xff)&&(x=-1);
		return r==1?x:-1;
	};
	while(fgets(buf,1024,f))
	{
		++ln;
		if(buf[0]=='#')continue;
		std::string b(buf);
		while(b.length()&&b.back()=='\n')b.pop_back();
		std::deque<std::string> tok;
		split(b,' ',tok);
		if(!tok.size())continue;
		if(tok.front()=="MAP")
		{
			if(st_inmapping)
				err("invalid command")
			st_inmapping=true;
		}
		else if(tok.front()=="ENDMAP")
		{
			if(!st_inmapping)
				err("invalid command")
			st_inmapping=false;
		}
		else if(tok.front()=="X")
		{
			if(st_inmapping)
				err("invalid command")
			tok.pop_front();
			std::string sysx;
			for(auto&i:tok)sysx+=i;
			SEvent ev;
			ev.type=0xF0;
			ev.str="";
			for(auto i=sysx.begin();i!=sysx.end();++i)
			{
				char hn=h2d((char)toupper(*i));
				if(hn<0)err("invalid sysex")
				if(++i==sysx.end())err("invalid sysex")
				char ln=h2d((char)toupper(*i));
				ev.str.push_back((char)(hn<<4|ln));
			}
			ret->initseq.appendEvent(ev);
		}
		else if(tok.front()=="C")
		{
			if(st_inmapping)
				err("invalid command")
			if(tok.size()!=4)err("invalid control")
			int ch=hh2d(tok[1].c_str());
			int cc=hh2d(tok[2].c_str());
			int cv=hh2d(tok[3].c_str());
			if(!~cc||!~cv)err("invalid control parameters")
			if(ch==0xff)
			{
				for(int i=0;i<16;++i)
				{
					SEvent e;
					e.type=(uint8_t)(0xB0|i);
					e.p1=(uint8_t)cc;
					e.p2=(uint8_t)cv;
					ret->initseq.appendEvent(e);
				}
			}
			else if(ch>=0&&ch<0x10)
			{
				SEvent e;
				e.type=(uint8_t)(0xB0|ch);
				e.p1=(uint8_t)cc;
				e.p2=(uint8_t)cv;
				ret->initseq.appendEvent(e);
			}
			else err("invalid channel")
		}
		else if(tok.front()=="IV")
		{
			if(st_inmapping)
				err("invalid command")
			int ci=0;
			tok.pop_front();
			for(auto&tk:tok)
			{
				int v=0,rep=1;
				if(tk.find(',')!=std::string::npos)
				sscanf(tk.c_str(),"%x,%d",&v,&rep);
				else sscanf(tk.c_str(),"%x",&v);
				if(v>0xff||v<0)err("invalid init vector value")
				for(int i=0;i<rep;++i)
				{
					if(ci>=130)err("invalid init vector")
					ret->initv[ci++]=(uint8_t)v;
				}
			}
		}
		else if(tok.front()=="SIV")
		{
			if(st_inmapping)
				err("invalid command")
			int ch=hh2d(tok[1].c_str());
			int cc=hh2d(tok[2].c_str());
			int cv=hh2d(tok[3].c_str());
			ret->sinitv[ch][cc]=uint8_t(cv);
		}
		else if(st_inmapping)
		{
			if(b.front()=='['&&b.back()==']')
			{
				b=b.substr(1,b.length()-2);
				split(b,':',tok);
				if(tok.size()<3)err("invalid bank")
				cmsb=strtol(tok[0].c_str(),nullptr,10);
				clsb=strtol(tok[1].c_str(),nullptr,10);
				ret->banks[cmsb<<7|clsb]=BankStore{{},tok[2]};
			}
			else if(b.find('=')!=std::string::npos)
			{
				if(!~cmsb||!~clsb)err("inst outside a bank")
				split(b,'=',tok);
				if(tok.size()<2)err("invalid inst")
				int p=strtol(tok[0].c_str(),nullptr,10);
				ret->banks[cmsb<<7|clsb].presets[p]=tok[1];
			}
			else err("invalid mapping line")
		}
	}
#undef err

	fclose(f);
	return ret;
}

qmpMidiOutRtMidi::qmpMidiOutRtMidi(unsigned _portid)
{
	portid=_portid;
	outport=nullptr;
	devinit=nullptr;
}
qmpMidiOutRtMidi::~qmpMidiOutRtMidi()
{
	if(!outport)return;
	if(devinit){delete devinit;devinit=nullptr;}
	if(outport->isPortOpen())outport->closePort();
	delete outport;outport=nullptr;
}
void qmpMidiOutRtMidi::deviceInit()
{
	try
	{
		outport=new RtMidiOut();
		reset(0xFF);
	}
	catch(RtMidiError &e)
	{
		fprintf(stderr,"Cannot create RtMidi Output instance: %s\n",e.what());
		outport=nullptr;
	}
}
void qmpMidiOutRtMidi::deviceDeinit()
{
	if(!outport||!outport->isPortOpen())return;
	outport->closePort();
}
void qmpMidiOutRtMidi::basicMessage(uint8_t type,uint8_t p1,uint8_t p2)
{
	if(!outport||!outport->isPortOpen())return;
	std::vector<unsigned char>msg;
	msg.push_back(type);
	msg.push_back(p1);
	if(((type&0xF0)!=0xC0)&&((type&0xF0)!=0xD0))
	msg.push_back(p2);
	try
	{
		outport->sendMessage(&msg);
	}
	catch(RtMidiError &e)
	{
		fprintf(stderr,"Failed to send midi message: %s\n",e.what());
	}
}
void qmpMidiOutRtMidi::extendedMessage(uint32_t length,const char *data)
{
	if(!outport||!outport->isPortOpen())return;
	std::vector<unsigned char>msg(data,data+length);
	try
	{
		outport->sendMessage(&msg);
	}
	catch(RtMidiError &e)
	{
		fprintf(stderr,"Failed to send midi message: %s\n",e.what());
	}
}
void qmpMidiOutRtMidi::rpnMessage(uint8_t ch,uint16_t type,uint16_t val)
{
	basicMessage(0xB0|ch,0x64,type&0x7F);
	basicMessage(0xB0|ch,0x65,type>>7);
	basicMessage(0xB0|ch,0x06,val>>7);
	basicMessage(0xB0|ch,0x26,val&0x7F);
}
void qmpMidiOutRtMidi::nrpnMessage(uint8_t ch,uint16_t type,uint16_t val)
{
	basicMessage(0xB0|ch,0x62,type&0x7F);
	basicMessage(0xB0|ch,0x63,type>>7);
	basicMessage(0xB0|ch,0x06,val>>7);
	basicMessage(0xB0|ch,0x26,val&0x7F);
}
void qmpMidiOutRtMidi::panic(uint8_t ch)
{
	//maybe all notes off is more close to panic?
	basicMessage(0xE0|ch,0x0,0x40);
	basicMessage(0xB0|ch,123,0);
}
void qmpMidiOutRtMidi::reset(uint8_t ch)
{
	if(ch==0xFF)
	{
		if(devinit)
			for(auto&msg:devinit->initseq.eventList)
			{
				if((msg.type&0xF0)==0xF0)
					extendedMessage(msg.str.length(),msg.str.data());
				else
					basicMessage(msg.type,msg.p1,msg.p2);
			}
	}
	else
	{
		basicMessage(0xB0|ch,121,0);
		basicMessage(0xB0|ch,123,0);
	}
}
void qmpMidiOutRtMidi::onMapped(uint8_t,int)
{
	if(!outport)deviceInit();
	if(outport->isPortOpen())return;
	try
	{
		outport->openPort(portid);
	}
	catch(RtMidiError &e)
	{
		fprintf(stderr,"Device initialization failure: %s\n",e.what());
	}

}
void qmpMidiOutRtMidi::onUnmapped(uint8_t ch,int refcnt)
{
	panic(ch);
	if(!refcnt&&outport)outport->closePort();
}
std::vector<std::pair<uint16_t,std::string>> qmpMidiOutRtMidi::getBankList()
{
	if(!devinit)return{};
	std::vector<std::pair<uint16_t,std::string>> ret;
	for(auto&i:devinit->banks)
		ret.push_back({i.first,i.second.bankname});
	std::sort(ret.begin(),ret.end(),[](std::pair<uint16_t,std::string>&a,std::pair<uint16_t,std::string>&b){return a.first<b.first;});
	return ret;
}
std::vector<std::pair<uint8_t,std::string>> qmpMidiOutRtMidi::getPresets(uint16_t bank)
{
	if(!devinit)return{};
	if(devinit->banks.find(bank)==devinit->banks.end())return{};
	std::vector<std::pair<uint8_t,std::string>> ret;
	for(auto&i:devinit->banks[bank].presets)
		ret.push_back({i.first,i.second});
	std::sort(ret.begin(),ret.end(),[](std::pair<uint8_t,std::string>&a,std::pair<uint8_t,std::string>&b){return a.first<b.first;});
	return ret;
}
std::string qmpMidiOutRtMidi::getPresetName(uint16_t bank,uint8_t preset)
{
	if(!devinit)return"";
	if(devinit->banks.find(bank)!=devinit->banks.end())
	if(devinit->banks[bank].presets.find(preset)!=devinit->banks[bank].presets.end())
		return devinit->banks[bank].presets[preset];
	return"";
}
bool qmpMidiOutRtMidi::getChannelPreset(int ch,uint16_t *bank,uint8_t *preset,std::string &presetname)
{
	//This ain't a state-tracking device
	return false;
}
uint8_t qmpMidiOutRtMidi::getInitialCCValue(uint8_t cc,uint8_t ch)
{
	if(!devinit||cc>=130)return 0;//Nope!
	if(ch<16&&devinit->sinitv[ch][cc]!=0xFF)
		return devinit->sinitv[ch][cc];
	return devinit->initv[cc];
}
void qmpMidiOutRtMidi::setInitializerFile(const char* path)
{
	if(devinit)delete devinit;
	devinit=qmpDeviceInitializer::parse(path);
	reset(0xFF);
}

std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> qmpRtMidiManager::devices;
std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> qmpRtMidiManager::getDevices()
{
	if(devices.size())return devices;
	RtMidiOut *dummy;
	try{dummy=new RtMidiOut();}
	catch(RtMidiError &e)
	{
		fprintf(stderr,"Failed to initialize the dummy device: %s\n",e.what());
		return{};
	}
	for(unsigned i=0;i<dummy->getPortCount();++i)
	devices.push_back(std::make_pair(new qmpMidiOutRtMidi(i),dummy->getPortName(i)));
	delete dummy;
	return devices;
}
