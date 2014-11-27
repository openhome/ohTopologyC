#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Job.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Av;


Job::Job(FunctorGeneric<void*> aAction, void* aObj)
    :iAction(aAction)
    ,iActionArg(aObj)
    ,iJobContinue(NULL)
    ,iSem("JOBS", 0)
    ,iMutex("JOBX")
    ,iStarted(false)
    ,iCompleted(false)
    ,iCancelled(false)
{
    iThread = new ThreadFunctor("JOB", MakeFunctor(*this, &Job::Run) );
    iThread->Start();
}


void Job::Start()
{
    AutoMutex mutex(iMutex);
    if (iStarted)
    {
        ASSERTS();
    }

    iSem.Signal();
    iStarted = true;
}


void Job::Run()
{
    iSem.Wait();
    if (iAction)
    {
        iAction(iActionArg);
    }

    AutoMutex mutex(iMutex);
    iCompleted = true;
    Continue();
}


void Job::Continue()
{
    if (iJobContinue!=NULL)
    {
        iJobContinue->Start();
    }
}


Job* Job::ContinueWith(FunctorGeneric<void*> aAction, void* aObj)
{
    AutoMutex mutex(iMutex);
    iJobContinue = new Job(aAction, aObj);
    return(iJobContinue);
}


void Job::Wait()
{
    iThread->Join();
}


void Job::Cancel()
{
    AutoMutex mutex(iMutex);
    if (iStarted && !iCompleted)
    {
        iCancelled = true;
    }
}


TBool Job::IsCancelled()
{
    AutoMutex mutex(iMutex);
    return(iCancelled);
}


Job* Job::StartNew(FunctorGeneric<void*> aAction, void* aObj)
{
    return(new Job(aAction, aObj));
}


//////////////////////////////////////////////////////////

JobDone::JobDone()
{
    iJob = new Job(FunctorGeneric<void*>(), NULL);
}


void JobDone::SetResult(TBool aResult)
{
    if(aResult)
    {
        iJob->Start();
    }
}


void JobDone::Cancel()
{
    iJob->Cancel();
}


Job* JobDone::GetJob()
{
    return(iJob);
}

////////////////////////////////////////////////////////////

Job2::Job2()
    :Thread("thname")
    ,iArg(NULL)
    ,iCbArg(NULL)
    ,iCancelled(false)
    ,iMutex("JOB2")
{
    Start();
}


void Job2::SetCallback(FunctorGeneric<AsyncCbArg*> aCallback, void* aArg)
{
    iCallback = aCallback;
    iArg = aArg;
}


void Job2::CallbackComplete()
{
    iCallback = FunctorGeneric<AsyncCbArg*>();  // reset to null functor
    iArg = NULL;
    iCbArg = NULL;
    // add "this" back into fifo of available jobs here
}


void Job2::Run()
{
    Wait();
    ASSERT(iCallback);
    iCallback(iCbArg);
    CallbackComplete();
}


Net::FunctorAsync Job2::AsyncCb()
{
    return (MakeFunctorAsync(*this, &Job2::AsyncComplete));
}


void Job2::AsyncComplete(Net::IAsync& aAsync)
{
    iMutex.Wait();
    TBool cancelled = iCancelled;
    iMutex.Signal();

    if (iCallback && !cancelled)
    {
        iCbArg = new AsyncCbArg();
        iCbArg->iAsync = &aAsync;
        iCbArg->iArg = iArg;

        Signal();
    }
}


void Job2::Cancel()
{
    iMutex.Wait();
    iCancelled = true;
    iMutex.Signal();
}
