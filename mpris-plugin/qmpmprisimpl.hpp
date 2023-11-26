#ifndef QMPMPRISIMPL_HPP
#define QMPMPRISIMPL_HPP

#include "qmprisdbusinterface.hpp"

class qmpPluginAPI;
class qmpMainWindow;

class QMPPlayer : public QMPrisPlayer
{
public:
    explicit QMPPlayer(qmpPluginAPI *_api, QObject *parent=nullptr);

    QString getPlaybackStatus();
    QString getLoopStatus();
    double getRate();
    bool getShuffle();
    QVariantMap getMetadata();
    //double getVolume();
    qlonglong getPosition();
    //double getMinimumRate();
    //double getMaximumRate();
    bool getCanGoNext();
    bool getCanGoPrevious();
    bool getCanPlay();
    bool getCanPause();
    bool getCanSeek();
    bool getCanControl();

    void Pause();
    void PlayPause();
    void Stop();
    void Play();
    void Next();
    void Previous();
    void Seek(qlonglong t);
    void SetPosition(QDBusObjectPath o, qlonglong t);
private:
    qmpPluginAPI *api;
    qmpMainWindow *qmw;
};

class QMPTrackList : public QMPrisTrackList
{
public:
    explicit QMPTrackList(qmpPluginAPI *_api, QObject *parent=nullptr);

    QList<QVariantMap> GetTracksMetaData(QList<QDBusObjectPath> trackIds);
    QList<QDBusObjectPath> getTracks();
private:
    qmpPluginAPI *api;
};

class QMPMediaPlayer2 : public QMPrisMediaPlayer2
{
public:
    explicit QMPMediaPlayer2(qmpPluginAPI *_api, QObject *parent=nullptr);

    void Raise() override;
    void Quit() override;

    bool getCanQuit() override;
    bool getCanRaise() override;
    QString getIdentity() override;
    QString getDesktopEntry() override;
    bool getHasTrackList() override;
private:
    qmpPluginAPI *api;
    qmpMainWindow *qmw;
};

#endif
