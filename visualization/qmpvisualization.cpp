#include <cstdio>
#include "qmpvisualization.hpp"

void CTestCallBack::callBack(void *callerdata,void *)
{
	if(par->c<3)
	{
		SEventCallBackData* cbd=(SEventCallBackData*)callerdata;
		printf("type %#x p1 %d p2 %d\n",cbd->type,cbd->p1,cbd->p2);
	}
}

qmpVisualization::qmpVisualization(qmpPluginAPI* _api)
{api=_api;}
qmpVisualization::~qmpVisualization()
{api=NULL;}
void qmpVisualization::init()
{
	puts("hello world from test plugin!");
	puts("I'll try to print the first 3 events from the file!");
	cb=new CTestCallBack(this);c=0;
	//!!FIXME: not working properly...
	api->registerEventReaderIntf(cb,NULL);
}
void qmpVisualization::deinit()
{}
const char* qmpVisualization::pluginGetName()
{return "QMidiPlayer Default Visualization Plugin";}
const char* qmpVisualization::pluginGetVersion()
{return "0.7.8";}

//dummy implementations of the api...
uint32_t qmpPluginAPI::getDivision(){return 0;}
uint32_t qmpPluginAPI::getRawTempo(){return 0;}
double qmpPluginAPI::getRealTempo(){return 0;}
uint32_t qmpPluginAPI::getTimeSig(){return 0;}
int qmpPluginAPI::getKeySig(){return 0;}
uint32_t qmpPluginAPI::getNoteCount(){return 0;}
uint32_t qmpPluginAPI::getCurrentPolyphone(){return 0;}
uint32_t qmpPluginAPI::getMaxPolyphone(){return 0;}
uint32_t qmpPluginAPI::getCurrentTimeStamp(){return 0;}
int qmpPluginAPI::registerEventHandlerIntf(IMidiCallBack*,void*){return -1;}
void qmpPluginAPI::unregisterEventHandlerIntf(int){}
int qmpPluginAPI::registerEventReaderIntf(IMidiCallBack*,void*){return -1;}
void qmpPluginAPI::unregisterEventReaderIntf(int){}
