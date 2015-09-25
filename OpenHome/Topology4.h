#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology3.h>
#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <vector>
#include <map>


namespace OpenHome
{
namespace Topology
{


class ICredentialsSubscription
{

};


class IProxyCredentials : public IDisposable
{
public:
    virtual void Dispose() = 0;
};

//////////////////////////////

class ITopology4Source
{
public:
    virtual TUint Index() = 0;
    virtual Brn Name() = 0;
    virtual Brn Type() = 0;
    virtual TBool Visible() = 0;
    virtual void Create(const Brx& aId, FunctorGeneric<ICredentialsSubscription*> aCallback) = 0;
    virtual ~ITopology4Source(){}
};

///////////////////////////////////

class Topology4Source : public ITopology4Source, public IDisposable, public INonCopyable
{
public:
    Topology4Source(IWatchableThread& aThread, IProxyCredentials& aProxy, ITopology2Source& aSource);

    void Dispose();
    TUint Index();
    Brn Name();
    Brn Type();
    TBool Visible();
    void Create(const Brx& aId, FunctorGeneric<ICredentialsSubscription*> aCallback);

private:
    //IWatchableThread& iThread;
    //IProxyCredentials& iProxy;
    ITopology2Source& iSource;
};

///////////////////////////////////////////////////////

class ITopology4Group
{
public:
    virtual Brn Id() = 0;
    virtual Brn Attributes() = 0;
    virtual Brn ModelName() = 0;
    virtual Brn ManufacturerName() = 0;
    virtual Brn ProductId() = 0;
    virtual Brn ProductImageUri() = 0;
    virtual IDevice& Device() = 0;

    virtual IWatchable<Brn>& RoomName() = 0;
    virtual IWatchable<Brn>& Name() = 0;
    virtual IWatchable<TBool>& Standby() = 0;
    virtual IWatchable<TUint>& SourceIndex() = 0;
    virtual std::vector<Watchable<ITopology4Source*>*>& Sources() = 0;
    virtual IWatchable<ISender*>& Sender() = 0;

    virtual void SetStandby(TBool aValue) = 0;
    virtual void SetSourceIndex(TUint aValue) = 0;
};

///////////////////////////////////////////////////////

class Topology4Group : public ITopology4Group, public IWatcher<ITopology2Source*>, public IDisposable, public INonCopyable
{
    friend class IWatcher<ITopology2Source*>;

public:
    Topology4Group(INetwork& aNetwork, ITopology3Group& aGroup, ILog& aLog);
    Topology4Group(INetwork& aNetwork, ITopology3Group& aGroup, IProxyCredentials& aProxy, ILog& aLog);
    ~Topology4Group();

    void Dispose();

    Brn Id();
    Brn Attributes();
    Brn ModelName();
    Brn ManufacturerName();
    Brn ProductId();
    Brn ProductImageUri();
    IDevice& Device();

    IWatchable<Brn>& RoomName();
    IWatchable<Brn>& Name();
    IWatchable<TBool>& Standby();
    IWatchable<TUint>& SourceIndex();

    std::vector<Watchable<ITopology4Source*>*>& Sources();
    IWatchable<ISender*>& Sender();

    void SetStandby(TBool aValue);
    void SetSourceIndex(TUint aValue);


private:
    virtual void ItemOpen(const Brx& aId, ITopology2Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology2Source* aValue);

private:
    INetwork& iNetwork;
    ITopology3Group& iGroup;
    std::vector<Watchable<ITopology4Source*>*> iWatchableSources;
    std::vector<ITopology4Source*> iSources;
    std::map<ITopology2Source*, Watchable<ITopology4Source*>*> iSourceLookup;
    IProxyCredentials* iProxy;
};

///////////////////////////////////////////////////////////////////////

class ITopology4 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopology4Group*>& Groups() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology4(){}
};

///////////////////////////////////////////////////////////////////////

class Topology4 : public ITopology4, public IWatcherUnordered<ITopology3Group*>, public INonCopyable
{
public:
    Topology4(ITopology3* aTopology3, ILog& aLog);
    ~Topology4();

    // IDisposable
    void Dispose();
    IWatchableUnordered<ITopology4Group*>& Groups();
    INetwork& Network();

private:
    // IUnorderedWatcher<ITopology3Group*>
    void UnorderedOpen();
    void UnorderedInitialised();
    void UnorderedClose();
    void UnorderedAdd(ITopology3Group* aItem);
    void UnorderedRemove(ITopology3Group* aItem);
    void UnorderedAddCallback(void* aItem);
    void UnorderedRemoveCallback(void* aItem);

    void CreateGroup(ITopology3Group& aGroup3, Topology4Group* aGroup4);
    void WatchT3Groups(void*);
    void DisposeCallback(void*);


private:
    ITopology3* iTopology3;
    ILog& iLog;
    INetwork& iNetwork;
    DisposeHandler* iDisposeHandler;
    TBool iDisposed;

    WatchableUnordered<ITopology4Group*>* iGroups;
    std::map<ITopology3Group*, Topology4Group*> iGroupsLookup;
    std::vector<ITopology3Group*> iPendingSubscriptions;
};

}

}



