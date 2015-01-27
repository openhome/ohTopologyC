#ifndef HEADER_TOPOLOGY3
#define HEADER_TOPOLOGY3

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
namespace Av
{

class Topology3;

/////////////////////////////////////////////////////////

// Added in ohTopologyC to get rid of Topology5Room::iGroupWatcherLookup
class ITopology3GroupWatcher : public IWatcher<Brn>, public IWatcher<ITopology2Source*>, public IDisposable
{
public:
    virtual void Dispose() = 0;
    virtual Brn RoomName() = 0;
    virtual Brn Name() = 0;
    virtual std::vector<ITopology2Source*>& Sources() = 0;

    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aValue) = 0;
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious) = 0;
    virtual void ItemClose(const Brx& aId, Brn aValue) = 0;

    // IWatcher<ITopology2Source*>
    virtual void ItemOpen(const Brx& aId, ITopology2Source* aValue) = 0;
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious) = 0;
    virtual void ItemClose(const Brx& aId, ITopology2Source* aValue) = 0;
};

////////////////////////////////////////////////////////

class ITopology3Group : public ITopology2Group
{
public:
    virtual ~ITopology3Group() {}

    virtual IWatchable<ITopology3Sender*>& Sender() = 0;

    // Added in ohTopologyC
    virtual ITopology3GroupWatcher* GroupWatcher() = 0;
    virtual void SetGroupWatcher(ITopology3GroupWatcher* aGroupWatcher) = 0;
};

/////////////////////////////////////////////////////////

class Topology3Group : public ITopology3Group, public INonCopyable
{
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
    virtual IDevice& Device();
    virtual IWatchable<Brn>& RoomName();
    virtual IWatchable<Brn>& Name();
    virtual IWatchable<TBool>& Standby();
    virtual IWatchable<TUint>& SourceIndex();
    virtual std::vector<Watchable<ITopology2Source*>*>& Sources();
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);

    virtual IWatchable<ITopology3Sender*>& Sender();
    virtual void SetSender(ITopology3Sender* aSender);

    virtual ITopology3GroupWatcher* GroupWatcher(); // Added in ohTopologyC
    virtual void SetGroupWatcher(ITopology3GroupWatcher* aGroupWatcher); // Added in ohTopologyC

private:
    INetwork& iNetwork;
    ITopology2Group& iGroup;
    Watchable<ITopology3Sender*>* iSender;
    TBool iDisposed;
    ITopology3Sender* iCurrentSender;
    ITopology3GroupWatcher* iGroupWatcher;
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
    virtual void SetSender(ITopology3Sender* aSender);

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
    void CreateCallback(ServiceCreateData* aArgs);

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
    void CreateCallback(ServiceCreateData* aArgs);


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
public:
    Topology3(ITopology2* aTopology2, ILog& aLog);
    ~Topology3();

    virtual void Dispose();
    virtual IWatchableUnordered<ITopology3Group*>& Groups();
    virtual INetwork& Network();
    virtual void ReceiverChanged(ReceiverWatcher& aReceiver);
    virtual void SenderChanged(IDevice& aDevice, const Brx& aUri, const Brx& aPreviousUri);

    // IWatcherUnordered<ITopology2Group*>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(ITopology2Group* aItem);
    virtual void UnorderedRemove(ITopology2Group* aItem);
    virtual void UnorderedClose();

private:
    void WatchT2Groups(void*);
    void DisposeCallback(void*);

private:
    ITopology2* iTopology2;
    INetwork& iNetwork;
    std::map<ITopology2Group*, Topology3Group*> iGroupLookup;
    std::map<ITopology2Group*, ReceiverWatcher*> iReceiverLookup;
    std::map<ITopology2Group*, SenderWatcher*> iSenderLookup;
    WatchableUnordered<ITopology3Group*>* iGroups;
    TBool iDisposed;
};

} // Av
} // OpenHome


#endif // HEADER_TOPOLOGY3
