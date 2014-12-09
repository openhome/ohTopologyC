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

////////////////////////////////////////////////////////////

Job3::Job3(JobManager& aMan)
    :iJobManager(aMan)
    ,iCombinedArgs(new AsyncCbArg())
{
    Reset();
}


Job3::~Job3()
{
    delete iCombinedArgs;
}


void Job3::SetCallback(FunctorGeneric<AsyncCbArg*> aCallback, void* aArg)
{
    iCallback = aCallback;
    iCombinedArgs->iArg = aArg;
}


Net::FunctorAsync Job3::AsyncCb()
{
    return (MakeFunctorAsync(*this, &Job3::AsyncComplete));
}


void Job3::AsyncComplete(Net::IAsync& aAsync)
{
    iCombinedArgs->iAsync = &aAsync;
    iCallback(iCombinedArgs);
    Reset();
    iJobManager.ReleaseJob(*this); // add "this" back into fifo of available jobs
}


void Job3::Reset()
{
    iCallback = FunctorGeneric<AsyncCbArg*>();  // reset to null functor
    iCombinedArgs->iAsync = NULL;
    iCombinedArgs->iArg = NULL;
}

//////////////////////////////////////////////

JobManager::JobManager()
    :iJobs(kJobCount)
{
    for (TUint i=0; i<kJobCount; i++)
    {
        iJobs.Write(new Job3(*this));
    }
}


JobManager::~JobManager()
{
    for (TUint i=0; i<kJobCount; i++)
    {
        delete (iJobs.Read());
    }
}


Job3& JobManager::GetJob()
{
    return(*iJobs.Read());
}


void JobManager::ReleaseJob(Job3& aJob)
{
    return(iJobs.Write(&aJob));
}



