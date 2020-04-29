#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <cstdio>
#include <cstring>
#include <QDirIterator>
#include "qmpplugin.hpp"
#include "qmpmainwindow.hpp"
#include "qmpsettingswindow.hpp"
qmpPluginAPIImpl* qmpPluginManager::pluginAPI=nullptr;
qmpMainWindow* qmpPluginManager::mainwindow=nullptr;
#ifdef _WIN32
#include <codecvt>
#include <locale>
std::string wstr2str(std::wstring s)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> wsc;
	return wsc.to_bytes(s);
}
void qmpPluginManager::scanPlugins(const std::vector<std::string> &pp)
{
	QDirIterator *dir;
	std::vector<std::wstring> cpluginpaths;
	std::wstring_convert<std::codecvt_utf8<wchar_t>,wchar_t> wsc;
	for(auto p:pp)cpluginpaths.push_back(wsc.from_bytes(p));
	dir=new QDirIterator(QCoreApplication::applicationDirPath()+"/plugins/");
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="dll")
			cpluginpaths.push_back(QCoreApplication::applicationDirPath().toStdWString()+std::wstring(L"/plugins/")+dir->fileName().toStdWString());
	}
	delete dir;
	for(unsigned i=0;i<cpluginpaths.size();++i)
	{
		HMODULE hso=LoadLibraryW(cpluginpaths[i].c_str());
		if(!hso){fprintf(stderr,"Error while loading library: %d\n",GetLastError());continue;}
		FARPROC hndi=GetProcAddress(hso,"qmpPluginGetInterface");
		if(!hndi){fprintf(stderr,"plugin %s doesn't seem to be a qmidiplayer plugin.\n",wstr2str(cpluginpaths[i]).c_str());continue;}
		FARPROC hndiv=GetProcAddress(hso,"qmpPluginGetAPIRev");
		if(!hndiv){fprintf(stderr,"plugin %s is incompatible with this version of qmidiplayer.\n",wstr2str(cpluginpaths[i]).c_str());continue;}
		qmpPluginAPIRevEntry getv=(qmpPluginAPIRevEntry)hndiv;
		if(strcmp(getv(),QMP_PLUGIN_API_REV))
		{fprintf(stderr,"plugin %s is incompatible with this version of qmidiplayer.\n",wstr2str(cpluginpaths[i]).c_str());continue;}
		qmpPluginEntry e=(qmpPluginEntry)hndi;
		qmpPluginIntf* intf=e(pluginAPI);
		plugins.push_back(qmpPlugin(std::string(intf->pluginGetName()),std::string(intf->pluginGetVersion()),wstr2str(cpluginpaths[i]),intf));
	}
}
#else
void qmpPluginManager::scanPlugins(const std::vector<std::string> &pp)
{
	QDirIterator *dir;
	std::vector<std::string> cpluginpaths(pp);
#ifdef NON_PORTABLE
	QString pdir=QString(QT_STRINGIFY(INSTALL_PREFIX))+"/lib/qmidiplayer/";
	dir=new QDirIterator(pdir);
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="so")
			cpluginpaths.push_back((pdir+dir->fileName()).toStdString());
	}
	delete dir;
