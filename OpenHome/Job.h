#ifndef HEADER_OHTOPC_JOB
#define HEADER_OHTOPC_JOB

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <vector>


namespace OpenHome
{

namespace Av
{

class CallbackHandler;


class Job
{
    friend class JobDone;

public:
    Job(FunctorGeneric<void*> aAction, void* aObj); // can't be cancelled
    Job(FunctorGeneric<void*> aAction, void* aObj, Functor aCancellation);

    void Start();
    void Wait();
    void Cancel();
    TBool Result();
    TBool IsCancelled();

    Job* ContinueWith(FunctorGeneric<void*> aAction, void* aObj);
    static Job* StartNew(FunctorGeneric<void*> aAction, void* aObj);

private:
    void Run();
    void Continue();
    Job();

private:
    CallbackHandler* iCallback;
    //CallbackHandler* iContinueCallback;
    Functor iCancellation;
    ThreadFunctor* iThread;
    Job* iJobContinue;
    Semaphore iSem;
    mutable Mutex iMutex;
    TBool iStarted;
    TBool iCompleted;
    TBool iCancelled;
};

//////////////////////////////////////////////////////////////

class JobDone
{
public:
    JobDone();
    void SetResult(TBool aResult);
    void SetException(Exception aException);
    Job* GetJob();
    void Cancel();
    //void TrySetCancelled();
private:
    Job* iJob;
};

//////////////////////////////////////////////////////////////

class CallbackHandler
{
public:
    CallbackHandler(FunctorGeneric<void*> aCallback, void* aObj);

    void Callback();

private:
    FunctorGeneric<void*> iCallback;
    void* iObj;

};



} // Av
} // OpenHome

#endif // HEADER_OHTOPC_JOB
