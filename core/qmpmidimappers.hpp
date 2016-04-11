#ifndef QMPMIDIMAPPERS_H
#define QMPMIDIMAPPERS_H
#include "RtMidi.h"
class qmpMidiMapperRtMidi
{
private:
	RtMidiOut *ports[16];
	static RtMidiOut *dummy;
public:
	qmpMidiMapperRtMidi();
	~qmpMidiMapperRtMidi();
	int deviceInit(int id);
	void deviceDeinit(int iid);
	void noteOn(int iid,int ch,int key,int vel);
	void noteOff(int iid,int ch,int key);
	void ctrlChange(int iid,int ch,int cc,int val);
	void progChange(int iid,int ch,int val);
	void pitchBend(int iid,int ch,int val);
	void sysEx(int iid,int length,const char* data);
	void panic(int iid,int ch);
	int enumDevices();
	std::string deviceName(int id);
};
#endif // QMPMIDIMAPPERS_H