#endif
	dir=new QDirIterator(QCoreApplication::applicationDirPath()+"/plugins/");
	while(dir->hasNext())
	{
		dir->next();
		if(dir->fileInfo().suffix()=="so")
			cpluginpaths.push_back(QCoreApplication::applicationDirPath().toStdString()+std::string("/plugins/")+dir->fileName().toStdString());
	}
	delete dir;
	for(unsigned i=0;i<cpluginpaths.size();++i)
	{
		void* hso=dlopen(cpluginpaths[i].c_str(),RTLD_LAZY);
		if(!hso){fprintf(stderr,"%s\n",dlerror());continue;}
		void* hndi=dlsym(hso,"qmpPluginGetInterface");
		if(!hndi){fprintf(stderr,"file %s doesn't seem to be a qmidiplayer plugin.\n",cpluginpaths[i].c_str());continue;}
		void* hndiv=dlsym(hso,"qmpPluginGetAPIRev");
		if(!hndiv){fprintf(stderr,"file %s is incompatible with this version of qmidiplayer.\n",cpluginpaths[i].c_str());continue;}
		qmpPluginAPIRevEntry getv=(qmpPluginAPIRevEntry)hndiv;
		if(strcmp(getv(),QMP_PLUGIN_API_REV))
		{fprintf(stderr,"file %s is incompatible with this version of qmidiplayer.\n",cpluginpaths[i].c_str());continue;}
		qmpPluginEntry e=(qmpPluginEntry)hndi;
		qmpPluginIntf* intf=e(pluginAPI);
		plugins.push_back(qmpPlugin(std::string(intf->pluginGetName()),std::string(intf->pluginGetVersion()),std::string(cpluginpaths[i]),intf));
	}
}
#endif
qmpPluginManager::qmpPluginManager()
{
	mainwindow=qmpMainWindow::getInstance();
	pluginAPI=new qmpPluginAPIImpl();
}
qmpPluginManager::~qmpPluginManager()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(plugins[i].initialized)plugins[i].pinterface->deinit();
		delete plugins[i].pinterface;
	}
	mainwindow=nullptr;delete pluginAPI;
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
		fprintf(stderr,"Loaded plugin: %s\n",plugins[i].path.c_str());
		plugins[i].pinterface->init();plugins[i].initialized=true;
	}
}
void qmpPluginManager::deinitPlugins()
{
	for(unsigned i=0;i<plugins.size();++i)
	{
		if(plugins[i].initialized)plugins[i].pinterface->deinit();
		plugins[i].enabled=plugins[i].initialized=false;
	}
}

qmpPluginAPIImpl::qmpPluginAPIImpl(){}
qmpPluginAPIImpl::~qmpPluginAPIImpl(){}
#define qmw qmpPluginManager::mainwindow
uint32_t qmpPluginAPIImpl::getDivision()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getDivision():0;}
uint32_t qmpPluginAPIImpl::getRawTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getRawTempo():0;}
double qmpPluginAPIImpl::getRealTempo()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTempo():0;}
uint32_t qmpPluginAPIImpl::getTimeSig()
{int n,d=0,t;qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentTimeSignature(&n,&t):void(0);for(;t>>=1;++d);return n<<8|d;}
int qmpPluginAPIImpl::getKeySig()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getCurrentKeySignature():0;}
uint32_t qmpPluginAPIImpl::getNoteCount()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getFileNoteCount():0;}
uint32_t qmpPluginAPIImpl::getMaxTick()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getMaxTick():0;}
uint32_t qmpPluginAPIImpl::getCurrentPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getFluid()->getPolyphone():0;}
uint32_t qmpPluginAPIImpl::getMaxPolyphone()
{return qmw&&qmw->getPlayer()?qmw->getFluid()->getMaxPolyphone():0;}
uint32_t qmpPluginAPIImpl::getCurrentTimeStamp()
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getTick():0;}
uint32_t qmpPluginAPIImpl::getCurrentPlaybackPercentage()
{return qmw?qmw->getPlaybackPercentage():0;}
int qmpPluginAPIImpl::getChannelCC(int ch,int cc)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getCC(ch,cc):0;}
int qmpPluginAPIImpl::getChannelPreset(int ch)
{
	uint16_t b;uint8_t p;std::string nm;
	if(qmw&&qmw->getPlayer())
	{
		if(qmw->getPlayer()->getChannelOutputDevice(ch)->getChannelPreset(ch,&b,&p,nm))return p;
		return qmw->getPlayer()->getCC(ch,128);
	}
	return 0;
}
void qmpPluginAPIImpl::playerSeek(uint32_t percentage)
{if(qmw)qmw->playerSeek(percentage);}
double qmpPluginAPIImpl::getPitchBend(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getPitchBend(ch):0;}
bool qmpPluginAPIImpl::getChannelMask(int ch)
{return qmw&&qmw->getPlayer()?qmw->getPlayer()->getChannelMask(ch):false;}
std::string qmpPluginAPIImpl::getTitle()
{return qmw?qmw->getTitle():"";}
std::wstring qmpPluginAPIImpl::getWTitle()
{return qmw?qmw->getWTitle():L"";}
std::string qmpPluginAPIImpl::getChannelPresetString(int ch)
{
	uint16_t b;uint8_t p;char ret[320];ret[0]=0;
	std::string nm;
	if(qmw&&qmw->getPlayer())
	{
		int r=qmw->getPlayer()->getChannelOutputDevice(ch)->getChannelPreset(ch,&b,&p,nm);
		if(!r)
		{
			b=qmw->getPlayer()->getCC(ch,0)<<7|qmw->getPlayer()->getCC(ch,32);
			p=qmw->getPlayer()->getCC(ch,128);
			nm=qmw->getPlayer()->getChannelOutputDevice(ch)->getPresetName(b,p);
		}
		snprintf(ret,320,"%03d:%03d %s",b,p,nm.c_str());
	}
	return std::string(ret);
}
bool qmpPluginAPIImpl::isDarkTheme(){return qmw?qmw->isDarkTheme():false;}
void* qmpPluginAPIImpl::getMainWindow(){return (void*)qmw;}

