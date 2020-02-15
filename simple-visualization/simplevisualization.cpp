#include "simplevisualization.hpp"
#include "qmpkeyboardwindow.hpp"

qmpSimpleVisualization::qmpSimpleVisualization(qmpPluginAPI *_api){api=_api;}
void qmpSimpleVisualization::show(){p->show();}
void qmpSimpleVisualization::close(){p->close();}
void qmpSimpleVisualization::init()
{
	api->registerFunctionality(this,"Keyboard","Keyboard",api->isDarkTheme()?":/img/visualization_i.svg":":/img/visualization.svg",0,true);
	for(int i=0;i<16;++i)
	{
		api->registerOptionUint("","","Keyboard/acolor"+std::to_string(i),0,0xffffff,0xffff66cc);
		api->registerOptionUint("","","Keyboard/bcolor"+std::to_string(i),0,0xffffff,0xff66ccff);
	}
	p=new qmpKeyboardWindow(api,(QWidget*)api->getMainWindow());
	auto refreshfn=[this](const void*,void*){this->p->resetAll();};
	uihs=api->registerUIHook("main.stop",refreshfn,nullptr);
	uihsk=api->registerUIHook("main.seek",refreshfn,nullptr);
	uihsk=api->registerUIHook("preset.set",refreshfn,nullptr);
	uihsk=api->registerUIHook("channel.ccchange",refreshfn,nullptr);
}
void qmpSimpleVisualization::deinit()
{
	if(!api)return;close();
	api->unregisterFunctionality("Keyboard");
	api->unregisterUIHook("main.stop",uihs);
	api->unregisterUIHook("main.seek",uihsk);
	delete p;
}
const char* qmpSimpleVisualization::pluginGetName()
{return "QMidiPlayer Simple Visualization Plugin";}
const char* qmpSimpleVisualization::pluginGetVersion()
{return PLUGIN_VERSION;}
