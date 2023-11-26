#include "qmprisdbusinterface.hpp"
#include "qmpriswrapper.hpp"

#include <QMetaEnum>

QMPrisPlayer::QMPrisPlayer(QObject* parent) : QDBusAbstractAdaptor(parent)
{
}

QString QMPrisPlayer::getPlaybackStatus()
{
    /*if (par->pbstgetter)
        return QMetaEnum::fromType<QMPrisWrapper::PlaybackStatus>().key(par->pbstgetter());*/
    return QString();
}
QString QMPrisPlayer::getLoopStatus()
{
    /*if (par->lpstgetter)
        return QMetaEnum::fromType<QMPrisWrapper::LoopStatus>().key(par->lpstgetter());*/
    return QString();
}
double QMPrisPlayer::getRate()
{
    return 1.0;
}
bool QMPrisPlayer::getShuffle()
{
    return false;
}
QVariantMap QMPrisPlayer::getMetadata()
{
    return {};
}
double QMPrisPlayer::getVolume()
{
    return 1.0;
}
qlonglong QMPrisPlayer::getPosition()
{
    return 0;
}
double QMPrisPlayer::getMinimumRate()
{
    return 1;
}
double QMPrisPlayer::getMaximumRate()
{
    return 1;
}
bool QMPrisPlayer::getCanGoNext()
{
    return false;
}
bool QMPrisPlayer::getCanGoPrevious()
{
    return false;
}
bool QMPrisPlayer::getCanPlay()
{
    return false;
}
bool QMPrisPlayer::getCanPause()
{
    return false;
}
bool QMPrisPlayer::getCanSeek()
{
    return false;
}
bool QMPrisPlayer::getCanControl()
{
    return false;
}

void QMPrisPlayer::setLoopStatus(QString loopStatus)
{
}
void QMPrisPlayer::setRate(double playbackRate)
{
}
bool QMPrisPlayer::setShuffle(bool shuffle)
{
    return false;
}
void QMPrisPlayer::setVolume(double volume)
{
}

void QMPrisPlayer::Next()
{
}
void QMPrisPlayer::Previous()
{
}
void QMPrisPlayer::Pause()
{
}
void QMPrisPlayer::PlayPause()
{
}
void QMPrisPlayer::Stop()
{
}
void QMPrisPlayer::Play()
{
}
void QMPrisPlayer::Seek(qlonglong t)
{
}

void QMPrisPlayer::SetPosition(QDBusObjectPath o, qlonglong t)
{
}
void QMPrisPlayer::OpenUri(QString s)
{
}

void QMPrisMediaPlayer2::Raise()
{
}
void QMPrisMediaPlayer2::Quit()
{
}

QMPrisMediaPlayer2::QMPrisMediaPlayer2(QObject *parent) : QDBusAbstractAdaptor(parent)
{
}

bool QMPrisMediaPlayer2::getCanQuit()
{
    return false;
}
bool QMPrisMediaPlayer2::getCanRaise()
{
    return false;
}
bool QMPrisMediaPlayer2::getFullscreen()
{
    return false;
}
bool QMPrisMediaPlayer2::getCanSetFullscreen()
{
    return false;
}
bool QMPrisMediaPlayer2::getHasTrackList()
{
    return false;
}

QString QMPrisMediaPlayer2::getIdentity()
{
    return QString();
}
QString QMPrisMediaPlayer2::getDesktopEntry()
{
    return QString();
}
QStringList QMPrisMediaPlayer2::getSupportedUriSchemes()
{
    return {};
}
QStringList QMPrisMediaPlayer2::getSupportedMimeTypes()
{
    return {};
}

void QMPrisMediaPlayer2::setFullscreen(bool fullscreen)
{
}

QList<QVariantMap> QMPrisTrackList::GetTracksMetaData(QList<QDBusObjectPath> trackIds)
{
    return {};
}

void QMPrisTrackList::AddTrack(QString uri, QDBusObjectPath after, bool setCurrent)
{
}

void QMPrisTrackList::RemoveTrack(QDBusObjectPath trackId)
{
}

void QMPrisTrackList::GoTo(QDBusObjectPath trackId)
{
}

QMPrisTrackList::QMPrisTrackList(QObject *parent) : QDBusAbstractAdaptor(parent)
{
}

QList<QDBusObjectPath> QMPrisTrackList::getTracks()
{
    return {};
}

bool QMPrisTrackList::getCanEditTracks()
{
    return false;
}
