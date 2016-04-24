#ifndef QMPVISUALIZATION_H
#define QMPVISUALIZATION_H

#include "../include/qmpcorepublic.hpp"

class qmpVisualization;
class CTestCallBack:public IMidiCallBack
{
	private:
		qmpVisualization *par;
	public:
		CTestCallBack(qmpVisualization *_par){par=_par;}
		void callBack(void *callerdata,void *userdata);
};
class qmpVisualization:public qmpPluginIntf
{
	friend class CTestCallBack;
	private:
		qmpPluginAPI* api;
		int c;
		CTestCallBack* cb;
	public:
		qmpVisualization(qmpPluginAPI* _api);
		~qmpVisualization();
		void init();
		void deinit();
		const char* pluginGetName();
		const char* pluginGetVersion();
};

extern "C"{
	qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
	{return new qmpVisualization(api);}
}

#endif // QMPVISUALIZATION_H
