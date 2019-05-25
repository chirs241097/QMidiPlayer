#ifndef SIMPLEVISUALIZATION_HPP
#define SIMPLEVISUALIZATION_HPP

#include "../include/qmpcorepublic.hpp"

class qmpKeyboardWindow;
class qmpSimpleVisualization:public qmpPluginIntf,public qmpFuncBaseIntf
{
	private:
		qmpPluginAPI* api;
		qmpKeyboardWindow *p;
		int uihs;
	public:
		qmpSimpleVisualization(qmpPluginAPI* _api);
		void show();
		void close();
		void init();
		void deinit();
		const char* pluginGetName();
		const char* pluginGetVersion();
};

extern "C"{
	EXPORTSYM qmpPluginIntf* qmpPluginGetInterface(qmpPluginAPI* api)
	{return new qmpSimpleVisualization(api);}
	EXPORTSYM const char* qmpPluginGetAPIRev()
	{return QMP_PLUGIN_API_REV;}
}

#endif // SIMPLEVISUALIZATION_HPP
