#include <OpenHome/WatchableThread.h>
#include <OpenHome/Private/Printer.h>


using namespace OpenHome;


WatchableThread::WatchableThread(IExceptionReporter* aReporter)
    :iExceptionReporter(aReporter)
    ,iFree(kMaxFifoEntries)
    ,iScheduled(kMaxFifoEntries)
{
    iThread = new ThreadFunctor("WTHR", MakeFunctor(*this, &WatchableThread::Run) );

    for (TUint i = 0; i < kMaxFifoEntries; i++)
    {
        iFree.Write(new SignalledCallback());
    }

    iThread->Start();
}


WatchableThread::~WatchableThread()
{
    for (TUint i = 0; i < kMaxFifoEntries; i++)
    {
        delete iFree.Read();
    }
}


void WatchableThread::Run()
{
    for (;;)
    {
        SignalledCallback* callback = iScheduled.Read();
        try
        {
            callback->Callback();
        }
        catch (Exception& e)
        {
            iExceptionReporter->Report(e);
        }
        catch(std::exception& e)
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
    SignalledCallback* callback = iFree.Read();
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
        catch (Exception& e)
        {
            iExceptionReporter->Report(e);
        }
        catch(std::exception& e)
        {
            iExceptionReporter->Report(e);
        }
    }
    else
    {
        Semaphore sem("wtch", 0);
        SignalledCallback* callback = iFree.Read();
        callback->Set(aCallback, sem);
        iScheduled.Write(callback);
    }
}


TBool WatchableThread::IsWatchableThread()
{
    return(Thread::Current()==iThread);
}

/////////////////////////////////////////////////////////

AutoSem::AutoSem(Semaphore* aSem)
    :iSem(aSem)
{

}


AutoSem::~AutoSem()
{
    if (iSem != NULL)
    {
        iSem->Signal();
    }
}

////////////////////////////////////////////////////////

SignalledCallback::SignalledCallback()
{

}

void SignalledCallback::Set(Functor aFunctor, Semaphore& aSem)
{
    iFunctor = aFunctor;
    iSem = &aSem;
}


void SignalledCallback::Set(Functor aFunctor)
{
    iFunctor = aFunctor;
    iSem = NULL;
}

void SignalledCallback::Callback()
{
    AutoSem as(iSem);
    iFunctor();
}






