#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Thread.h>

namespace OpenHome
{
namespace Topology
{

class Job
{
public:
    Job(FunctorGeneric<void*> aAction, void* aObj);
    void Start();
    void Wait();

private:
    void Run();

private:
    FunctorGeneric<void*> iAction;
    void* iActionArg;
    ThreadFunctor* iThread;
    Semaphore iSem;
    mutable Mutex iMutex;
    TBool iStarted;
};

} // Topology
} // OpenHome
