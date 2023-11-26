/*
 * DBus adaptor for the MPRIS Interface
 * Based on MPRIS D-Bus Interface Specification Version 2.2:
 * https://specifications.freedesktop.org/mpris-spec/2.2/index.html
 */
#ifndef QMPRISDBUSINTERFACE_HPP
#define QMPRISDBUSINTERFACE_HPP

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QVariant>

class QMPrisWrapper;

class QMPrisPlayer : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.Player")

    Q_PROPERTY(QString PlaybackStatus READ getPlaybackStatus)
    Q_PROPERTY(QString LoopStatus READ getLoopStatus WRITE setLoopStatus)
    Q_PROPERTY(double Rate READ getRate WRITE setRate)
    Q_PROPERTY(bool Shuffle READ getShuffle WRITE setShuffle)
    Q_PROPERTY(QVariantMap Metadata READ getMetadata)
    Q_PROPERTY(double Volume READ getVolume WRITE setVolume)
    Q_PROPERTY(qlonglong Position READ getPosition)
    Q_PROPERTY(double MinimumRate READ getMinimumRate)
    Q_PROPERTY(double MaximumRate READ getMaximumRate)

    Q_PROPERTY(bool CanGoNext READ getCanGoNext)
    Q_PROPERTY(bool CanGoPrevious READ getCanGoPrevious)
    Q_PROPERTY(bool CanPlay READ getCanPlay)
    Q_PROPERTY(bool CanPause READ getCanPause)
    Q_PROPERTY(bool CanSeek READ getCanSeek)
    Q_PROPERTY(bool CanControl READ getCanControl)
public:
    enum PlaybackStatus
    {
        Playing,
        Paused,
        Stopped
    };
    Q_ENUM(PlaybackStatus)

    enum LoopStatus
    {
        None,
        Track,
        Playlist
    };
    Q_ENUM(LoopStatus)

    explicit QMPrisPlayer(QObject *parent=nullptr);

    virtual QString getPlaybackStatus();
    virtual QString getLoopStatus();
    virtual double getRate();
    virtual bool getShuffle();
    virtual QVariantMap getMetadata();
    virtual double getVolume();
    virtual qlonglong getPosition();
    virtual double getMinimumRate();
    virtual double getMaximumRate();
    virtual bool getCanGoNext();
    virtual bool getCanGoPrevious();
    virtual bool getCanPlay();
    virtual bool getCanPause();
    virtual bool getCanSeek();
    virtual bool getCanControl();

    virtual void setLoopStatus(QString loopStatus);
    virtual void setRate(double playbackRate);
    virtual bool setShuffle(bool shuffle);
    virtual void setVolume(double volume);

public slots:
    virtual void Next();
    virtual void Previous();
    virtual void Pause();
    virtual void PlayPause();
    virtual void Stop();
    virtual void Play();
    virtual void Seek(qlonglong t);
    virtual void SetPosition(QDBusObjectPath o, qlonglong t);
    virtual void OpenUri(QString s);

signals:
    void Seeked(qlonglong t);
};

class QMPrisTrackList : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2.TrackList")
    Q_PROPERTY(QList<QDBusObjectPath> Tracks READ getTracks)
    Q_PROPERTY(bool CanEditTracks READ getCanEditTracks)
public slots:
    virtual QList<QVariantMap> GetTracksMetaData(QList<QDBusObjectPath> trackIds);
    virtual void AddTrack(QString uri, QDBusObjectPath after, bool setCurrent);
    virtual void RemoveTrack(QDBusObjectPath trackId);
    virtual void GoTo(QDBusObjectPath trackId);
signals:
    void TrackListReplaced(QList<QDBusObjectPath> tracks, QDBusObjectPath currentTrack);
    void TrackAdded(QVariantMap metadata, QDBusObjectPath after);
    void TrackRemoved(QDBusObjectPath track);
    void TrackMetadataChanged(QDBusObjectPath track, QVariantMap metadata);
public:
    explicit QMPrisTrackList(QObject *parent=nullptr);
    virtual QList<QDBusObjectPath> getTracks();
    virtual bool getCanEditTracks();
};

class QMPrisMediaPlayer2 : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.mpris.MediaPlayer2")

    Q_PROPERTY(bool CanQuit READ getCanQuit)
    Q_PROPERTY(bool CanRaise READ getCanRaise)
    Q_PROPERTY(bool Fullscreen READ getFullscreen WRITE setFullscreen)
    Q_PROPERTY(bool CanSetFullscreen READ getCanSetFullscreen)
    Q_PROPERTY(bool HasTrackList READ getHasTrackList)
    Q_PROPERTY(QString Identity READ getIdentity)
    Q_PROPERTY(QString DesktopEntry READ getDesktopEntry)
    Q_PROPERTY(QStringList SupportedUriSchemes READ getSupportedUriSchemes)
    Q_PROPERTY(QStringList SupportedMimeTypes READ getSupportedMimeTypes)

public slots:
    virtual void Raise();
    virtual void Quit();

public:
    explicit QMPrisMediaPlayer2(QObject *parent=nullptr);

    virtual bool getCanQuit();
    virtual bool getCanRaise();
    virtual bool getFullscreen();
    virtual bool getCanSetFullscreen();
    virtual bool getHasTrackList();
    virtual QString getIdentity();
    virtual QString getDesktopEntry();
    virtual QStringList getSupportedUriSchemes();
    virtual QStringList getSupportedMimeTypes();

    virtual void setFullscreen(bool fullscreen);
};

#endif // QMPRISDBUSINTERFACE_HPP