void qmpPluginAPIImpl::discardCurrentEvent(){if(qmw&&qmw->getPlayer())qmw->getPlayer()->discardCurrentEvent();}
void qmpPluginAPIImpl::commitEventChange(SEvent d){if(qmw&&qmw->getPlayer())qmw->getPlayer()->commitEventChange(d);}
void qmpPluginAPIImpl::callEventReaderCB(SEvent d){if(qmw&&qmw->getPlayer())qmw->getPlayer()->callEventReaderCB(d);}
void qmpPluginAPIImpl::setFuncState(std::string name,bool state){if(qmw)qmw->setFuncState(name,state);}
void qmpPluginAPIImpl::setFuncEnabled(std::string name,bool enable){if(qmw)qmw->setFuncEnabled(name,enable);}

void qmpPluginAPIImpl::registerMidiOutDevice(qmpMidiOutDevice *dev, std::string name)
{qmw->getPlayer()->registerMidiOutDevice(dev,name);}
void qmpPluginAPIImpl::unregisterMidiOutDevice(std::string name)
{qmw->getPlayer()->unregisterMidiOutDevice(name);}
int qmpPluginAPIImpl::registerEventHandlerIntf(ICallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventHandlerCB(cb,userdata);}
void qmpPluginAPIImpl::unregisterEventHandlerIntf(int intfhandle)
{qmw->getPlayer()->unsetEventHandlerCB(intfhandle);}
int qmpPluginAPIImpl::registerEventReaderIntf(ICallBack *cb,void *userdata)
{return qmw->getPlayer()->setEventReaderCB(cb,userdata);}
void qmpPluginAPIImpl::unregisterEventReaderIntf(int intfhandle)
{qmw->getPlayer()->unsetEventReaderCB(intfhandle);}
int qmpPluginAPIImpl::registerUIHook(std::string e,ICallBack* cb,void* userdat)
{return qmw->registerUIHook(e,cb,userdat);}
int qmpPluginAPIImpl::registerUIHook(std::string e,callback_t cb,void* userdat)
{return qmw->registerUIHook(e,cb,userdat);}
void qmpPluginAPIImpl::unregisterUIHook(std::string e,int hook)
{qmw->unregisterUIHook(e,hook);}
void qmpPluginAPIImpl::registerFunctionality(qmpFuncBaseIntf *i,std::string name,std::string desc,const char *icon,int iconlen,bool checkable)
{qmw->registerFunctionality(i,name,desc,icon,iconlen,checkable);}
void qmpPluginAPIImpl::unregisterFunctionality(std::string name)
{qmw->unregisterFunctionality(name);}
int qmpPluginAPIImpl::registerFileReadFinishedHandlerIntf(ICallBack* cb,void* userdata)
{return qmw->getPlayer()->setFileReadFinishedCB(cb,userdata);}
void qmpPluginAPIImpl::unregisterFileReadFinishedHandlerIntf(int intfhandle)
{qmw->getPlayer()->unsetFileReadFinishedCB(intfhandle);}
void qmpPluginAPIImpl::registerFileReader(qmpFileReader* reader,std::string name)
{qmw->getPlayer()->registerReader(reader,name);}
void qmpPluginAPIImpl::unregisterFileReader(std::string name)
{qmw->getPlayer()->unregisterReader(name);}
int qmpPluginAPIImpl::registerEventHandler(callback_t cb,void *userdata,bool post)
{return qmw->getPlayer()->registerEventHandler(cb,userdata,post);}
void qmpPluginAPIImpl::unregisterEventHandler(int id)
{qmw->getPlayer()->unregisterEventHandler(id);}
int qmpPluginAPIImpl::registerEventReadHandler(callback_t cb,void *userdata)
{return qmw->getPlayer()->registerEventReadHandler(cb,userdata);}
void qmpPluginAPIImpl::unregisterEventReadHandler(int id)
{qmw->getPlayer()->unregisterEventReadHandler(id);}
int qmpPluginAPIImpl::registerFileReadFinishHook(callback_t cb,void *userdata)
{return qmw->getPlayer()->registerFileReadFinishHook(cb,userdata);}
void qmpPluginAPIImpl::unregisterFileReadFinishHook(int id)
{qmw->getPlayer()->unregisterFileReadFinishHook(id);}

