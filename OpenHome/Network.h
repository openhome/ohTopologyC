#ifndef HEADER_OHTOPOLOGYC_NETWORK
#define HEADER_OHTOPOLOGYC_NETWORK

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/DisposeHandler.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/AsyncAdaptor.h>
#include <OpenHome/IdCache.h>
#include <OpenHome/ServiceSender.h>
#include <OpenHome/ServiceInfo.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Topology
{


class IdCache;
//class ITagManager;
class IEventSupervisor;
//class TagManager;



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
    virtual IIdCache& IdCache() = 0;
    virtual ITagManager& GetTagManager() = 0;
    virtual IEventSupervisor& EventSupervisor() = 0;
    virtual AsyncAdaptorManager& GetAsyncAdaptorManager() = 0;
    virtual IWatchableUnordered<IDevice*>& Create(EServiceType aServiceType) = 0;

    virtual Sender* SenderEmpty() = 0;
    virtual InfoMetadata* InfoMetadataEmpty() = 0;
    virtual SenderMetadata* SenderMetadataEmpty() = 0;
    virtual InfoDetails* InfoDetailsEmpty() = 0;
    virtual InfoMetatext* InfoMetatextEmpty() = 0;

    virtual ~INetwork() {}
};

//////////////////////////////////////////////////////////////////

class Network : public INetwork, public IExceptionReporter, public INonCopyable
{
public:
    Network(TUint aMaxCacheEntries, ILog& aLog);
    Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog& aLog);
    ~Network();

    void Wait();

    void Add(IInjectorDevice* aDevice);
    void Add(Net::CpDevice* aDevice);
    void AddCpDevice(Net::CpDevice* aDevice); // to get round FunctorGeneric with overloaded methods issue

    void Remove(IInjectorDevice* aDevice);
    void Remove(Net::CpDevice* aDevice);
    void RemoveCpDevice(Net::CpDevice* aDevice);  // to get round FunctorGeneric with overloaded methods issue


    // INetwork
    virtual IIdCache& IdCache();
    virtual ITagManager& GetTagManager();
    virtual IEventSupervisor& EventSupervisor();
    virtual IWatchableUnordered<IDevice*>& Create(EServiceType aServiceType);
    virtual IInjectorDevice* Create(Net::CpDevice* aDevice);
    virtual AsyncAdaptorManager& GetAsyncAdaptorManager();

    virtual Sender* SenderEmpty();
    virtual InfoMetadata* InfoMetadataEmpty();
    virtual SenderMetadata* SenderMetadataEmpty();
    virtual InfoDetails* InfoDetailsEmpty();
    virtual InfoMetatext* InfoMetatextEmpty();

    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute();

    // IDisposable
    virtual void Dispose();

    // IExceptionReporter
    virtual void Report(Exception& aException);
    virtual void Report(std::exception& aException);


private:
    void ReportException(Exception& aException);
    TBool WaitDevices();
    void AddCallback(void*);
    void DisposeCallback(void*);
    void RemoveCallback(void* aDevice);
    void Remove(const Brx& aUdn);
    void DeleteDevice(void* aDevice);

private:
    ILog& iLog;
    std::vector<Exception> iExceptions;
    DisposeHandler* iDisposeHandler;
    IWatchableThread* iWatchableThread;
    IIdCache* iIdCache;
    ITagManager* iTagManager;
    IEventSupervisor* iEventSupervisor;
    AsyncAdaptorManager* iAsyncAdaptorManager;
    std::map<Brn, Device*, BufferCmp> iDevices;
    std::map<EServiceType, WatchableUnordered<IDevice*>*> iDeviceLists;

    Sender* iSenderEmpty;
    InfoMetadata* iInfoMetadataEmpty;
    SenderMetadata* iSenderMetadataEmpty;
    InfoDetails* iInfoDetailsEmpty;
    InfoMetatext* iInfoMetatextEmpty;
};



} // Topology

} // OpenHome


#endif // HEADER_OHTOPOLOGYC_NETWORK
