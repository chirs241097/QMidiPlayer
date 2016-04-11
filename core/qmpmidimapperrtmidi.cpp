#include <cstdio>
#include <cstring>
#include <vector>
#include "RtMidi.h"
#include "qmpmidimappers.hpp"
RtMidiOut* qmpMidiMapperRtMidi::dummy=NULL;
qmpMidiMapperRtMidi::qmpMidiMapperRtMidi()
{
	dummy=new RtMidiOut();
	memset(ports,0,sizeof(ports));
}
qmpMidiMapperRtMidi::~qmpMidiMapperRtMidi()
{
	delete dummy;for(int i=0;i<16;++i)if(ports[i])delete ports[i];
}
int qmpMidiMapperRtMidi::enumDevices()
{
	return dummy->getPortCount();
}
std::string qmpMidiMapperRtMidi::deviceName(int id)
{
	fprintf(stderr,"port #%d: %s\n",id,dummy->getPortName(id).c_str());
	return dummy->getPortName(id);
}
int qmpMidiMapperRtMidi::deviceInit(int id)
{
	int i=0;for(;ports[i]&&i<16;++i);
	if(i==16)return -1;
	try
	{
		ports[i]=new RtMidiOut();
		ports[i]->openPort(id);
	}
	catch(RtMidiError e)
	{
		printf("Device initialization failure: %s\n",e.what());
		ports[i]=NULL;
		return -1;
	}
	return i;
}
void qmpMidiMapperRtMidi::deviceDeinit(int iid)
{
	if(ports[iid]){ports[iid]->closePort();delete ports[iid];ports[iid]=NULL;}
}
void qmpMidiMapperRtMidi::noteOn(int iid,int ch,int key,int vel)
{
	if(!ports[iid])return;ch&=0x0F;
	std::vector<unsigned char>message;
	message.push_back(0x90|ch);
	message.push_back(key);
	message.push_back(vel);
	ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::noteOff(int iid,int ch,int key)
{
	if(!ports[iid])return;ch&=0x0F;
	std::vector<unsigned char>message;
	message.push_back(0x80|ch);message.push_back(key);message.push_back(0);
	ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::ctrlChange(int iid,int ch,int cc,int val)
{
	if(!ports[iid])return;ch&=0x0F;
	std::vector<unsigned char>message;
	message.push_back(0xB0|ch);message.push_back(cc);message.push_back(val);
	ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::progChange(int iid,int ch,int val)
{
	if(!ports[iid])return;ch&=0x0F;
	std::vector<unsigned char>message;
	message.push_back(0xC0|ch);message.push_back(val);
	ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::pitchBend(int iid,int ch,int val)
{
	if(!ports[iid])return;ch&=0x0F;
	std::vector<unsigned char>message;
	message.push_back(0xE0|ch);message.push_back(val&0x7F);
	message.push_back(val>>7);ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::sysEx(int iid,int length,const char *data)
{
	if(!ports[iid])return;
	std::vector<unsigned char>message(data,data+length);
	ports[iid]->sendMessage(&message);
}
void qmpMidiMapperRtMidi::panic(int iid,int ch)
{
	//maybe all notes off is more close to panic?
	ctrlChange(iid,ch,120,0);
	//ctrlChange(iid,ch,123,0);
}
