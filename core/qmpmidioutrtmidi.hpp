#ifndef QMPMIDIMAPPERS_H
#define QMPMIDIMAPPERS_H
#include <unordered_map>
#include <vector>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
#include "rtmidi/RtMidi.h"
struct qmpDeviceInitializer
{
	CMidiTrack initseq;
	struct BankStore
	{
		std::unordered_map<uint8_t,std::string> presets;
		std::string bankname;
	};
	std::unordered_map<uint16_t,BankStore> banks;
	uint8_t initv[130];

	static qmpDeviceInitializer* parse(const char* path);
};
class qmpMidiOutRtMidi:public qmpMidiOutDevice
{
private:
	unsigned portid;
	RtMidiOut* outport;
	qmpDeviceInitializer* devinit;
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
	std::vector<std::pair<uint16_t,std::string>> getBankList();
	std::vector<std::pair<uint8_t,std::string>> getPresets(uint16_t bank);
	std::string getPresetName(uint16_t bank,uint8_t preset);
	bool getChannelPreset(int ch,uint16_t *bank,uint8_t *preset,std::string &presetname);
	uint8_t getInitialCCValue(uint8_t cc);

	void setInitializerFile(const char* path);
};
class qmpRtMidiManager
{
	private:
		static std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> devices;
	public:
		static std::vector<std::pair<qmpMidiOutRtMidi*,std::string>> getDevices();
};
#endif // QMPMIDIMAPPERS_H
