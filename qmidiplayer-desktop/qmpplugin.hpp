#ifndef QMPPLUGIN_H
#define QMPPLUGIN_H
#define QMP_MAIN
#include <string>
#include <vector>
#define QMP_MAIN
#include "../include/qmpcorepublic.hpp"
struct qmpPlugin
{
	std::string name,version,path;
	qmpPluginIntf* pinterface;
	bool enabled,initialized;
	qmpPlugin(std::string _n,std::string _v,std::string _p,qmpPluginIntf* _i)
	{name=_n;version=_v;path=_p;pinterface=_i;enabled=initialized=false;}
};
class qmpMainWindow;
class qmpSettings;
class qmpPluginManager
{
	private:
		std::vector<qmpPlugin> plugins;
		static qmpPluginAPI* pluginAPI;
		static qmpMainWindow* mainwindow;
		static qmpSettings* settings;
	public:
		qmpPluginManager();
		~qmpPluginManager();
		std::vector<qmpPlugin> *getPlugins();
		void scanPlugins(const std::vector<std::string> &pp);
		void initPlugins();
		void deinitPlugins();
	friend class qmpPluginAPI;
	friend class qmpMainWindow;
};
#endif // QMPPLUGIN_H
