#ifndef QMPIMIDIMAPPER_H
#define QMPIMIDIMAPPER_H
class qmpIMidiMapper
{
	virtual void deviceInit(int id)=0;
	virtual void deviceDeinit(int id)=0;
	virtual void noteOn(int ch,int key,int vel)=0;
	virtual void noteOff(int ch,int key)=0;
	virtual void ctrlChange(int ch,int cc,int val)=0;
	virtual void progChange(int ch,int val)=0;
	virtual void pitchBend(int ch,int val)=0;
	virtual void sysEx(int length,const char* data)=0;
	virtual static int enumDevices()=0;
	virtual static char* deviceName(int id)=0;
};
#endif // QMPIMIDIMAPPER_H
