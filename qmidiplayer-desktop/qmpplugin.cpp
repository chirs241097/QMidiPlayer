#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <dirent.h>
#endif
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

uint32_t qmpPluginAPI::getDivision()
{return qmw->getPlayer()->getDivision();}
