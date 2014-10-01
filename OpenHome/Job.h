#ifndef HEADER_OHTOPC_JOB
#define HEADER_OHTOPC_JOB

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <vector>


namespace OpenHome
{

namespace Av
{

class CallbackHandler;


class Job
{
public:
    Job(FunctorGeneric<void*> aAction, void* aObj); // can't be cancelled
    Job();

    void Start();
    void Wait();
    void Cancel();
    TBool IsCancelled();

    Job* ContinueWith(FunctorGeneric<void*> aAction, void* aObj);
    static Job* StartNew(FunctorGeneric<void*> aAction, void* aObj);

private:
    void Run();
    void Continue();
    void SetComplete();

private:
    FunctorGeneric<void*> iAction;
    void* iActionArg;
    Functor iCancellation;
    ThreadFunctor* iThread;
    Job* iJobContinue;
    Semaphore iSem;
    mutable Mutex iMutex;
    TBool iStarted;
    TBool iCompleted;
    TBool iCancelled;
};


////////////////////////////////////////////////////////////////////////////

class JobDone
{
public:
    JobDone() : iJob(new Job(FunctorGeneric<void*>(), NULL)) { }

    void SetResult(TBool aResult)
    {
        if(aResult)
        {
            iJob->Start();
        }
    }

    void Cancel() {iJob->Cancel();}
    Job* GetJob() {return(iJob);}

private:
    Job* iJob;
};

///////////////////////////////////////////////////////////////////////////

class Job2 : private Thread
{
private:
    FunctorGeneric<void*> iCallback;
    void* iCbArg;
    TBool iCancelled;
    mutable Mutex iMutex;

public:
    Job2()
        :Thread("thname")
        ,iCbArg(NULL)
        ,iCancelled(false)
        ,iMutex("JOBX")
    {
        Start();
    }


    void SetCallback(FunctorGeneric<void*> aAction, void* aArg)
    {
        iCallback = aAction;
        iCbArg = aArg;
    }


    void CallbackComplete()
    {
        iCallback = FunctorGeneric<void*>();  // reset to null functor
        iCbArg = NULL;
        // add this back into fifo of available jobs
    }


    void Run()
    {
        Wait();
        ASSERT(iCallback);
        iCallback(iCbArg);
    }


    Net::FunctorAsync AsyncCb()
    {
        return (MakeFunctorAsync(*this, &Job2::AsyncComplete));
    }


    void AsyncComplete(Net::IAsync& /*aAsync*/)
    {
        iMutex.Wait();
        TBool cancelled = iCancelled;
        iMutex.Signal();

        if (iCallback && !cancelled)
        {
            Signal();
        }
    }


    void Cancel()
    {
        iMutex.Wait();
        iCancelled = true;
        iMutex.Wait();
    }
};


//////////////////////////////////////////////////////////////

} // Av
} // OpenHome

#endif // HEADER_OHTOPC_JOB
