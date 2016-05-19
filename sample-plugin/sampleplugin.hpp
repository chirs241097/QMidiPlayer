#ifndef SAMPLEPLUGIN_H
#define SAMPLEPLUGIN_H

#include "../include/qmpcorepublic.hpp"

class qmpSamplePlugin:public qmpPluginIntf
{
	private:
		qmpPluginAPI* api;
	public:
		qmpSamplePlugin(qmpPluginAPI* _api);
		~qmpSamplePlugin();
		void init();
		void deinit();
		const char* pluginGetName();
		const char* pluginGetVersion();
};

extern "C"{
	qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
	{return new qmpSamplePlugin(api);}
}

#endif // SAMPLEPLUGIN_H
