#ifndef QMPRISWRAPPER_HPP
#define QMPRISWRAPPER_HPP

#include <QObject>

class QMPrisMediaPlayer2;
class QMPrisPlayer;
class QMPrisTrackList;
class qmpPluginAPI;

class QMPrisWrapper : public QObject
{
    Q_OBJECT
public:
    ~QMPrisWrapper();

    template <class TP, class TM, class TT>
    static QMPrisWrapper *create(QString serviceSuffix, qmpPluginAPI *api, QObject *parent = nullptr)
    {
        static_assert(std::is_base_of<QMPrisPlayer, TP>(), "TP must be a subclass of QMPrisPlayer");
        static_assert(std::is_base_of<QMPrisMediaPlayer2, TM>(), "TM must be a subclass of QMPrisMediaPlayer2");
        static_assert(std::is_base_of<QMPrisTrackList, TT>(), "TT must be a subclass of QMPrisTrackList");

        auto w = new QMPrisWrapper(serviceSuffix, api, parent);
        auto p = new TP(api, w);
        auto t = new TT(api, w);
        auto mp = new TM(api, w);
        w->player = p;
        w->tracklist = t;
        w->mediaplayer = mp;
        w->post_creation();

        return w;
    }

    static void notifyPropertyChange(QString intf, QString prop, QVariant val);

private:
    explicit QMPrisWrapper(QString serviceSuffix, qmpPluginAPI *_api, QObject *parent = nullptr);
    void post_creation();
    QMPrisPlayer *player = nullptr;
    QMPrisMediaPlayer2 *mediaplayer = nullptr;
    QMPrisTrackList *tracklist = nullptr;
    QString svcsuffix;
    qmpPluginAPI *api;

    const QString PLAYER_INTERFACE = "org.mpris.MediaPlayer2.Player";

    friend class QMPrisPlayer;
    friend class QMPrisMediaPlayer2;
};

#endif // QMPRISWRAPPER_HPP
