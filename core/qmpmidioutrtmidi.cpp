#include <cctype>
#include <cstdio>
#include <cstring>
#include <deque>
#include <vector>
#include RT_MIDI_H
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
	FILE* f=fopen(path, "r");
	if(!f)return nullptr;

	bool st_inmapping=false;
	char buf[1024];
	int ln=0;
	int cmsb=-1,clsb=-1;

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
#define err(e) {delete ret;return fprintf(stderr,"line %d: %s",ln,e),nullptr;}
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
	}
#undef err

	fclose(f);
	return ret;
}

qmpMidiOutRtMidi::qmpMidiOutRtMidi(unsigned _portid)
{
	portid=_portid;
	outport=nullptr;
}
qmpMidiOutRtMidi::~qmpMidiOutRtMidi()
{
	if(!outport)return;
	if(outport->isPortOpen())outport->closePort();
	delete outport;outport=nullptr;
}
void qmpMidiOutRtMidi::deviceInit()
{
	try
	{
		outport=new RtMidiOut();
	}
	catch(RtMidiError &e)
	{
		printf("Cannot create RtMidi Output instance: %s\n",e.what());
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
	outport->sendMessage(&msg);
}
void qmpMidiOutRtMidi::extendedMessage(uint8_t length,const char *data)
{
	if(!outport||!outport->isPortOpen())return;
	std::vector<unsigned char>msg(data,data+length);
	outport->sendMessage(&msg);
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
	basicMessage(0xB0|ch,121,0);
	basicMessage(0xB0|ch,123,0);
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
		printf("Device initialization failure: %s\n",e.what());
	}

}
void qmpMidiOutRtMidi::onUnmapped(uint8_t ch,int refcnt)
{
	panic(ch);
	if(!refcnt&&outport)outport->closePort();
}
std::vector<std::pair<uint16_t,std::string>> qmpMidiOutRtMidi::getBankList()
{
}
std::vector<std::pair<uint8_t,std::string>> qmpMidiOutRtMidi::getPresets(int bank)
{
}
std::string qmpMidiOutRtMidi::getPresetName(uint16_t bank,uint8_t preset)
{
}
bool qmpMidiOutRtMidi::getChannelPreset(int ch,uint16_t *bank,uint8_t *preset,std::string &presetname)
{
}
uint8_t qmpMidiOutRtMidi::getInitialCCValue(uint8_t cc)
{
}

RtMidiOut* qmpRtMidiManager::dummy=nullptr;
void qmpRtMidiManager::createDevices()
{
	try{dummy=new RtMidiOut();}
	catch(RtMidiError &e)
	{
		printf("Failed to initialize the dummy device: %s\n",e.what());
		dummy=nullptr;return;
	}
	for(unsigned i=0;i<dummy->getPortCount();++i)
	devices.push_back(std::make_pair(new qmpMidiOutRtMidi(i),dummy->getPortName(i)));
}
void qmpRtMidiManager::deleteDevices()
{
	for(size_t i=0;i<devices.size();++i)
		delete devices[i].first;
	devices.clear();
	delete dummy;
}
std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> qmpRtMidiManager::getDevices()
{
	return devices;
}
