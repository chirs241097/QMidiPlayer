#include "qmpvisrendercore.hpp"
#include "qmppluginapistub.hpp"
#include "qmpsettingsro.hpp"
#include "qmpmidiplay.hpp"
#include "qmpcorepublic.hpp"

#include <cassert>
#include <dlfcn.h>

#include <QProcess>
#include <QDebug>
#include <QThread>
qmpVisRenderCore *qmpVisRenderCore::inst=nullptr;

qmpVisRenderCore::qmpVisRenderCore():QObject(nullptr)
{
	inst=this;
	player=new CMidiPlayer();
	api=new qmpPluginAPIStub(this);
	msettings=new qmpSettingsRO();
	msettings->registerOptionEnumInt("MIDI","Text encoding","Midi/TextEncoding",{"Unicode","Big5","Big5-HKSCS","CP949","EUC-JP","EUC-KR","GB18030","KOI8-R","KOI8-U","Macintosh","Shift-JIS"},0);
}

void qmpVisRenderCore::loadVisualizationLibrary()
{
	mp=dlopen("libvisualization.so",RTLD_LAZY);
	if(!mp)fprintf(stderr,"failed to load visualization module!\n");
	GetInterface_func getintf=reinterpret_cast<GetInterface_func>(dlsym(mp,"qmpPluginGetInterface"));
	SwitchMode_func switchmode=reinterpret_cast<SwitchMode_func>(dlsym(mp,"switchToRenderMode"));
	vf=nullptr;
	vp=getintf(api);
	switchmode(&qmpVisRenderCore::framefunc,false);
	vp->init();
	msettings->load("/home/chrisoft/.config/qmprc");
}

void qmpVisRenderCore::unloadVisualizationLibrary()
{
	vp->deinit();
	dlclose(mp);
}

void qmpVisRenderCore::setMIDIFile(const char *url)
{
	player->playerLoadFile(url);
}

void qmpVisRenderCore::startRender()
{
	assert(vf);
	ffmpegproc=new QProcess();
	ffmpegproc->setProgram("ffmpeg");
	QStringList arguments;
	arguments
		<<"-f"<<"rawvideo"
		<<"-pixel_format"<<"rgba"
		<<"-video_size"<<"1600x900"
		<<"-framerate"<<"60"
		<<"-i"<<"pipe:";
	arguments
		<<"-vf"<<"vflip"
		<<"-pix_fmt"<<"yuv420p"
		<<"-c:v"<<"libx264"
		<<"-preset"<<"fast"
		<<"-crf"<<"22";
	arguments<<"output.mp4";
	ffmpegproc->setArguments(arguments);
	ffmpegproc->start();
	connect(ffmpegproc,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
			[this](int x,QProcess::ExitStatus){qDebug("%d",x);qDebug()<<this->ffmpegproc->readAllStandardError();});
	vf->show();
	startcb(nullptr,nullptr);
}

void qmpVisRenderCore::framefunc(void *px, size_t sz)
{
	if(sz)
	{
		inst->ffmpegproc->write((const char*)px,sz);
		while(inst->ffmpegproc->bytesToWrite()>1<<26)
		{
			inst->ffmpegproc->waitForBytesWritten();
			QThread::yieldCurrentThread();
		}
	}
	else
		inst->ffmpegproc->closeWriteChannel();
}
