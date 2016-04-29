#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#endif
#include <cstdio>
#include <cstring>
#include "qmpplugin.hpp"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"
qmpPluginAPI pluginAPI;
qmpMainWindow* qmw;
qmpSettingsWindow* qsw;
#ifdef _WIN32
void qmpPluginManager::scanPlugins()
{
	HANDLE dir;
	std::vector<std::string> cpluginpaths;
	//FindFirstFile, FindNextFile, FindClose
}
#else
void qmpPluginManager::scanPlugins()
{
	DIR *dir;
	struct dirent *file;
	std::vector<std::string> cpluginpaths;
	if((dir=opendir("/usr/lib/qmidiplayer/")))
	{
		while((file=readdir(dir)))
		if(strcmp(file->d_name+strlen(file->d_name)-3,".so")==0)
			cpluginpaths.push_back(std::string("/usr/lib/qmidiplayer/")+std::string(file->d_name));
		closedir(dir);
	}
	if((dir=opendir("./")))
	{
		while((file=readdir(dir)))
		if(strcmp(file->d_name+strlen(file->d_name)-3,".so")==0)
			cpluginpaths.push_back(std::string("./")+std::string(file->d_name));
		closedir(dir);
	}
	for(unsigned i=0;i<cpluginpaths.size();++i)
	{
		void* hso=dlopen(cpluginpaths[i].c_str(),RTLD_LAZY);
		if(!hso){fprintf(stderr,"%s\n",dlerror());continue;}
		void* hndi=dlsym(hso,"qmpPluginGetInterface");
		if(!hndi)continue;
		qmpPluginEntry e=(qmpPluginEntry)hndi;
		qmpPluginIntf* intf=e(&pluginAPI);
		plugins.push_back(qmpPlugin(std::string(intf->pluginGetName()),std::string(intf->pluginGetVersion()),std::string(cpluginpaths[i]),intf));
	}
}
#endif
qmpPluginManager::qmpPluginManager()
{
	qmw=qmpMainWindow::getInstance();
	qsw=qmw->getSettingsWindow();
}
qmpPluginManager::~qmpPluginManager()
{
	for(unsigned i=0;i<plugins.size();++i)delete plugins[i].interface;
	qmw=NULL;qsw=NULL;
}
std::vector<qmpPlugin> *qmpPluginManager::getPlugins()
{
	return &plugins;
}
void qmpPluginManager::initPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(!plugins[i].enabled)continue;
		printf("Loaded plugin: %s\n",plugins[i].path.c_str());
		plugins[i].interface->init();
	}
}
void qmpPluginManager::deinitPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
		plugins[i].interface->deinit();
}

uint32_t qmpPluginAPI::getDivision()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getDivision():0;}
uint32_t qmpPluginAPI::getRawTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getRawTempo():0;}
double qmpPluginAPI::getRealTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTempo():0;}
uint32_t qmpPluginAPI::getTimeSig()
{int n,d=0,t;qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentTimeSignature(&n,&t):void(0);for(;t>>=1;++d);return n<<8|d;}
int qmpPluginAPI::getKeySig()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentKeySignature():0;}
uint32_t qmpPluginAPI::getNoteCount()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getFileNoteCount():0;}
uint32_t qmpPluginAPI::getCurrentPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getPolyphone():0;}
uint32_t qmpPluginAPI::getMaxPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getMaxPolyphone():0;}
uint32_t qmpPluginAPI::getCurrentTimeStamp()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTick():0;}
double qmpPluginAPI::getPitchBend(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getPitchBend(ch):0;}
bool qmpPluginAPI::getChannelMask(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getChannelMask(ch):false;}
std::string qmpPluginAPI::getTitle()
{return qmw?qmw->getTitle():"";}

int qmpPluginAPI::registerEventHandlerIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventHandlerCB(cb,userdata);}
void qmpPluginAPI::unregisterEventHandlerIntf(int intfhandle)
{qmw->getPlayer()->unsetEventHandlerCB(intfhandle);}
int qmpPluginAPI::registerEventReaderIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventReaderCB(cb,userdata);}
void qmpPluginAPI::unregisterEventReaderIntf(int intfhandle)
{qmw->getPlayer()->unsetEventReaderCB(intfhandle);}
int qmpPluginAPI::registerVisualizationIntf(qmpVisualizationIntf* intf)
{return qmw->registerVisualizationIntf(intf);}
void qmpPluginAPI::unregisterVisualizationIntf(int intfhandle)
{qmw->unregisterVisualizationIntf(intfhandle);}
void qmpPluginAPI::registerOptionInt(std::string,std::string,int,int,int){}
int qmpPluginAPI::getOptionInt(std::string){return 0;}
void qmpPluginAPI::registerOptionUint(std::string,std::string,unsigned,unsigned,unsigned){}
unsigned qmpPluginAPI::getOptionUint(std::string){return 0;}
void qmpPluginAPI::registerOptionBool(std::string,std::string,bool){}
bool qmpPluginAPI::getOptionBool(std::string){return false;}
void qmpPluginAPI::registerOptionDouble(std::string,std::string,double,double,double){}
double qmpPluginAPI::getOptionDouble(std::string){return 0;}
void qmpPluginAPI::registerOptionString(std::string,std::string,std::string){}
std::string qmpPluginAPI::getOptionString(std::string){return "";}
