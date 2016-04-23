#ifndef QMPPLUGIN_H
#define QMPPLUGIN_H
#include <string>
#include <vector>
#include "../include/qmpcorepublic.hpp"
struct qmpPlugin
{
	std::string name,version,path;
	qmpPluginIntf* interface;
	bool enabled;
	qmpPlugin(std::string _n,std::string _v,std::string _p,qmpPluginIntf* _i)
	{name=_n;version=_v;path=_p;interface=_i;enabled=false;}
};
class qmpPluginManager
{
	private:
		std::vector<qmpPlugin> plugins;
	public:
		qmpPluginManager();
		~qmpPluginManager();
		std::vector<qmpPlugin> getPlugins() const;
		void scanPlugins();
		void initPlugins();
		void deinitPlugins();
};
#endif // QMPPLUGIN_H
