#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlError>
#include <QQuickStyle>
#include <QtQml/qqmlextensionplugin.h>

#ifdef Q_OS_WIN
#include <cstdio>
#include <Windows.h>
#endif

namespace {

void attachDebugConsole()
{
#ifdef Q_OS_WIN
    if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
        return;
    }
    FILE *stream = nullptr;
    freopen_s(&stream, "CONOUT$", "w", stderr);
    freopen_s(&stream, "CONOUT$", "w", stdout);
#endif
}

void logQmlErrors(const QList<QQmlError> &errors)
{
    for (const QQmlError &error : errors) {
        qWarning().noquote() << error.toString();
    }
}

} // namespace

Q_IMPORT_QML_PLUGIN(BvhScenePlugin)

int main(int argc, char *argv[])
{
    attachDebugConsole();

    QQuickStyle::setStyle("Fusion");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::warnings,
                     &app, [](const QList<QQmlError> &warnings) {
                         logQmlErrors(warnings);
                     });
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
                     &app, []() {
                         qWarning() << "QML object creation failed.";
                         QCoreApplication::exit(-1);
                     },
                     Qt::QueuedConnection);
    engine.loadFromModule("BvhViewer", "Main");

    return app.exec();
}
