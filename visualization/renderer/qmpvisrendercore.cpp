#include "qmpvisrendercore.hpp"
#include "qmppluginapistub.hpp"
#include "qmpsettingsro.hpp"
#include "qmpmidiplay.hpp"
#include "qmpcorepublic.hpp"

#include <cassert>
#include <dlfcn.h>

#include <QProcess>
#include <QCommandLineParser>
#include <QDebug>
#include <QThread>
qmpVisRenderCore *qmpVisRenderCore::inst=nullptr;

qmpVisRenderCore::qmpVisRenderCore(QCommandLineParser *_clp):QObject(nullptr),clp(_clp)
{
	inst=this;
	player=new CMidiPlayer();
	api=new qmpPluginAPIStub(this);
	msettings=new qmpSettingsRO();
	msettings->registerOptionEnumInt("MIDI","Text encoding","Midi/TextEncoding",{"Unicode","Big5","Big5-HKSCS","CP949","EUC-JP","EUC-KR","GB18030","KOI8-R","KOI8-U","Macintosh","Shift-JIS"},0);
}

bool qmpVisRenderCore::loadVisualizationLibrary()
{
	mp=dlopen("libvisualization.so",RTLD_LAZY);
	if(!mp)
	{
		fprintf(stderr,"failed to load the visualization module!\n");
		return false;
	}
	GetInterface_func getintf=reinterpret_cast<GetInterface_func>(dlsym(mp,"qmpPluginGetInterface"));
	SwitchMode_func switchmode=reinterpret_cast<SwitchMode_func>(dlsym(mp,"switchToRenderMode"));
	vf=nullptr;
	vp=getintf(api);
	switchmode(&qmpVisRenderCore::framefunc,!clp->isSet("show-window"));
	vp->init();
	resetcb(nullptr,nullptr);
	return true;
}

void qmpVisRenderCore::unloadVisualizationLibrary()
{
	vp->deinit();
	dlclose(mp);
}

void qmpVisRenderCore::loadSettings()
{
	if(clp->isSet("config"))
		msettings->load(clp->value("config").toStdString().c_str());
	for(auto &o:clp->values("option"))
	{
		int sp=o.indexOf('=');
		if(!~sp)
		{
			qDebug("invalid option pair: %s",o.toStdString().c_str());
			continue;
		}
		QString key=o.left(sp);
		QString value=o.mid(sp+1);
		msettings->setopt(key.toStdString(),value.toStdString());
	}
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
	arguments.append(split_arguments(clp->value("ffmpeg-pre-args")));
	arguments
		<<"-f"<<"rawvideo"
		<<"-pixel_format"<<"rgba"
		<<"-video_size"<<QString("%1x%2").arg(msettings->getOptionInt("Visualization/wwidth")).arg(msettings->getOptionInt("Visualization/wheight"))
		<<"-framerate"<<QString::number(msettings->getOptionInt("Visualization/tfps"))
		<<"-i"<<"pipe:";
	arguments.append(split_arguments(clp->value("ffmpeg-args")));
	arguments<<clp->value("output-file");
	ffmpegproc->setArguments(arguments);
	QMetaObject::Connection frameconn=connect(this,&qmpVisRenderCore::frameRendered,this,
	[this,&frameconn](void* px,size_t sz,uint32_t c,uint32_t t)
	{
		if(sz)
		{
			if(!ffmpegproc->isOpen())return;
			ffmpegproc->write(static_cast<const char*>(px),static_cast<qint64>(sz));
			while(ffmpegproc->bytesToWrite()>1<<26)
				ffmpegproc->waitForBytesWritten();
		}
		fprintf(stderr,"Rendered tick %u of %u, %.2f%% done.\r",c,t,100.*c/t);
		if(c>t)
		{
			this->ffmpegproc->closeWriteChannel();
			disconnect(frameconn);
			qApp->exit(0);
		}
	},Qt::ConnectionType::BlockingQueuedConnection);
	connect(ffmpegproc,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
			[this,&frameconn](int x,QProcess::ExitStatus st){
				qDebug("%s",this->ffmpegproc->readAllStandardError().data());
				disconnect(frameconn);
				if(x||st==QProcess::ExitStatus::CrashExit)
					qApp->exit(1);
				else
					qApp->exit(0);
	});
	QMetaObject::invokeMethod(this,[this](){
		ffmpegproc->start();
		vf->show();
		startcb(nullptr,nullptr);
	},Qt::ConnectionType::QueuedConnection);
}

QStringList qmpVisRenderCore::split_arguments(QString a)
{
	QStringList ret;
	QString buf;
	bool escaped=false;
	for(int i=0;i<a.length();++i)
	{
		if(a[i]=='\\')
		{
			if(escaped)
			{
				buf+='\\';
				escaped=false;
			}
			else escaped=true;
		}
		else if(a[i]==' ')
		{
			if(escaped)buf+=' ';
			else
			{
				ret.append(buf);
				buf.clear();
			}
			escaped=false;
		}
		else
		{
			if(escaped)
			{
				buf+='\\';
				escaped=false;
			}
			buf+=a[i];
		}
	}
	if(buf.length())
		ret.append(buf);
	return ret;
}

void qmpVisRenderCore::framefunc(void *px, size_t sz,uint32_t curf,uint32_t totf)
{
	emit inst->frameRendered(px,sz,curf,totf);
}
