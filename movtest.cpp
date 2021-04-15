#include "movtest.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#endif

MovTest::MovTest(QObject *parent) : QObject(parent)
{

}

void MovTest::slotTest()
{
    qDebug("slotTest in TestThread");

}

//void MovTest::slotGetMov(Mov&& m)
//void MovTest::slotGetMov(const Mov &m)
void MovTest::slotGetMov(Mov& m)
//void MovTest::slotGetMov(Mov m)
{
    DWORD tid = GetCurrentThreadId();
    qDebug("slotGetMov in TestThread %lu", tid);
    m.id();
}
