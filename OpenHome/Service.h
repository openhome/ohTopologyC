#ifndef HEADER_SERVICE
#define HEADER_SERVICE

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/Job.h>
#include <vector>

EXCEPTION(ServiceNotFoundException)


namespace OpenHome
{
namespace Av
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

/*
template <class T>
class Proxy : public IProxy
{
public:
    // IProxy
    virtual IDevice& Device();
    // IDisposable
    virtual void Dispose();

protected:
    Proxy(T aService, IDevice& aDevice);

protected:
    T& iService;
private:
    IDevice& iDevice;
};

////////////////////////////////////////////////

template <class T>
Proxy<T>::Proxy(T aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}


template <class T>
IDevice& Proxy<T>::Device()
{
    return (iDevice);
}


template <class T>
void Proxy<T>::Dispose()
{
    iService.Unsubscribe();
}


*/

////////////////////////////////////////////////

struct ServiceCreateData
{
    FunctorGeneric<ServiceCreateData*> iCallback1;
    FunctorGeneric<ServiceCreateData*> iCallback2;
    IDevice* iDevice;
    IProxy* iProxy;
    TBool iCancelled;
};

////////////////////////////////////////////////

class IService : public IMockable, public IDisposable
{
public:
    virtual void Create(FunctorGeneric<ServiceCreateData*>, IDevice* aDevice) = 0;
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
    virtual void Create(FunctorGeneric<ServiceCreateData*> aCallback, IDevice* aDevice);

    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);


    // IMockable
    virtual void Execute(ICommandTokens& aCommand);

    // IDisposable
    virtual void Dispose();

protected:
    Service(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

    virtual void OnSubscribe(ServiceCreateData& aServiceCreateData);
    Job* Start(FunctorGeneric<void*> aAction);
    //JobDone* Start();
    //Task<T> Start<T>(Func<T> aFunction);
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    //void HandleAggregate(AggregateException aException);
    void DisposeCallback(void*);
    void CreateCallback1(ServiceCreateData* aArgs);
    void CreateCallback2(void* aArgs);
    void StartCallback1(void* aArgs);
    void StartCallback2(void* aArgs);



protected:
    INetwork& iNetwork;
    ILog& iLog;
    DisposeHandler* iDisposeHandler;
    Semaphore iSemaSubscribe;
    Semaphore* iSubscribeTask;

private:
    IInjectorDevice& iDevice;
    //CancellationTokenSource iCancelSubscribe;
    std::vector<Job*> iJobs;
    TUint iRefCount;
    mutable Mutex iMutexJobs;
};


/////////////////////////////////////////////////////


} // Av
} // OpenHome


#endif // HEADER_SERVICE
