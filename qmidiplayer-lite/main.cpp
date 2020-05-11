#include <QApplication>
#include <QtQml>
#include <QQmlApplicationEngine>
#include "qmpcorewrapper.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    qmlRegisterType<CQMPCoreWrapper>("org.chrisoft.qmpcore", 1, 0, "CQMPCoreWrapper");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
