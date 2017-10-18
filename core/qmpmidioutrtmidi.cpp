#include <cstdio>
#include <cstring>
#include <vector>
#include RT_MIDI_H
#include "qmpmidioutrtmidi.hpp"
qmpMidiOutRtMidi::qmpMidiOutRtMidi(unsigned _portid)
{
	portid=_portid;
	outport=NULL;
}
qmpMidiOutRtMidi::~qmpMidiOutRtMidi()
{
	if(!outport)return;
	if(outport->isPortOpen())outport->closePort();
	delete outport;outport=NULL;
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
		outport=NULL;
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

RtMidiOut* qmpRtMidiManager::dummy=NULL;
void qmpRtMidiManager::createDevices()
{
	try{dummy=new RtMidiOut();}
	catch(RtMidiError &e)
	{
		printf("Failed to initialize the dummy device: %s\n",e.what());
		dummy=NULL;return;
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
