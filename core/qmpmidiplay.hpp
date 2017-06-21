//sorry for the stupid C-like code...
#ifndef QMPMIDIPLAY_H
#define QMPMIDIPLAY_H
#include <cstring>
#include <cstdlib>
#include <utility>
#include <vector>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
#include "qmpmidioutrtmidi.hpp"
#include "qmpmidioutfluid.hpp"
class CMidiPlayer;
class CSMFReader:public IMidiFileReader
{
	private:
		CMidiFile* ret;
		CMidiTrack* curTrack;
		uint32_t fmt,trk;
		FILE *f;
		int byteread,valid,eventdiscarded;
		uint32_t curt,curid;

		void error(int fatal,const char* format,...);
		uint32_t readSW();
		uint32_t readDW();
		uint32_t readVL();
		int eventReader();
		void trackChunkReader();
		void headerChunkReader();
		int chunkReader(int hdrXp);
	public:
		CSMFReader();
		~CSMFReader();
		CMidiFile* readFile(const char* fn);
		void discardCurrentEvent();
		void commitEventChange(SEventCallBackData d);
};
class CMidiFileReaderCollection{
	private:
		std::vector<std::pair<IMidiFileReader*,std::string>> readers;
		IMidiFileReader* currentReader;
	public:
		CMidiFileReaderCollection();
		~CMidiFileReaderCollection();
		void registerReader(IMidiFileReader* reader,std::string name);
		void unregisterReader(std::string name);
		CMidiFile* readFile(const char* fn);
		IMidiFileReader* getCurrentReader();
};
class CMidiPlayer
{
	friend class CMidiFileReaderCollection;
	private:
		CMidiFileReaderCollection *midiReaders;
		CMidiFile* midiFile;
		std::vector<std::pair<size_t,size_t>> eorder;
		uint32_t stamps[101],notes,ecnt,maxtk;
		uint32_t ccstamps[101][16][135],ccc[16][135];
		//0..127:cc 128:pc 129:cp 130:pb 131:tempo 132:ts 133:ks 134:pbr
		int32_t rpnid[16],rpnval[16];
		uint16_t mute,solo;
		double ftime;
		bool sendSysEx,waitvoice;
		uint8_t chstate[16],chstatus[16][130];//0..127: cc 128: pc
		qmpMidiOutFluid* internalFluid;
		uint32_t ctempo,ctsn,ctsd,dpt,divs,cks;
		//raw tempo, timesig num., timesig den., delay per tick, division, keysig
		//thread control
		uint32_t tceptr,tcpaused,tcstop,ct;
		uint32_t finished,resumed;
		uint32_t pbr[16],pbv[16];
		struct SMidiDev
		{
			std::string name;
			qmpMidiOutDevice* dev;
			int refcnt;
		};
		std::vector<SMidiDev> mididev;
		int mappedoutput[16];
		IMidiCallBack* eventHandlerCB[16];
		IMidiCallBack* eventReaderCB[16];
		IMidiCallBack* fileReadFinishCB[16];
		void* eventHandlerCBuserdata[16];
		void* eventReaderCBuserdata[16];
		void* fileReadFinishCBuserdata[16];
		static CMidiPlayer* ref;

		SEvent *getEvent(int id);
		void dumpFile();
		void setBit(uint16_t &n,uint16_t bn,uint16_t b);
		bool processEvent(const SEvent *e);
		void processEventStub(const SEvent *e);
		void prePlayInit();
		void playEvents();
		void fileTimer1Pass();
		void fileTimer2Pass();
	public:
		CMidiPlayer();
		~CMidiPlayer();
		bool playerLoadFile(const char* fn);
		void playerInit();
		void playerDeinit();
		void playerThread();
		void playerPanic(bool reset=false);

		//playing control methods
		uint32_t getStamp(int id);
		uint32_t getTCeptr();
		void setTCeptr(uint32_t ep,uint32_t st);
		uint32_t getTCpaused();
		void setTCpaused(uint32_t ps);
		uint32_t isFinished();
		void setResumed();
		void setWaitVoice(bool wv);

		double getFtime();
		void getCurrentTimeSignature(int *n,int *d);
		int getCurrentKeySignature();
		uint32_t getFileNoteCount();
		uint32_t getFileStandard();
		double getTempo();
		uint32_t getTick();
		uint32_t getRawTempo();
		uint32_t getDivision();
		uint32_t getMaxTick();
		double getPitchBend(int ch);
		const char* getTitle();
		const char* getCopyright();

		void sendSysX(bool send);

		void setChannelPreset(int ch,int b,int p);
		void getChannelPreset(int ch,int *b,int *p,char *name);
		void setMute(int ch,bool m);
		void setSolo(int ch,bool s);
		bool getChannelMask(int ch);
		int getCC(int ch,int id);
		void setCC(int ch,int id,int val);

		qmpMidiOutFluid* fluid();

		void registerMidiOutDevice(qmpMidiOutDevice* dev,std::string name);
		void unregisterMidiOutDevice(std::string name);
		std::vector<std::string> getMidiOutDevices();
		int getChannelOutput(int ch);
		void setChannelOutput(int ch,int outid);
		uint8_t* getChstates();
		int setEventHandlerCB(IMidiCallBack *cb,void *userdata);
		void unsetEventHandlerCB(int id);
		int setEventReaderCB(IMidiCallBack *cb,void *userdata);
		void unsetEventReaderCB(int id);
		int setFileReadFinishedCB(IMidiCallBack *cb,void *userdata);
		void unsetFileReadFinishedCB(int id);
		void registerReader(IMidiFileReader* reader,std::string name);
		void unregisterReader(std::string name);
		void callEventReaderCB(SEventCallBackData d);

		void discardCurrentEvent();
		void commitEventChange(SEventCallBackData d);

		static CMidiPlayer* getInstance();
};
#endif
