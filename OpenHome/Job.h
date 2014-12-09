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

private:
    FunctorGeneric<void*> iAction;
    void* iActionArg;
    //Functor iCancellation;
    ThreadFunctor* iThread;
    Job* iJobContinue;
    Semaphore iSem;
    mutable Mutex iMutex;
    TBool iStarted;
    TBool iCompleted;
    TBool iCancelled;
};

///////////////////////////////////////////////////////////////////////////

struct AsyncCbArg
{
    Net::IAsync* iAsync;
    void* iArg;
};

///////////////////////////////////////////////////////////////////////////

class Job3;

class JobManager
{
private:
    static const TUint kJobCount = 10;

public:
    JobManager();
    virtual ~JobManager();
    Job3& GetJob();
    void ReleaseJob(Job3& aJob);

private:
    Fifo<Job3*> iJobs;
};

///////////////////////////////////////////////////////////////////////////

class Job3 : public INonCopyable
{
    friend class JobManager;

private:
    Job3(JobManager& aMan);
    ~Job3();

public:
    void SetCallback(FunctorGeneric<AsyncCbArg*> aCallback, void* aArg);
    Net::FunctorAsync AsyncCb();

private:
    void AsyncComplete(Net::IAsync& aAsync);
    void Reset();

private:
    JobManager& iJobManager;
    FunctorGeneric<AsyncCbArg*> iCallback;
    AsyncCbArg* iCombinedArgs;
};

//////////////////////////////////////////////////////////////


} // Av
} // OpenHome

#endif // HEADER_OHTOPC_JOB
