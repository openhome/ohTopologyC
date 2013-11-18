#ifndef HEADER_WATCHABLE_THREAD
#define HEADER_WATCHABLE_THREAD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>

namespace OpenHome
{

class WatchableCb;


class IExceptionReporter
{
public:
    virtual void Report(Exception aException) = 0;
};

///////////////////////////////////////////////

class IWatchableThread
{
public:
    virtual void Assert() = 0;
    virtual void Schedule(Functor aCallback) = 0;
    virtual void Execute(Functor aCallback) = 0;
};

///////////////////////////////////////////////

class WatchableThread : public IWatchableThread
{
private:
    static const TUint kMaxFifoEntries = 10;

public:
    WatchableThread(IExceptionReporter* aReporter);
    ~WatchableThread();
    virtual void Assert();
    virtual void Schedule(Functor aCallback);
    virtual void Execute(Functor aCallback);

private:
    TBool IsWatchableThread();
    void Run();

private:
    IExceptionReporter* iExceptionReporter;
    Fifo<WatchableCb*> iFree;
    Fifo<WatchableCb*> iScheduled;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////

class WatchableCb
{
public:
    WatchableCb();
    void Set(Functor aFunctor, Semaphore& aSem);
    void Set(Functor aFunctor);
    void Callback();
private:
    Functor iFunctor;
    Semaphore* iSem;
};


}




#endif //HEADER_WATCHABLE_THREAD
