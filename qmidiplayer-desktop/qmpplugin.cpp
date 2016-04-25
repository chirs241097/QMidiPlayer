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
	qmw=NULL;qsw=NULL;
}
void qmpPluginManager::initPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		printf("Loaded plugin: %s\n",plugins[i].path.c_str());
		plugins[i].interface->init();
	}
}

uint32_t qmpPluginAPI::getDivision()
{return qmw->getPlayer()->getDivision();}
uint32_t qmpPluginAPI::getRawTempo()
{return qmw->getPlayer()->getRawTempo();}
double qmpPluginAPI::getRealTempo()
{return qmw->getPlayer()->getTempo();}
uint32_t qmpPluginAPI::getTimeSig()
{int n,d=0,t;qmw->getPlayer()->getCurrentTimeSignature(&n,&t);for(;t>>=1;++d);return n<<8|d;}
int qmpPluginAPI::getKeySig()
{return qmw->getPlayer()->getCurrentKeySignature();}
uint32_t qmpPluginAPI::getNoteCount()
{return qmw->getPlayer()->getFileNoteCount();}
uint32_t qmpPluginAPI::getCurrentPolyphone()
{return qmw->getPlayer()->getPolyphone();}
uint32_t qmpPluginAPI::getMaxPolyphone()
{return qmw->getPlayer()->getMaxPolyphone();}
uint32_t qmpPluginAPI::getCurrentTimeStamp()
{return qmw->getPlayer()->getTCeptr();}
int qmpPluginAPI::registerEventHandlerIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventHandlerCB(cb,userdata);}
void qmpPluginAPI::unregisterEventHandlerIntf(int intfhandle)
{qmw->getPlayer()->unsetEventHandlerCB(intfhandle);}
int qmpPluginAPI::registerEventReaderIntf(IMidiCallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventReaderCB(cb,userdata);}
void qmpPluginAPI::unregisterEventReaderIntf(int intfhandle)
{qmw->getPlayer()->unsetEventReaderCB(intfhandle);}
int qmpPluginAPI::registerVisualizationIntf(qmpVisualizationIntf*){return 0;}
void qmpPluginAPI::unregisterVisualizationIntf(int){}
void qmpPluginAPI::registerOptionInt(std::string,std::string,int){}
int qmpPluginAPI::getOptionInt(std::string){return 0;}
void qmpPluginAPI::registerOptionDouble(std::string,std::string,double){}
double qmpPluginAPI::getOptionDouble(std::string){return 0;}
void qmpPluginAPI::registerOptionString(std::string,std::string,std::string){}
std::string qmpPluginAPI::getOptionString(std::string){return "";}
