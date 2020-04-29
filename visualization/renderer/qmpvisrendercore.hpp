#ifndef QMPVISRENDERCORE_HPP
#define QMPVISRENDERCORE_HPP

#include <cstddef>

#include <QObject>
#include "qmpcorepublic.hpp"

class qmpPluginAPIStub;
class CMidiPlayer;
class qmpSettingsRO;

class QProcess;

class qmpVisRenderCore:public QObject
{
	Q_OBJECT
public:
	qmpVisRenderCore();
	void loadVisualizationLibrary();
	void unloadVisualizationLibrary();
	void setMIDIFile(const char* url);
	void startRender();

	qmpSettingsRO* settings(){return msettings;}

private:
	qmpPluginIntf *vp;
	qmpFuncBaseIntf *vf;
	callback_t startcb;
	void *mp;
	qmpPluginAPIStub *api;
	CMidiPlayer *player;
	qmpSettingsRO *msettings;
	QProcess *ffmpegproc;
	typedef qmpPluginIntf*(*GetInterface_func)(qmpPluginAPI*);
	typedef void(*SwitchMode_func)(void(*frameCallback)(void*,size_t),bool hidewindow);

	friend class qmpPluginAPIStub;
	static void framefunc(void* px,size_t sz);
	static qmpVisRenderCore *inst;
};

#endif // QMPVISRENDERCORE_HPP
