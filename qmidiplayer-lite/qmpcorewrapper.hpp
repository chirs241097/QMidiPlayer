#ifndef QMPCOREWRAPPER_H
#define QMPCOREWRAPPER_H
#include <QObject>
#include <QUrl>
#include <thread>
#include <fluidsynth.h>
#include "../core/qmpmidiplay.hpp"
class CQMPCoreWrapper:public QObject
{
	Q_OBJECT
private:
	CMidiPlayer *mp;
	std::thread *playerTh;
	int curprog;
public:
	explicit CQMPCoreWrapper(QObject* parent=0):QObject(parent)
	{
		mp=new CMidiPlayer();
	}
	~CQMPCoreWrapper(){delete mp;}
	Q_INVOKABLE void initFluidSynth(QUrl sfpath)
	{
		mp->fluid()->setOptStr("audio.driver","pulseaudio");
		mp->fluid()->deviceInit();
		mp->fluid()->loadSFont(sfpath.toLocalFile().toStdString().c_str());
		for(int i=0;i<16;++i)
		mp->setChannelOutput(i,0);
	}
	Q_INVOKABLE void deinitFluidSynth()
	{
		mp->fluid()->deviceDeinit();
	}
	Q_INVOKABLE void loadFile(QUrl file)
	{
		mp->playerLoadFile(file.toLocalFile().toStdString().c_str());
		mp->playerInit();curprog=0;
	}
	Q_INVOKABLE void playFile()
	{
		playerTh=new std::thread(&CMidiPlayer::playerThread,mp);
	}
	Q_INVOKABLE void stop()
	{
		mp->playerDeinit();playerTh->join();delete playerTh;
		mp->playerPanic();
	}
	Q_INVOKABLE int getProgress()
	{
		while(!mp->isFinished()&&mp->getTCeptr()>mp->getStamp(curprog)
			  &&curprog<=100)
			++curprog;
		return curprog;
	}
	Q_INVOKABLE void panic(){mp->playerPanic();}
	Q_INVOKABLE void setTCeptr(int perct)
	{
		mp->setTCeptr(mp->getStamp(perct),perct);curprog=perct;
	}
};
#endif // QMPCOREWRAPPER_H
