#include <OpenHome/WatchableThread.h>
#include <OpenHome/Private/Printer.h>


using namespace OpenHome;


WatchableThread::WatchableThread(IExceptionReporter& aReporter)
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
    // delete SignalledCallbacks  (all but one)
    for (TUint i = 0; i < kMaxFifoEntries-1; i++)
    {
        delete iFree.Read();
    }

    TUint x;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &WatchableThread::Shutdown);

    Schedule(f, &x);
    //MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor1)

    delete iThread;  // kills then joins
    delete iFree.Read();  // last one
}


void WatchableThread::Shutdown(void*)
{
    THROW(ThreadKill);
}

void WatchableThread::Run()
{
    for(;;)
    {
        SignalledCallback* callback = iScheduled.Read();
        try
        {
            callback->Callback();
        }
        catch (ThreadKill& )
        {
            iFree.Write(callback);
            break;
        }
        catch (Exception& e)
        {
            iExceptionReporter.Report(e);
        }
        catch(std::exception& e)
        {
            iExceptionReporter.Report(e);
        }

        iFree.Write(callback);
    }
}


void WatchableThread::Assert()
{
    ASSERT(IsWatchableThread());
}


void WatchableThread::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    SignalledCallback* callback = iFree.Read();
    callback->Set(aCallback, aObj);
    iScheduled.Write(callback);
}


void WatchableThread::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    if (IsWatchableThread())
    {
        try
        {
            aCallback(aObj);
        }
        catch (Exception& e)
        {
            iExceptionReporter.Report(e);
        }
        catch(std::exception& e)
        {
            iExceptionReporter.Report(e);
        }
    }
    else
    {
        Semaphore sem("wtch", 0);
        SignalledCallback* callback = iFree.Read();
        callback->Set(aCallback, aObj, sem);
        iScheduled.Write(callback);
        sem.Wait();
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
    :iObj(NULL)
    ,iSem(NULL)
{
}

void SignalledCallback::Set(FunctorGeneric<void*> aFunctor, void* aObj, Semaphore& aSem)
{
    iFunctor = aFunctor;
    iObj = aObj;
    iSem = &aSem;
}


void SignalledCallback::Set(FunctorGeneric<void*> aFunctor, void* aObj)
{
    iFunctor = aFunctor;
    iObj = aObj;
    iSem = NULL;
}


void SignalledCallback::Callback()
{
    AutoSem as(iSem);
    iFunctor(iObj);
}

////////////////////////////////////////////////////////



