#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Job.h>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Av;


Job::Job(FunctorGeneric<void*> aAction, void* aObj)
    :iCallback(new CallbackHandler(aAction, aObj))
    //,iContinueCallback(NULL)
    ,iSem("TKSM", 0)
    ,iMutex("TKMX")
    ,iStarted(false)
    ,iCompleted(false)
    ,iCancelled(false)
{
    iThread = new ThreadFunctor("JOB", MakeFunctor(*this, &Job::Run) );
    iThread->Start();
}


Job::Job()
    :iCallback(NULL)
    ,iSem("TKSM", 0)
    ,iMutex("TKMX")
    ,iStarted(false)
    ,iCompleted(false)
    ,iCancelled(false)
{

}


void Job::Run()
{
    iSem.Wait();
    //iAction(NULL);
    iCallback->Callback();

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
    //iThreadContinue = new ThreadFunctor("JOB", MakeFunctor(*this, &aAction) );
    iJobContinue = new Job(aAction, aObj);
    return(iJobContinue);
}



void Job::Start()
{
    AutoMutex mutex(iMutex);
    ASSERT(!iStarted);
    iSem.Signal();
    iStarted = true;
}



void Job::Wait()
{

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
    return(iCancelled);
}


Job* Job::StartNew(FunctorGeneric<void*> aAction, void* aObj)
{
    return(new Job(aAction, aObj));
}


TBool Job::Result()
{
    return(true); // FIXME:
}

//////////////////////////////////////////////////////////

JobDone::JobDone()
    :iJob(new Job())
{

}


void JobDone::SetResult(TBool /*aResult*/)
{
}


void JobDone::SetException(Exception /*aException*/)
{

}


Job* JobDone::GetJob()
{
    return(iJob);
}

void JobDone::Cancel()
{
    iJob->Cancel();
}

//////////////////////////////////////////////////////////

CallbackHandler::CallbackHandler(FunctorGeneric<void*> aCallback, void* aObj)
    :iCallback(aCallback)
    ,iObj(aObj)
{

}


void CallbackHandler::Callback()
{
    iCallback(iObj);
}
