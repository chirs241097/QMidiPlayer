#ifndef QMPVISUALIZATION_H
#define QMPVISUALIZATION_H

#include <vector>
#include <stack>
#include <thread>
#include <smelt.hpp>
#include <smmath.hpp>
#include <smttfont.hpp>
#include "qmpvirtualpiano3d.hpp"
#include "../include/qmpcorepublic.hpp"

class qmpVisualization;
class CReaderCallBack:public IMidiCallBack
{
	private:
		qmpVisualization *par;
	public:
		CReaderCallBack(qmpVisualization *_par){par=_par;}
		void callBack(void *callerdata,void *userdata);
};
class CHandlerCallBack:public IMidiCallBack
{
	private:
		qmpVisualization *par;
	public:
		CHandlerCallBack(qmpVisualization *_par){par=_par;}
		void callBack(void*,void*);
};
struct MidiVisualEvent
{
	uint32_t tcs,tce;
	uint32_t key,vel;
	uint32_t ch;
};
class qmpVisualization:public qmpPluginIntf
{
	friend class CHandlerCallBack;
	friend class CloseHandler;
	private:
		qmpPluginAPI* api;
		CReaderCallBack* cb;
		CHandlerCallBack* hcb;
		qmpVisualizationIntf* vi;
		std::thread* rendererTh;
		std::vector<MidiVisualEvent*>pool;
		smHandler *h,*closeh;
		std::stack<uint32_t> pendingt[16][128],pendingv[16][128];
		SMELT *sm;
		SMTRG tdscn;
		SMTEX chequer;
		smTTFont font,font2;
		qmpVirtualPiano3D* p3d[16];
		float pos[3],rot[3],lastx,lasty;
		uint32_t ctc,ctk,fintk,elb;
		double etps;
		bool shouldclose,playing;
		int hvif,herif,hehif;
		int traveld[16][128];bool notestatus[16][128];
		void drawCube(smvec3d a,smvec3d b,DWORD col,SMTEX tex);
		void showThread();
	public:
		qmpVisualization(qmpPluginAPI* _api);
		~qmpVisualization();
		void pushNoteOn(uint32_t tc,uint32_t ch,uint32_t key,uint32_t vel);
		void pushNoteOff(uint32_t tc,uint32_t ch,uint32_t key);
		bool update();
		void start();
		void stop();
		void pause();
		void show();
		void close();
		void reset();

		void init();
		void deinit();
		const char* pluginGetName();
		const char* pluginGetVersion();
};

class CMidiVisualHandler:public smHandler
{
	private:
		qmpVisualization *p;
	public:
		CMidiVisualHandler(qmpVisualization* par){p=par;}
		bool handlerFunc(){return p->update();}
};

class CloseHandler:public smHandler
{
	private:
		qmpVisualization *p;
	public:
		CloseHandler(qmpVisualization* par){p=par;}
	public:
		bool handlerFunc(){return true;}
};

class CDemoVisualization:public qmpVisualizationIntf
{
	private:
		qmpVisualization* par;
	public:
		CDemoVisualization(qmpVisualization *p){par=p;}
		void show(){par->show();}
		void close(){par->close();}
		void start(){par->start();}
		void stop(){par->stop();}
		void pause(){par->pause();}
		void reset(){par->reset();}
};

extern "C"{
	qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
	{return new qmpVisualization(api);}
}

#endif // QMPVISUALIZATION_H
