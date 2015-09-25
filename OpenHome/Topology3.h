#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology2.h>
#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <vector>
#include <map>



namespace OpenHome
{
namespace Topology
{

class Topology3;

/////////////////////////////////////////////////////////

////////////////////////////////////////////////////////

class ITopology3Group : public ITopology2Group
{
public:
    virtual ~ITopology3Group() {}
    virtual IWatchable<ISender*>& Sender() = 0;
};

/////////////////////////////////////////////////////////

class Topology3Group : public ITopology3Group, public INonCopyable
{
    friend class ReceiverWatcher;

public:
    Topology3Group(INetwork& aNetwork, ITopology2Group& aGroup);
    ~Topology3Group();


    // IDisposable
    virtual void Dispose();

    // ITopology3Group
    virtual Brn Id();
    virtual Brn Attributes();
    virtual Brn ModelName();
    virtual Brn ManufacturerName();
    virtual Brn ProductId();
    virtual Brn ProductImageUri();
    virtual IDevice& Device();
    virtual IWatchable<Brn>& RoomName();
    virtual IWatchable<Brn>& Name();
    virtual IWatchable<TBool>& Standby();
    virtual IWatchable<TUint>& SourceIndex();
    virtual std::vector<Watchable<ITopology2Source*>*>& Sources();
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);

    virtual IWatchable<ISender*>& Sender();

private:
    virtual void SetSender(ISender* aSender);

private:
    INetwork& iNetwork;
    ITopology2Group& iGroup;
    ISender* iCurrentSender;
    Watchable<ISender*>* iSender;
    TBool iDisposed;
};

/////////////////////////////////////////////////////////

class ReceiverWatcher : public IWatcher<Brn>, public IWatcher<IInfoMetadata*>, public IWatcher<ITopology2Source*>, public IDisposable, public INonCopyable
{
public:
    ReceiverWatcher(Topology3& aTopology3, Topology3Group& aGroup);
    ~ReceiverWatcher();

    // IDisposable
    virtual void Dispose();
    virtual const Brx& ListeningToUri();
    virtual void SetSender(ISender* aSender);

    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);

    // IWatcher<IInfoMetadata*>
    virtual void ItemOpen(const Brx& aId, IInfoMetadata* aValue);
    virtual void ItemUpdate(const Brx& aId, IInfoMetadata* aValue, IInfoMetadata* aPrevious);
    virtual void ItemClose(const Brx& aId, IInfoMetadata* aValue);

    // IWatcher<ITopology2Source*>
    virtual void ItemOpen(const Brx& aId, ITopology2Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology2Source* aValue);

private:
    void CreateCallback(IProxy* aProxy);

private:
    TBool iDisposed;
    Topology3& iTopology3;
    Topology3Group& iGroup;
    IProxyReceiver* iReceiver;
    Bws<100> iTransportState;
    IInfoMetadata* iMetadata;
};

/////////////////////////////////////////////////////////

class SenderWatcher : public IWatcher<ISenderMetadata*>, public IDisposable, public INonCopyable
{
public:
    SenderWatcher(Topology3& aTopology3, ITopology2Group& aGroup);
    ~SenderWatcher();
    // IDisposable
    virtual void Dispose();
    virtual const Brx& Uri();
    virtual IDevice& Device();

    // IWatcher<ISenderMetadata*>
    virtual void ItemOpen(const Brx& aId, ISenderMetadata* aValue);
    virtual void ItemUpdate(const Brx& aId, ISenderMetadata* aValue, ISenderMetadata* aPrevious);
    virtual void ItemClose(const Brx& aId, ISenderMetadata* aValue);

private:
    void CreateCallback(IProxy* aProxy);


private:
    Topology3& iTopology3;
    DisposeHandler* iDisposeHandler;
    IDevice& iDevice;
    IProxySender* iSender;
    ISenderMetadata* iMetadata;
    TBool iDisposed;
};

/////////////////////////////////////////////////////////

class ITopology3 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopology3Group*>& Groups() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology3() {}
};

/////////////////////////////////////////////////////////

class Topology3 : public ITopology3, public IWatcherUnordered<ITopology2Group*>, public INonCopyable
{
    friend class ReceiverWatcher;
    friend class SenderWatcher;

public:
    Topology3(ITopology2* aTopology2, ILog& aLog);
    ~Topology3();

    virtual void Dispose();
    virtual IWatchableUnordered<ITopology3Group*>& Groups();
    virtual INetwork& Network();

    // IWatcherUnordered<ITopology2Group*>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(ITopology2Group* aItem);
    virtual void UnorderedRemove(ITopology2Group* aItem);
    virtual void UnorderedClose();

private:
    void WatchT2Groups(void*);
    void DisposeCallback(void*);
    void ReceiverChanged(ReceiverWatcher& aReceiver);
    void SenderChanged(IDevice& aDevice, ISenderMetadata& aMetadata, ISenderMetadata& aPreviousMetadata);

private:
    ITopology2* iTopology2;
    INetwork& iNetwork;
    std::map<ITopology2Group*, Topology3Group*> iGroupLookup;
    std::map<ITopology2Group*, ReceiverWatcher*> iReceiverLookup;
    std::map<ITopology2Group*, SenderWatcher*> iSenderLookup;
    WatchableUnordered<ITopology3Group*>* iGroups;
    TBool iDisposed;
};

} // Topology
} // OpenHome


