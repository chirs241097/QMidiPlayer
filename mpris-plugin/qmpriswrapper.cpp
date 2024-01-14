#include "qmpriswrapper.hpp"
#include "qmprisdbusinterface.hpp"
#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusMessage>

#include "qmpcorepublic.hpp"

QMPrisWrapper::QMPrisWrapper(QString serviceSuffix, qmpPluginAPI *_api, QObject *parent) :
    api(_api),
    QObject(parent),
    svcsuffix(serviceSuffix)
{
    qDBusRegisterMetaType<QStringList>();
    qDBusRegisterMetaType<QVariantMap>();
    qDBusRegisterMetaType<QList<QVariantMap>>();
    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
}

void QMPrisWrapper::post_creation()
{
    QDBusConnection sessbus = QDBusConnection::sessionBus();
    sessbus.registerService("org.mpris.MediaPlayer2." + svcsuffix);
    sessbus.registerObject("/org/mpris/MediaPlayer2", this);
    api->registerUIHook("main.stop", [this](const void* a, void* _) {
        tracklist->TrackListReplaced({}, QDBusObjectPath("/"));
        this->notifyPropertyChange(PLAYER_INTERFACE, "Metadata", player->getMetadata());
        this->notifyPropertyChange(PLAYER_INTERFACE, "PlaybackStatus", player->getPlaybackStatus());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPause", player->getCanPause());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPlay", player->getCanPlay());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanSeek", player->getCanSeek());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoNext", player->getCanGoNext());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoPrevious", player->getCanGoPrevious());
    }, nullptr);
    api->registerUIHook("main.start", [this](const void* a, void* _) {
        tracklist->TrackListReplaced(tracklist->getTracks(), QDBusObjectPath("/org/chrisoft/qmidiplayer/dummylist/0"));
        this->notifyPropertyChange(PLAYER_INTERFACE, "Metadata", player->getMetadata());
        this->notifyPropertyChange(PLAYER_INTERFACE, "PlaybackStatus", player->getPlaybackStatus());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPause", player->getCanPause());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPlay", player->getCanPlay());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanSeek", player->getCanSeek());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoNext", player->getCanGoNext());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoPrevious", player->getCanGoPrevious());
        this->notifyPropertyChange(PLAYER_INTERFACE, "Rate", player->getRate());
    }, nullptr);
    api->registerUIHook("main.pause", [this](const void* a, void* _) {
        this->notifyPropertyChange(PLAYER_INTERFACE, "PlaybackStatus", player->getPlaybackStatus());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPause", player->getCanPause());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanPlay", player->getCanPlay());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanSeek", player->getCanSeek());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoNext", player->getCanGoNext());
        this->notifyPropertyChange(PLAYER_INTERFACE, "CanGoPrevious", player->getCanGoPrevious());
    }, nullptr);
    api->registerUIHook("main.seek", [this](const void* a, void *_) {
        auto ps = static_cast<const ::PlaybackStatus*>(a);
        player->Seeked(ps->curtime_ms * 1000);
    }, nullptr);
}

QMPrisWrapper::~QMPrisWrapper()
{
    QDBusConnection sessbus = QDBusConnection::sessionBus();
    sessbus.unregisterObject("/org/mpris/MediaPlayer2");
    sessbus.unregisterService("org.mpris.MediaPlayer2." + svcsuffix);
}

void QMPrisWrapper::notifyPropertyChange(QString intf, QString prop, QVariant val)
{
    QDBusConnection sessbus = QDBusConnection::sessionBus();
    auto signal = QDBusMessage::createSignal("/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", "PropertiesChanged");
    signal.setArguments({
        intf,
        QVariantMap{{prop, val}},
        QStringList{}
    });
    sessbus.send(signal);
}
