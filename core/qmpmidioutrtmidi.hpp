#ifndef QMPMIDIMAPPERS_H
#define QMPMIDIMAPPERS_H
#include <vector>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
#include RT_MIDI_H
class qmpMidiOutRtMidi:public qmpMidiOutDevice
{
private:
	unsigned portid;
	RtMidiOut* outport;
public:
	qmpMidiOutRtMidi(unsigned _portid);
	~qmpMidiOutRtMidi();
	void deviceInit();
	void deviceDeinit();
	void basicMessage(uint8_t type,uint8_t p1,uint8_t p2);
	void extendedMessage(uint8_t length,const char *data);
	void rpnMessage(uint8_t ch,uint16_t type,uint16_t val);
	void nrpnMessage(uint8_t ch,uint16_t type,uint16_t val);
	void panic(uint8_t ch);
	void reset(uint8_t ch);
	void onMapped(uint8_t ch,int refcnt);
	void onUnmapped(uint8_t ch,int refcnt);
};
class qmpRtMidiManager
{
	private:
		static RtMidiOut* dummy;
		std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> devices;
	public:
		void createDevices();
		void deleteDevices();
		std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> getDevices();
};
#endif // QMPMIDIMAPPERS_H