#ifndef HEADER_SERVICE
#define HEADER_SERVICE


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <vector>

namespace OpenHome
{
namespace Av
{

class INetwork;
class IInjectorDevice;
class IDevice;



class IProxy //: public IDisposable
{
public:
    virtual IDevice& Device();
};

////////////////////////////////////////////////

class IService : public IMockable, public IDisposable
{
public:
    virtual void Create(FunctorGeneric<void*>, EServiceType aServiceType, IDevice* aDevice) = 0;
};

////////////////////////////////////////////////

class Service : public IService, public IWatchableThread
{
public:
    virtual TBool Wait();
    virtual void Unsubscribe();
    virtual IInjectorDevice& Device();
    virtual IProxy* OnCreate(IDevice* aDevice) = 0;

    // IService
    virtual void Create(FunctorGeneric<void*> aCallback, EServiceType aServiceType, IDevice* aDevice);

    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual TBool IsWatchableThread();


    // IMockable
    virtual void Execute(ICommandTokens& aCommand);

    // IDisposable
    virtual void Dispose();

protected:
    Service(INetwork& aNetwork, IInjectorDevice* aDevice, ILog& aLog);


    //virtual Task OnSubscribe();
    //Task Start(FunctorGeneric<void*> aAction);
    //Task<T> Start<T>(Func<T> aFunction);
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    //void HandleAggregate(AggregateException aException);
    void DisposeCallback(void*);
    void CreateCallback(void*);



protected:
    INetwork& iNetwork;
    ILog& iLog;
    //DisposeHandler* iDisposeHandler;
    //Task iSubscribeTask;

private:
    IInjectorDevice* iDevice;
    //CancellationTokenSource iCancelSubscribe;
    //std::vector<Task> iTasks;
    TUint iRefCount;
};

////////////////////////////////////////////////

template <class T>
class Proxy
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


} // Av
} // OpenHome


#endif // HEADER_SERVICE
