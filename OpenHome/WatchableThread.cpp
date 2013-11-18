#include <OpenHome/WatchableThread.h>


using namespace OpenHome;




WatchableThread::WatchableThread(IExceptionReporter* aReporter)
    :iExceptionReporter(aReporter)
    ,iFree(kMaxFifoEntries)
    ,iScheduled(kMaxFifoEntries)
    ,iThread(new ThreadFunctor("WTHR", MakeFunctor(*this, &WatchableThread::Run) ))
{
    for (TUint i = 0; i < kMaxFifoEntries; i++)
    {
        iFree.Write(new WatchableCb());
    }

    iThread->Start();
}


WatchableThread::~WatchableThread()
{
    TUint freeCount = iFree.SlotsUsed();
    TUint schedCount = iScheduled.SlotsUsed();

    for (TUint i = 0; i < freeCount; i++)
    {
        delete iFree.Read();
    }

    for (TUint i = 0; i < schedCount; i++)
    {
        delete iScheduled.Read();
    }
}

void WatchableThread::Run()
{
    while (true)
    {
        WatchableCb* callback = iScheduled.Read();
        try
        {
            callback->Callback();
        }
        catch (Exception e)
        {
            iExceptionReporter->Report(e);
        }

        iFree.Write(callback);
    }
}


void WatchableThread::Assert()
{
    ASSERT(IsWatchableThread());
}


void WatchableThread::Schedule(Functor aCallback)
{
    WatchableCb* callback = iFree.Read();
    callback->Set(aCallback);
    iScheduled.Write(callback);
}


void WatchableThread::Execute(Functor aCallback)
{
    if (IsWatchableThread())
    {
        try
        {
            aCallback();
        }
        catch (Exception e)
        {
            iExceptionReporter->Report(e);
        }
    }
    else
    {
        Semaphore sem("wtch", 0);
        WatchableCb* callback = iFree.Read();
        callback->Set(aCallback, sem);
        iScheduled.Write(callback);
    }
}


TBool WatchableThread::IsWatchableThread()
{
    return(Thread::Current()==iThread);
}


//////////////////////////////////////////////

WatchableCb::WatchableCb()
{

}


void WatchableCb::Set(Functor aFunctor, Semaphore& aSem)
{
    iFunctor = aFunctor;
    iSem = &aSem;
}


void WatchableCb::Set(Functor aFunctor)
{
    iFunctor = aFunctor;
    iSem = NULL;
}

void WatchableCb::Callback()
{
    try
    {
        iFunctor();
    }
    catch (...) {
    }

    if (iSem != NULL)
    {
        iSem->Signal();
    }
}






