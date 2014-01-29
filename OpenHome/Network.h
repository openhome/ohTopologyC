#ifndef HEADER_NETWORK
#define HEADER_NETWORK

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{

/*
class IEventSupervisorSession : public IDisposable
{
    IDisposable Create(string aId, Action<string, uint> aHandler);
}


////////////////////////////////////////////////////////////////////

class IEventSupervisor : public IWatchableThread
{
    IEventSupervisorSession Create(const Brx& aEndpoint) = 0;
    IWatchable<TUint> Servers() = 0; // { get; }
    IWatchable<TUint> Alive() = 0; // { get; }
}
*/

////////////////////////////////////////////////////////////////////

class INetwork : public IWatchableThread, public IDisposable
{
public:
    //virtual IIdCache& IdCache() = 0;
    //virtual ITagManager& TagManager() = 0;
    //virtual IEventSupervisor& EventSupervisor() = 0;
    virtual IWatchableUnordered<IDevice*>* Create() = 0;
};

//////////////////////////////////////////////////////////////////

class Network : public INetwork
{
public:
    Network(TUint aMaxCacheEntries, ILog& aLog);
    Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog& aLog);

    void Wait();
    void Add(IInjectorDevice& aDevice);
    void Remove(IInjectorDevice& aDevice);

    // INetwork
    virtual IWatchableUnordered<IDevice*>* Create();
    //virtual IIdCache& IdCache();
    //virtual ITagManager& TagManager();
    //virtual IEventSupervisor& EventSupervisor();

    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);

    // IDisposable
    virtual void Dispose();

private:
    void ReportException(Exception& aException);
    TBool WaitDevices();
    void AddCallback(void*);
    void RemoveCallback(void*);
    void WaitDevicesCallback(void*);
    void DoNothing(void*);
    void DisposeCallback(void*);

private:
    std::vector<Exception> iExceptions;
    IWatchableThread* iWatchableThread;
    Action iDispose;
    //DisposeHandler& iDisposeHandler;
    //IdCache& iCache;
    //ITagManager &iTagManager;
    //EventSupervisor& iEventSupervisor;
    std::map<Brn, Device*, BufferCmp> iDevices;
    //std::map<Type, WatchableUnordered<IDevice>*> iDeviceLists;
};



} // Av

} // OpenHome


#endif // HEADER_NETWORK
