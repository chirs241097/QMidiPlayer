//sorry for the stupid C-like code...
#ifndef QMPMIDIPLAY_H
#define QMPMIDIPLAY_H
#include <cstring>
#include <cstdlib>
#include <utility>
#include <vector>
#include <fluidsynth.h>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
#include "qmpmidimappers.hpp"
class CMidiPlayer;
class CSMFReader:public IMidiFileReader
{
	private:
		CMidiFile* ret;
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
	std::vector<std::pair<IMidiFileReader*,std::string>> readers;
	CMidiFile* file;
	uint32_t maxtk;
	IMidiFileReader* currentReader;
	void destructFile(CMidiFile*& f);
	void dumpFile();
public:
	CMidiFileReaderCollection();
	~CMidiFileReaderCollection();
	void registerReader(IMidiFileReader* reader,std::string name);
	void unregisterReader(std::string name);
	void readFile(const char* fn);
	void destructFile();
	IMidiFileReader* getCurrentReader();
	bool isValid();
	const char* getTitle();
	const char* getCopyright();
	const SEvent* getEvent(uint32_t id);
	uint32_t getEventCount();
	uint32_t getDivision();
	uint32_t getMaxTick();
	uint32_t getStandard();
};
class CMidiPlayer
{
	friend class CMidiFileReaderCollection;
	private:
		CMidiFileReaderCollection *midiReaders;
		uint32_t stamps[101],notes;
		uint32_t ccstamps[101][16][135],ccc[16][135];
		//0..127:cc 128:pc 129:cp 130:pb 131:tempo 132:ts 133:ks 134:pbr
		int32_t rpnid[16],rpnval[16];
		uint16_t mute,solo;
		double ftime;
		bool sendSysEx,singleInstance,waitvoice;
		fluid_settings_t* settings;
		fluid_synth_t* synth;
		fluid_audio_driver_t* adriver;
		fluid_player_t* player;
		uint32_t ctempo,ctsn,ctsd,dpt,divs,cks;
		//raw tempo, timesig num., timesig den., delay per tick, division, keysig
		//thread control
		uint32_t tceptr,tcpaused,tcstop,ct;
		uint32_t finished,resumed;
		uint32_t pbr[16],pbv[16];
		qmpMidiMapperRtMidi *mapper;
		int mappedoutput[16],deviceusage[16],deviceiid[128];
		uint8_t chstate[16],chstatus[16][130];//0..127: cc 128: pc
		IMidiCallBack* eventHandlerCB[16];
		IMidiCallBack* eventReaderCB[16];
		IMidiCallBack* fileReadFinishCB[16];
		void* eventHandlerCBuserdata[16];
		void* eventReaderCBuserdata[16];
		void* fileReadFinishCBuserdata[16];
		static CMidiPlayer* ref;

		void setBit(uint16_t &n,uint16_t bn,uint16_t b);
		void processEvent(const SEvent *e);
		void processEventStub(const SEvent *e);
		void prePlayInit();
		void playEvents();
		void fileTimer1Pass();
		void fileTimer2Pass();
	public:
		CMidiPlayer(bool singleInst=false);
		~CMidiPlayer();
		bool playerLoadFile(const char* fn);
		void playerInit();
		void fluidPreInitialize();
		void fluidInitialize();
		void fluidDeinitialize();
		void playerDeinit();
		void playerThread();
		void playerPanic(bool reset=false);

		void rendererLoadFile(const char* ofn);
		void rendererInit(const char *fn);
		void rendererThread();
		void rendererDeinit();

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

		void setGain(double gain);
		void sendSysX(bool send);
		int getPolyphone();
		int getMaxPolyphone();
		void setMaxPolyphone(int p);

		void setChannelPreset(int ch,int b,int p);
		void getChannelPreset(int ch,int *b,int *p,char *name);
		void setMute(int ch,bool m);
		void setSolo(int ch,bool s);
		bool getChannelMask(int ch);
		int getCC(int ch,int id);
		void setCC(int ch,int id,int val);
		void getReverbPara(double *r,double *d,double *w,double *l);
		void setReverbPara(int e,double r,double d,double w,double l);
		void getChorusPara(int *fb,double *l,double *r,double *d,int *type);
		void setChorusPara(int e,int fb,double l,double r,double d,int type);

		fluid_settings_t* getFluidSettings();
		void pushSoundFont(const char* sf);
		int getSFCount();
		fluid_sfont_t* getSFPtr(int sfid);

		qmpMidiMapperRtMidi* getMidiMapper();
		int getChannelOutput(int ch);
		void setChannelOutput(int ch,int devid);
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
