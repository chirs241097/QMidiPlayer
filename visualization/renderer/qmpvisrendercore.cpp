#include "qmpvisrendercore.hpp"
#include "qmppluginapistub.hpp"
#include "qmpsettingsro.hpp"
#include "qmpmidiplay.hpp"
#include "qmpcorepublic.hpp"

#include <algorithm>
#include <cassert>
#ifdef _WIN32
#include <windows.h>
#define dlopen(a,b) LoadLibraryW(a)
#define dlsym GetProcAddress
#define dlclose FreeLibrary
#else
#include <dlfcn.h>
#endif

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
	frameno=0;
	msettings->registerOptionEnumInt("MIDI","Text encoding","Midi/TextEncoding",{"Unicode","Big5","Big5-HKSCS","CP949","EUC-JP","EUC-KR","GB18030","KOI8-R","KOI8-U","Macintosh","Shift-JIS"},0);
}

bool qmpVisRenderCore::loadVisualizationLibrary()
{
#ifdef _WIN32
	std::vector<std::wstring> libpath={
		QCoreApplication::applicationDirPath().toStdWString()+L"/plugins/libvisualization.dll"
		L"libvisualization.dll",
		L"../libvisualization.dll"//for debugging only...?
	};
#else
	std::vector<std::string> libpath={
		QCoreApplication::applicationDirPath().toStdString()+"/plugins/libvisualization.so"
		QT_STRINGIFY(INSTALL_PREFIX)+std::string("/lib/qmidiplayer/libvisualization.so"),
		"../libvisualization.so"//for debugging only
	};
#endif
	for(auto&l:libpath)
	{
		mp=dlopen(l.c_str(),RTLD_LAZY);
		if(mp)break;
	}
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
	if(clp->isSet("list-options"))
	{
		msettings->listopt();
		exit(0);
	}
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
	subst={
		{'w',QString::number(msettings->getOptionInt("Visualization/wwidth"))},
		{'h',QString::number(msettings->getOptionInt("Visualization/wheight"))},
		{'r',QString::number(msettings->getOptionInt("Visualization/tfps"))},
		{'i',
			QStringList()
				<<"-f"<<"rawvideo"
				<<"-pixel_format"<<"rgba"
				<<"-video_size"<<QString("%1x%2").arg(msettings->getOptionInt("Visualization/wwidth")).arg(msettings->getOptionInt("Visualization/wheight"))
				<<"-framerate"<<QString::number(msettings->getOptionInt("Visualization/tfps"))
				<<"-i"<<"pipe:"
		},
		{'o',clp->value("output-file")}
	};
	if(clp->value("receiver-execution")=="per-frame")
	{
		subst['o']=clp->value("output-file").replace("%f",QString("%1").arg(frameno,6,10,QChar('0')));
		oneshot=false;
	}
	else
	{
		oneshot=true;
		if(clp->value("receiver-execution")!="one-shot")
			qWarning("Invalid value set for --receiver-execution. Using default value.");
	}
	rxproc=new QProcess();
	QStringList arguments=process_arguments(clp->value("receiver"),subst);
	assert(arguments.length()>0);
	rxproc->setProgram(arguments.front());
	arguments.pop_front();
	rxproc->setArguments(arguments);
	frameconn=connect(this,&qmpVisRenderCore::frameRendered,this,
	[this](void* px,size_t sz,uint32_t c,uint32_t t)
	{
		if(sz)
		{
			if(!oneshot)
			{
				subst['f']=QString("%1").arg(frameno,6,10,QChar('0'));
				subst['o']=clp->value("output-file").replace("%f",QString("%1").arg(frameno,6,10,QChar('0')));
				frameno++;
				QStringList arguments=process_arguments(clp->value("receiver"),subst);
				arguments.pop_front();
				rxproc->setArguments(arguments);
				rxproc->start();
				rxproc->waitForStarted();
			}
			if(!rxproc->isOpen())return;
			rxproc->write(static_cast<const char*>(px),static_cast<qint64>(sz));
			while(rxproc->bytesToWrite()>(oneshot?(1<<26):0))
				rxproc->waitForBytesWritten();
			if(!oneshot)
			{
				rxproc->closeWriteChannel();
				rxproc->waitForFinished(-1);
			}
		}
		fprintf(stderr,"Rendered tick %u of %u, %.2f%% done.\r",c,t,std::min(100.,100.*c/t));
		if(c>t)
		{
			this->rxproc->closeWriteChannel();
			disconnect(frameconn);
			qApp->exit(0);
		}
	},Qt::ConnectionType::BlockingQueuedConnection);
	connect(rxproc,QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
			[this](int x,QProcess::ExitStatus st){
				qDebug("%s",this->rxproc->readAllStandardError().data());
				if(oneshot)
				{
					disconnect(frameconn);
					if(x||st==QProcess::ExitStatus::CrashExit)
						qApp->exit(1);
					else
						qApp->exit(0);
				}
	});
	QMetaObject::invokeMethod(this,[this](){
		if(oneshot)
			rxproc->start();
		vf->show();
		startcb(nullptr,nullptr);
	},Qt::ConnectionType::QueuedConnection);
}

QStringList qmpVisRenderCore::process_arguments(QString a,QMap<QChar,QVariant> subst)
{
	QStringList ret;
	QString buf;
	bool escaped=false;
	bool substi=false;
	for(int i=0;i<a.length();++i)
	{
		if(a[i]=='%')
		{
			if(escaped)
			{
				buf+='%';
				escaped=false;
			}
			else if(substi)
			{
				buf+='%';
				substi=false;
			}
			else substi=true;
		}
		else if(a[i]=='\\')
		{
			if(substi)
			{
				buf+='%';
				substi=false;
			}
			if(escaped)
			{
				buf+='\\';
				escaped=false;
			}
			else escaped=true;
		}
		else if(a[i]==' ')
		{
			if(substi)
			{
				buf+='%';
				substi=false;
			}
			if(escaped)buf+=' ';
			else
			{
				if(buf.length())
					ret.append(buf);
				buf.clear();
			}
			escaped=false;
		}
		else
		{
			if(substi&&subst.contains(a[i]))
			{
				if(subst[a[i]].canConvert(QMetaType::QString))
					buf+=subst[a[i]].toString();
				else
				{
					if(buf.length())
					{
						ret.append(buf);
						buf.clear();
					}
					for(auto &it:subst[a[i]].toStringList())
						ret.append(it);
				}
				substi=false;
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
	}
	if(buf.length())
		ret.append(buf);
	return ret;
}

void qmpVisRenderCore::framefunc(void *px, size_t sz,uint32_t curf,uint32_t totf)
{
	emit inst->frameRendered(px,sz,curf,totf);
}
