#include "simplevisualization.hpp"
#include "qmpkeyboardwindow.hpp"

qmpSimpleVisualization::qmpSimpleVisualization(qmpPluginAPI *_api){api=_api;}
void qmpSimpleVisualization::show(){p->show();}
void qmpSimpleVisualization::close(){p->close();}
void qmpSimpleVisualization::init()
{
	api->registerFunctionality(this,"Keyboard","Keyboard",api->isDarkTheme()?":/img/visualization_i.svg":":/img/visualization.svg",0,true);
	p=new qmpKeyboardWindow(api,NULL);
	uihs=api->registerUIHook("main.stop",qmpSimpleVisualization::cbstop,(void*)this);
}
void qmpSimpleVisualization::deinit()
{
	if(!api)return;close();
	api->unregisterFunctionality("Keyboard");
	api->unregisterUIHook("main.stop",uihs);
	delete p;
}
const char* qmpSimpleVisualization::pluginGetName()
{return "QMidiPlayer Simple Visualization Plugin";}
const char* qmpSimpleVisualization::pluginGetVersion()
{return "0.8.6";}

void qmpSimpleVisualization::cbstop(void*,void* usrd)
{
	qmpSimpleVisualization *v=(qmpSimpleVisualization*)usrd;
	v->p->resetAll();
}
