#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include "src/ui/bvhscenemodel.h"
#include "src/ui/bvhscenelistmodel.h"
#include "src/ui/bvhviewitem.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<BvhSceneModel>("BvhViewer", 1, 0, "BvhSceneModel");
    qmlRegisterType<BvhSceneListModel>("BvhViewer", 1, 0, "BvhSceneListModel");
    qmlRegisterType<BvhViewItem>("BvhViewer", 1, 0, "BvhViewItem");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [](QObject *obj, const QUrl &objUrl) {
        if (!obj)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.loadFromModule("BvhViewer", "Main");

    return app.exec();
}