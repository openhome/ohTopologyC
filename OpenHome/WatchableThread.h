#ifndef HEADER_WATCHABLE_THREAD
#define HEADER_WATCHABLE_THREAD

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>

namespace OpenHome
{

class SignalledCallback;


class IExceptionReporter
{
public:
    virtual ~IExceptionReporter() {}
    virtual void Report(Exception& aException) = 0;
    virtual void Report(std::exception& aException) = 0;
};

///////////////////////////////////////////////

class IWatchableThread
{
public:
    virtual ~IWatchableThread() {};
    virtual void Assert() = 0;
    virtual void Schedule(Functor aCallback) = 0;
    virtual void Execute(Functor aCallback) = 0;
    virtual TBool IsWatchableThread() = 0;
};

///////////////////////////////////////////////

class WatchableThread : public IWatchableThread
{
private:
    static const TUint kMaxFifoEntries = 10;

public:
    WatchableThread(IExceptionReporter& aReporter);
    virtual ~WatchableThread();
    virtual void Assert();
    virtual void Schedule(Functor aCallback);
    virtual void Execute(Functor aCallback);
    virtual TBool IsWatchableThread();

private:
    void Run();
    void Shutdown();

private:
    IExceptionReporter& iExceptionReporter;
    Fifo<SignalledCallback*> iFree;
    Fifo<SignalledCallback*> iScheduled;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////

class SignalledCallback
{
public:
    SignalledCallback();
    void Set(Functor aFunctor, Semaphore& aSem);
    void Set(Functor aFunctor);
    void Callback();
private:
    Functor iFunctor;
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



}




#endif //HEADER_WATCHABLE_THREAD
