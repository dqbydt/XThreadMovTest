#ifndef XTMAPP_H
#define XTMAPP_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QGuiApplication>
#include <QDebug>
#include <QThread>

#include "mov.h"
#include "movtest.h"

// X-thread Mov App
class XTMApp : public QObject
{
    Q_OBJECT
public:
    XTMApp(const QGuiApplication &a,
           const QQmlApplicationEngine &e,
           QObject *parent = nullptr);

public slots:
    void slotTest();
    void slotGetMovLocal(Mov& m);   // Get mov slot within this thread
    void quit();

signals:
    //void signalSendMov(Mov&& m); <-- does not work, moc does not have rvalue support
    //void signalSendMov(const Mov& m);
    void signalSendMov(Mov& m);
    //void signalSendMov(Mov m);

private:
    const QQmlApplicationEngine &mEngine;
    const QGuiApplication &mApp;

    QObject* mpTestWnd;
    std::unique_ptr<MovTest> mupMTest;
    QThread mTestThread;

};

#endif // XTMAPP_H