void qmpPluginAPIImpl::registerOptionInt(std::string tab,std::string desc,std::string key,int min,int max,int defaultval)
{qmw->getSettings()->registerOptionInt(tab,desc,key,min,max,defaultval);}
int qmpPluginAPIImpl::getOptionInt(std::string key){return qmw->getSettings()->getOptionInt(key);}
void qmpPluginAPIImpl::setOptionInt(std::string key,int val){qmw->getSettings()->setOptionInt(key,val);}
void qmpPluginAPIImpl::registerOptionUint(std::string tab,std::string desc,std::string key,unsigned min,unsigned max,unsigned defaultval)
{qmw->getSettings()->registerOptionUint(tab,desc,key,min,max,defaultval);}
unsigned qmpPluginAPIImpl::getOptionUint(std::string key){return qmw->getSettings()->getOptionUint(key);}
void qmpPluginAPIImpl::setOptionUint(std::string key,unsigned val){qmw->getSettings()->setOptionUint(key,val);}
void qmpPluginAPIImpl::registerOptionBool(std::string tab,std::string desc,std::string key,bool defaultval)
{qmw->getSettings()->registerOptionBool(tab,desc,key,defaultval);}
bool qmpPluginAPIImpl::getOptionBool(std::string key){return qmw->getSettings()->getOptionBool(key);}
void qmpPluginAPIImpl::setOptionBool(std::string key,bool val){qmw->getSettings()->setOptionBool(key,val);}
void qmpPluginAPIImpl::registerOptionDouble(std::string tab,std::string desc,std::string key,double min,double max,double defaultval)
{qmw->getSettings()->registerOptionDouble(tab,desc,key,min,max,defaultval);}
double qmpPluginAPIImpl::getOptionDouble(std::string key){return qmw->getSettings()->getOptionDouble(key);}
void qmpPluginAPIImpl::setOptionDouble(std::string key,double val){qmw->getSettings()->setOptionDouble(key,val);}
void qmpPluginAPIImpl::registerOptionString(std::string tab,std::string desc,std::string key,std::string defaultval,bool ispath)
{qmw->getSettings()->registerOptionString(tab,desc,key,defaultval,ispath);}
std::string qmpPluginAPIImpl::getOptionString(std::string key){return qmw->getSettings()->getOptionString(key);}
void qmpPluginAPIImpl::setOptionString(std::string key,std::string val){return qmw->getSettings()->setOptionString(key,val);}
void qmpPluginAPIImpl::registerOptionEnumInt(std::string tab,std::string desc,std::string key,std::vector<std::string> options,int defaultval)
{qmw->getSettings()->registerOptionEnumInt(tab,desc,key,options,defaultval);}
int qmpPluginAPIImpl::getOptionEnumInt(std::string key){return qmw->getSettings()->getOptionEnumInt(key);}
void qmpPluginAPIImpl::setOptionEnumInt(std::string key,int val){return qmw->getSettings()->setOptionEnumInt(key,val);}
