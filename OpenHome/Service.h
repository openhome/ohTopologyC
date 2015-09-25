#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/Private/Thread.h>
#include <vector>

EXCEPTION(ServiceNotFoundException)


namespace OpenHome
{
namespace Topology
{

class INetwork;
class IInjectorDevice;
class IDevice;


class IProxy : public IDisposable
{
public:
    virtual IDevice& Device() = 0;
};

////////////////////////////////////////////////

struct ServiceCreateData
{
    FunctorGeneric<IProxy*> iCallback;
    IDevice* iDevice;
};

////////////////////////////////////////////////

class IService : public IMockable, public IDisposable
{
public:
    virtual void Create(FunctorGeneric<IProxy*>, IDevice* aDevice) = 0;
};

////////////////////////////////////////////////


class Service : public IService, public IWatchableThread, public INonCopyable
{
public:
    ~Service();

    virtual TBool Wait();
    virtual void Unsubscribe();
    virtual IInjectorDevice& Device();
    virtual IProxy* OnCreate(IDevice& aDevice) = 0;

    // IService
    virtual void Create(FunctorGeneric<IProxy*> aCallback, IDevice* aDevice);

    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute();

    // IMockable
    virtual void Execute(ICommandTokens& aCommand);

    // IDisposable
    virtual void Dispose();

protected:
    Service(IInjectorDevice& aDevice, ILog& aLog);

    virtual TBool OnSubscribe();
    void Start(FunctorGeneric<void*> aCallback, void* aArg);
    //Job* Start(FunctorGeneric<void*> aAction);
    //JobDone* Start();
    //Task<T> Start<T>(Func<T> aFunction);
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();
    void SubscribeCompleted();

private:
    //void HandleAggregate(AggregateException aException);
    void DisposeCallback(void*);
    void SubscribeCompletedCallback(void* aArgs);
    void StartCallback(void* aArg);
    void AddTask(Semaphore* sema);



private:
    struct StartData
    {
        FunctorGeneric<void*> iCallback;
        void* iArg;
        Semaphore* iSema;
    };


protected:
    INetwork& iNetwork;
    ILog& iLog;
    DisposeHandler* iDisposeHandler;

private:
    IInjectorDevice& iDevice;
    Brn iUdn;
    TUint iRefCount;
    std::vector<ServiceCreateData*> iSubscriptionsData;
    mutable Mutex iMutexSubscribe;
    TBool iMockSubscribe;
    TBool iSubscribed;
    mutable Mutex iMutexSemas;
    std::vector<OpenHome::Semaphore*> iSemas;
};


/////////////////////////////////////////////////////


} // Topology
} // OpenHome


