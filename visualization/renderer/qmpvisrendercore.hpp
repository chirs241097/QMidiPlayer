#ifndef QMPVISRENDERCORE_HPP
#define QMPVISRENDERCORE_HPP

#include <cstddef>

#include <QObject>
#include <QVariant>
#include <QMap>
#include "qmpcorepublic.hpp"

class qmpPluginAPIStub;
class CMidiPlayer;
class qmpSettingsRO;
class QCommandLineParser;

class QProcess;

class qmpVisRenderCore:public QObject
{
	Q_OBJECT
public:
	qmpVisRenderCore(QCommandLineParser *_clp);
	bool loadVisualizationLibrary();
	void unloadVisualizationLibrary();
	void loadSettings();
	void setMIDIFile(const char* url);
	void startRender();

	qmpSettingsRO* settings(){return msettings;}

signals:
	void frameRendered(void* px,size_t sz,uint32_t current_tick,uint32_t total_ticks);

private:
	qmpPluginIntf *vp;
	qmpFuncBaseIntf *vf;
	callback_t startcb;
	callback_t resetcb;
	void *mp;
	qmpPluginAPIStub *api;
	CMidiPlayer *player;
	qmpSettingsRO *msettings;
	QProcess *rxproc;
	QMap<QChar,QVariant> subst;
	QCommandLineParser *clp;
	QStringList process_arguments(QString a, QMap<QChar,QVariant> subst);
	int frameno;
	bool oneshot;
	QMetaObject::Connection frameconn;
	typedef qmpPluginIntf*(*GetInterface_func)(qmpPluginAPI*);
	typedef void(*SwitchMode_func)(void(*frameCallback)(void*,size_t,uint32_t,uint32_t),bool hidewindow);

	friend class qmpPluginAPIStub;
	static void framefunc(void* px, size_t sz, uint32_t curf, uint32_t totf);
	static qmpVisRenderCore *inst;
};

#endif // QMPVISRENDERCORE_HPP
