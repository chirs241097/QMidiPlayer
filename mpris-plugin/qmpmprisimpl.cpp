#include <QMetaEnum>

#include "../include/qmpcorepublic.hpp"
#include "../qmidiplayer-desktop/qmpmainwindow.hpp"
#include "qmpmprisimpl.hpp"

inline QVariantMap get_metadata(qmpPluginAPI *api)
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    return {
        {"mpris:trackid", QDBusObjectPath("/org/chrisoft/qmidiplayer/dummylist/0")},
        {"xesam:url",     QString::fromStdWString(api->getWFilePath())},
        {"xesam:title",   QString::fromStdWString(api->getWTitle())},
        {"mpris:length",  qlonglong(ps.maxtime_ms * 1000)}
    };
}

QMPPlayer::QMPPlayer(qmpPluginAPI *_api, QObject *parent) :
    api(_api),
    QMPrisPlayer(parent)
{
    qmw = static_cast<qmpMainWindow*>(api->getMainWindow());
}

QString QMPPlayer::getPlaybackStatus()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    QMPrisPlayer::PlaybackStatus r = QMPrisPlayer::PlaybackStatus::Stopped;
    if (!ps.stopped)
        r = ps.paused ? QMPrisPlayer::PlaybackStatus::Paused : QMPrisPlayer::PlaybackStatus::Playing;
    return QMetaEnum::fromType<QMPrisPlayer::PlaybackStatus>().key(r);
}

QString QMPPlayer::getLoopStatus()
{
    return QMetaEnum::fromType<QMPrisPlayer::LoopStatus>().key(QMPrisPlayer::LoopStatus::None);
}

double QMPPlayer::getRate()
{
    return 1;
}

bool QMPPlayer::getShuffle()
{
    return false;
}

QVariantMap QMPPlayer::getMetadata()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    if (ps.stopped) return {};
    return get_metadata(api);
}

qlonglong QMPPlayer::getPosition()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    return ps.curtime_ms * 1000;
}

bool QMPPlayer::getCanGoNext()
{
    return getCanPlay();
}

bool QMPPlayer::getCanGoPrevious()
{
    return getCanPlay();
}

bool QMPPlayer::getCanPlay()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    return !ps.stopped;
}

bool QMPPlayer::getCanPause()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    return !ps.stopped && !ps.paused;
}

bool QMPPlayer::getCanSeek()
{
    return getCanPlay();
}

bool QMPPlayer::getCanControl()
{
    return true;
}

void QMPPlayer::Pause()
{
    api->playbackControl(PlaybackControlCommand::Pause, nullptr);
}

void QMPPlayer::PlayPause()
{
    api->playbackControl(PlaybackControlCommand::TogglePause, nullptr);
}

void QMPPlayer::Stop()
{
    api->playbackControl(PlaybackControlCommand::Stop, nullptr);
}

void QMPPlayer::Play()
{
    api->playbackControl(PlaybackControlCommand::Play, nullptr);
}

void QMPPlayer::Next()
{
    api->playbackControl(PlaybackControlCommand::NextTrack, nullptr);
}

void QMPPlayer::Previous()
{
    api->playbackControl(PlaybackControlCommand::PrevTrack, nullptr);
}

void QMPPlayer::Seek(qlonglong t)
{
    double td = t / 1e6;
    api->playbackControl(PlaybackControlCommand::SeekAbs, &td);
}

void QMPPlayer::SetPosition(QDBusObjectPath o, qlonglong t)
{
    if (o.path() == QString("/org/chrisoft/qmidiplayer/dummylist/0"))
    {
        double td = t / 1e6;
        api->playbackControl(PlaybackControlCommand::SeekAbs, &td);
    }
}

QMPMediaPlayer2::QMPMediaPlayer2(qmpPluginAPI *_api, QObject *parent) :
    api(_api),
    QMPrisMediaPlayer2(parent)
{
    qmw = static_cast<qmpMainWindow*>(api->getMainWindow());
}

void QMPMediaPlayer2::Raise()
{
    qmw->raise();
}

void QMPMediaPlayer2::Quit()
{
    qmw->close();
}

bool QMPMediaPlayer2::getCanQuit()
{
    return true;
}

bool QMPMediaPlayer2::getCanRaise()
{
    return true;
}

QString QMPMediaPlayer2::getIdentity()
{
    return QString("QMidiPlayer");
}

QString QMPMediaPlayer2::getDesktopEntry()
{
    return QString("qmidiplayer");
}

bool QMPMediaPlayer2::getHasTrackList()
{
    return true;
}

QMPTrackList::QMPTrackList(qmpPluginAPI *_api, QObject *parent) :
    api(_api),
    QMPrisTrackList(parent)
{
}

QList<QVariantMap> QMPTrackList::GetTracksMetaData(QList<QDBusObjectPath> trackIds)
{
    QList<QVariantMap> ret;
    for (auto &i : trackIds)
    {
        if (i.path() == QString("/org/chrisoft/qmidiplayer/dummylist/0"))
            ret.push_back(get_metadata(api));
        else ret.push_back({});
    }
    return ret;
}

QList<QDBusObjectPath> QMPTrackList::getTracks()
{
    ::PlaybackStatus ps = api->getPlaybackStatus();
    if (ps.stopped)
        return {};
    return {QDBusObjectPath("/org/chrisoft/qmidiplayer/dummylist/0")};
}
