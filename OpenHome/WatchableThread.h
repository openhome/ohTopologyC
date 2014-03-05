#ifndef HEADER_WATCHABLE_THREAD
#define HEADER_WATCHABLE_THREAD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>
#include <OpenHome/OhTopologyC.h>

#include <stddef.h>


namespace OpenHome
{

namespace Av
{


/////////////////////////////////////////////////////////

class IWatchableThread
{
public:
    virtual ~IWatchableThread() {};

    virtual void Assert() = 0;
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj) = 0;
    //virtual TBool IsWatchableThread() = 0;
};

///////////////////////////////////////////////


class SignalledCallback
{
public:
    SignalledCallback();
    void Set(FunctorGeneric<void*> aFunctor, void* aObj, Semaphore& aSem);
    void Set(FunctorGeneric<void*> aFunctor, void* aObj);
    void Callback();

private:
    FunctorGeneric<void*> iFunctor;
    void* iObj;
    Semaphore* iSem;
};

/////////////////////////////////////////////

class AutoSem
{
public:
    AutoSem(Semaphore* aSem);
    ~AutoSem();
private:
    Semaphore* iSem;
};

/////////////////////////////////////////////

class WatchableThread : public IWatchableThread
{
public:
    static const TUint kMaxFifoEntries = 10;

public:
    WatchableThread(IExceptionReporter& aReporter);
    virtual ~WatchableThread();
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);

private:
    void Run();
    void Shutdown(void*);
    TBool IsWatchableThread();

private:
    IExceptionReporter& iExceptionReporter;
    Fifo<SignalledCallback*> iFree;
    Fifo<SignalledCallback*> iScheduled;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////


} // namsespace Av

} // namespace OpenHome




#endif //HEADER_WATCHABLE_THREAD
