//#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Private/Printer.h>




using namespace OpenHome;
using namespace OpenHome::Topology;
EXCEPTION(Windows_SEH)
EXCEPTION(Windows_SEH_AccessViolation)
EXCEPTION(Windows_SEH_ArrayBoundsExceeded)
EXCEPTION(Windows_SEH_DatatypeMisalignment)
EXCEPTION(Windows_SEH_GuardPage)
EXCEPTION(Windows_SEH_DivideByZero)
EXCEPTION(Windows_SEH_InvalidHandle)
EXCEPTION(Windows_SEH_StackOverflow)

WatchableThread::WatchableThread(IExceptionReporter& aReporter)
    :iExceptionReporter(aReporter)
    ,iFree(kMaxFifoEntries)
    ,iScheduled(kMaxFifoEntries)
{
    iThread = new ThreadFunctor("WatchableThread", MakeFunctor(*this, &WatchableThread::Run) );

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

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &WatchableThread::Shutdown);

    Schedule(f, 0);

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
        catch (AssertionFailed&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_AccessViolation&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_ArrayBoundsExceeded&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_DatatypeMisalignment&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_GuardPage&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_DivideByZero&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_InvalidHandle&)
        {
            throw;
        }
        catch (OpenHome::Windows_SEH_StackOverflow&)
        {
            throw;
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
    TBool wt = IsWatchableThread();
    ASSERT(wt);
}


void WatchableThread::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    // Add the callback to queue of callbacks to be run on the WT.
    // Don't wait on the callback being run
    if (IsWatchableThread())
    {
        if(iFree.SlotsUsed()==0)
        {
            ASSERTS();
        }
    }
    SignalledCallback* callback = iFree.Read();
    callback->Set(aCallback, aObj);
    iScheduled.Write(callback);
}


void WatchableThread::DoNothing(void*)
{

}


void WatchableThread::Execute()
{
    Execute(MakeFunctorGeneric(*this, &WatchableThread::DoNothing), NULL);
}


void WatchableThread::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    // If calling function is on the WT - run the callback immediately.
    // If calling function is not on the WT - schedule the callback to be run on the WT
    // In either case - don't return until callback has been run
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

AutoSig::AutoSig(Semaphore* aSem)
    :iSem(aSem)
{

}


AutoSig::~AutoSig()
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
    AutoSig as(iSem);
    iFunctor(iObj);
}

////////////////////////////////////////////////////////



