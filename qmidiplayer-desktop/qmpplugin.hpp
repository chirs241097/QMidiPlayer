#ifndef QMPPLUGIN_H
#define QMPPLUGIN_H
#define QMP_MAIN
#include <string>
#include <vector>
#include "../include/qmpcorepublic.hpp"
struct qmpPlugin
{
	std::string name,version,path;
	qmpPluginIntf* interface;
	bool enabled,initialized;
	qmpPlugin(std::string _n,std::string _v,std::string _p,qmpPluginIntf* _i)
	{name=_n;version=_v;path=_p;interface=_i;enabled=initialized=false;}
};
class qmpPluginManager
{
	private:
		std::vector<qmpPlugin> plugins;
	public:
		qmpPluginManager();
		~qmpPluginManager();
		std::vector<qmpPlugin> *getPlugins();
		void scanPlugins();
		void initPlugins();
		void deinitPlugins();
};
#endif // QMPPLUGIN_H
