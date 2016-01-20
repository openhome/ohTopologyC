#include <OpenHome/Job.h>

using namespace OpenHome;
using namespace OpenHome::Topology;


Job::Job(FunctorGeneric<void*> aAction, void* aObj)
    :iAction(aAction)
    ,iActionArg(aObj)
    ,iSem("JOBS", 0)
    ,iMutex("JOBX")
    ,iStarted(false)
{
    iThread = new ThreadFunctor("JOB", MakeFunctor(*this, &Job::Run) );
    iThread->Start();
}


void Job::Start()
{
    AutoMutex mutex(iMutex);
    ASSERT(!iStarted)
    iStarted = true;
    iSem.Signal();
}


void Job::Run()
{
    iSem.Wait();
    if (iAction)
    {
        iAction(iActionArg);
    }
}


void Job::Wait()
{
    iThread->Join();
}

