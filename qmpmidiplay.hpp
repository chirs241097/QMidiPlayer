//sorry for the stupid C-like code...
#ifndef QMPMIDIPLAY_H
#define QMPMIDIPLAY_H
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <fluidsynth.h>
struct SEvent
{
	uint32_t iid,time,p1,p2;
	uint8_t type;
	char *str;
	SEvent(){time=p1=p2=0;type=0;str=NULL;}
	SEvent(uint32_t _iid,uint32_t _t,char _tp,uint32_t _p1,uint32_t _p2,const char* s=NULL)
	{
		iid=_iid;time=_t;type=_tp;
		p1=_p1;p2=_p2;
		if(s){str=new char[strlen(s)+2];strcpy(str,s);}else str=NULL;
	}
};
class CMidiFile
{
	private:
		SEvent *eventList[10000000];
		uint32_t eventc;
		uint32_t fmt,trk,divs;
		FILE *f;
		int byteread;
		uint32_t notes,curt,curid;

		void error(int fatal,const char* format,...);
		uint32_t readSW();
		uint32_t readDW();
		uint32_t readVL();
		int eventReader();
		void trackChunkReader();
		void headerChunkReader();
		void chunkReader(int hdrXp);
	public:
		CMidiFile(const char* fn);
		~CMidiFile();
		const SEvent* getEvent(uint32_t id);
		uint32_t getEventCount();
		uint32_t getDivision();
};
class CMidiPlayer
{
	private:
		CMidiFile *midiFile;
		uint32_t stamps[101];
		double ftime;
		fluid_settings_t* settings;
		fluid_synth_t* synth;
		fluid_audio_driver_t* adriver;
		uint32_t ctempo,ctsn,ctsd,dpt,divs;//delay_per_tick
		//thread control
		uint32_t tceptr,tcpaused,tcstop;
		uint32_t finished,resumed;

		void fluidInitialize(const char* sf);
		void fluidDeinitialize();
		void processEvent(const SEvent *e);
		void processEventStub(const SEvent *e);
		void playEvents();
		void fileTimer1Pass();
		void fileTimer2Pass();
	public:
		CMidiPlayer();
		void playerLoadFile(const char* fn);
		void playerInit();
		void playerDeinit();
		void playerThread();
		void playerPanic();

		uint32_t getStamp(int id);
		uint32_t getTCeptr();
		void setTCeptr(uint32_t ep);
		uint32_t getTCpaused();
		void setTCpaused(uint32_t ps);
		double getFtime();
		uint32_t isFinished();
		void setResumed();

		void setGain(double gain);
		int getPolyphone();
		int getMaxPolyphone();
		void setMaxPolyphone(int p);
};
#endif
