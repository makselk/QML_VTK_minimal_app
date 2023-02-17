#include "QVTKFrameBufferObjectItem.h"
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QQuickView>
#include <QList>


int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<QVTKFrameBufferObjectItem>("VTK", 8, 2, "VtkFboItem");

    QQmlApplicationEngine engine;
    const QUrl url(QStringLiteral("qrc:/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    QObject* topLevel = engine.rootObjects().value(0);
    QQuickWindow* window = qobject_cast<QQuickWindow*>(topLevel);
    window->show();

    return app.exec();
}
