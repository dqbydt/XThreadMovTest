#include "xtmapp.h"

#include <malloc.h>
#include <intsafe.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <processthreadsapi.h>
#endif

// Tries to determine stack and heap on various platforms
void queryStackHeap()
{
#ifdef Q_OS_WIN

    // Link to my SO post detailing the strange case of perverse allocation addresses
    // seen on Win10:
    // https://stackoverflow.com/q/65298802/3367247

    // https://docs.microsoft.com/en-us/windows/win32/memory/getting-process-heaps
    // 1.   Retrieve the number of active heaps for the current process
    //      so we can calculate the buffer size needed for the heap handles.
    DWORD NumberOfHeaps;
    NumberOfHeaps = GetProcessHeaps(0, NULL);
    Q_ASSERT(NumberOfHeaps > 0);

    // 2.   Calculate the buffer size needed to store heap handles.
    //      Size = sizeof(HANDLE) * numHeaps
    HRESULT Result;
    PHANDLE aHeaps;
    SIZE_T BytesToAllocate;
    Result = SizeTMult(NumberOfHeaps, sizeof(*aHeaps), &BytesToAllocate);   // Bytes <-- NumHeaps*sizeof(HANDLE)
    Q_ASSERT(Result == S_OK);

    // 3.   Get a handle to the default process heap.
    HANDLE hDefaultProcessHeap;
    hDefaultProcessHeap = GetProcessHeap();
    Q_ASSERT(hDefaultProcessHeap != NULL);

    // 4.   Allocate the buffer to store heap handles, from the default process heap.
    aHeaps = (PHANDLE)HeapAlloc(hDefaultProcessHeap, 0, BytesToAllocate);
    Q_ASSERT_X((aHeaps != NULL), "main", "HeapAlloc failed to allocate space for buffer");

    // 5.   Save the original number of heaps because we are going to compare it
    //      to the return value of the next GetProcessHeaps call.
    DWORD HeapsLength = NumberOfHeaps;

    // 6.   Retrieve handles to the process heaps and print.
    NumberOfHeaps = GetProcessHeaps(HeapsLength, aHeaps);
    Q_ASSERT(NumberOfHeaps > 0);

    qDebug("Num heaps = %lu", NumberOfHeaps);
    DWORD HeapsIndex;
    for (HeapsIndex = 0; HeapsIndex < HeapsLength; ++HeapsIndex) {

        // https://j00ru.vexillium.org/2016/07/disclosing-stack-data-from-the-default-heap-on-windows/
        MEMORY_BASIC_INFORMATION mbi = { };
        SIZE_T heapStart = (SIZE_T) aHeaps[HeapsIndex];
        assert(VirtualQuery(aHeaps[HeapsIndex], &mbi, sizeof(mbi)) != 0);
        SIZE_T heapEnd = heapStart + mbi.RegionSize;

        qDebug("Heap %lu at address: %p - 0x%llx, size %llu bytes", HeapsIndex, aHeaps[HeapsIndex], heapEnd, mbi.RegionSize);
    }

    // 7.   Release memory allocated for buffer from default process heap.
    if (HeapFree(hDefaultProcessHeap, 0, aHeaps) == FALSE) {
        qDebug("Failed to free allocation from default process heap.");
    }

#endif

#ifdef Q_OS_LINUX
    // Read from /proc/self/maps, parse out stack and heap


#endif

    size_t stackObject;
    size_t stackBot = (((size_t) &stackObject) & ~0xfff);  // For a 4kB stack frame
    size_t stackTop = stackBot + 0xfff;

    AddrClassifier::stackTop = stackTop;
    AddrClassifier::stackBot = stackBot;

    qDebug("Stack: 0x%llx - 0x%llx (Sample stack obj address: %p)\n", stackTop, stackBot, &stackObject);
}

XTMApp::XTMApp(const QGuiApplication &a, const QQmlApplicationEngine &e, QObject *parent) : QObject(parent),
    mEngine(e),
    mApp(a)
{
    QObject *rootObject = mEngine.rootObjects().first();
    mpTestWnd = rootObject->findChild<QObject*>("xtmui");

    queryStackHeap();   // NOTE: stack addresses change in member fns!

    qRegisterMetaType<Mov>("Mov");
    qRegisterMetaType<Mov>("Mov&");
    //qRegisterMetaType<Mov>("Mov&&");  // Needed for the now-defunct rvalue Mov attempt

    mupMTest = std::make_unique<MovTest>();
    mupMTest->moveToThread(&mTestThread);

    connect(mpTestWnd,  SIGNAL(sendIt()),
            this,       SLOT(slotTest()));

    // New syntax does not work for overloaded signal/slots!
    connect(this,           &XTMApp::signalSendMov,
            mupMTest.get(), &MovTest::slotGetMov);

    // To test signal getting sent to local slot
    connect(this,   &XTMApp::signalSendMov,
            this,   &XTMApp::slotGetMovLocal);
    //
    //connect(this,           SIGNAL(signalSendMov(const Mov&)),
    //        mupMTest.get(), SLOT(slotGetMov(const Mov&)));

    // This does not work! Looks like Qt does not support rvalue signals/slots,
    // primarily because a signal might have multiple slots, so copies cannot be
    // avoided. See
    // https://stackoverflow.com/questions/30873859/qt-5-unable-to-declare-signal-that-takes-rvalue-reference
    // Results in build error
    // "cannot bind rvalue reference of type 'Mov&&' to lvalue of type 'Mov'" in the moc file.
    // case 1: _t->slotGetMov((*reinterpret_cast< Mov(*)>(_a[1]))); break;
    //                        ~^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    //connect(this,           SIGNAL(signalSendMov(Mov&&)),
    //        mupMTest.get(), SLOT(slotGetMov(Mov&&)));

    connect(mpTestWnd,  SIGNAL(quit()),
            this,       SLOT(quit()));

    mTestThread.start();
    qDebug() << "Test thread started";
    DWORD tid = GetCurrentThreadId();
    // Looks like on Windows QThread::currentThreadId returns the same value as
    // GetCurrentThreadId(). (Of course, must reinterpret_cast the Qt::HANDLE i.e. (void *) to DWORD).
    // e.g. the former might return 0x351c and GCTId prints 13596.
    qDebug("Main thread running as threadId %p, PExp TID %lu", QThread::currentThreadId(), tid);

}

void XTMApp::slotTest()
{
    queryStackHeap();   // Need to do this again because stack addresses change!
    DWORD tid = GetCurrentThreadId();

    qDebug("Sending a Mov from MainThread %lu", tid);
    Mov m;
    emit signalSendMov(m);
    //emit signalSendMov(std::move(m)); //<-- rvalues do not work!
    qDebug("Exiting slotTest in MainThread");
}

void XTMApp::slotGetMovLocal(Mov &m)
{
    DWORD tid = GetCurrentThreadId();
    qDebug("slotGetMovLocal in MainThread %lu", tid);
    m.id();
}

void XTMApp::quit()
{
    qDebug("XTMApp quit slot");
    mTestThread.quit();
    mTestThread.wait();
    qDebug() << "Test thread terminated";
    mApp.quit();
}
