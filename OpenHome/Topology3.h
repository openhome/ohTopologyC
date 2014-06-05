#ifndef HEADER_TOPOLOGYM
#define HEADER_TOPOLOGYM

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

class ITopology3Sender
{
public:
    virtual TBool Enabled() = 0;
    virtual IDevice& Device() = 0;
    virtual ~ITopology3Sender() {}
};

/////////////////////////////////////////////////////////

class Topology3Sender : public ITopology3Sender
{
public:
    static Topology3Sender* Empty();
    static void DestroyStatics();

    Topology3Sender(IDevice& aDevice);

    // ITopology3Sender
    virtual TBool Enabled();
    virtual IDevice& Device();

private:
    Topology3Sender();

private:
    TBool iEnabled;
    IDevice* iDevice;
    static Topology3Sender* iEmpty;
};

/////////////////////////////////////////////////////////

class ITopology3Group
{
public:
    virtual ~ITopology3Group() {}

    virtual Brn Id() = 0;
    virtual Brn Attributes() = 0;
    virtual Brn ModelName() = 0;
    virtual Brn ManufacturerName() = 0;
    virtual Brn ProductId() = 0;
    virtual IDevice& Device() = 0;

    virtual IWatchable<Brn>& Room() = 0;
    virtual IWatchable<Brn>& Name() = 0;
    virtual IWatchable<TBool>& Standby() = 0;
    virtual IWatchable<TUint>& SourceIndex() = 0;
    //virtual IEnumerable<IWatchable<ITopology2Source*>*>& Sources() = 0;
    virtual std::vector<Watchable<ITopology2Source*>*>& Sources() = 0;
    virtual IWatchable<ITopology3Sender*>& Sender() = 0;

    virtual void SetStandby(TBool aValue) = 0;
    virtual void SetSourceIndex(TUint aValue) = 0;
};

/////////////////////////////////////////////////////////

class Topology3Group : public ITopology3Group, public INonCopyable
{
public:
    Topology3Group(INetwork& aNetwork, ITopology2Group& aGroup);

    virtual void SetSender(ITopology3Sender* aSender);

    // IDisposable
    virtual void Dispose();

    // ITopology3Group
    virtual Brn Id();
    virtual Brn Attributes();
    virtual Brn ModelName();
    virtual Brn ManufacturerName();
    virtual Brn ProductId();
    virtual IDevice& Device();
    virtual IWatchable<Brn>& Room();
    virtual IWatchable<Brn>& Name();
    virtual IWatchable<TBool>& Standby();
    virtual IWatchable<TUint>& SourceIndex();
    //virtual IEnumerable<IWatchable<ITopology2Source*>*> Sources();
    virtual std::vector<Watchable<ITopology2Source*>*>& Sources();

    virtual IWatchable<ITopology3Sender*>& Sender();
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);


private:
    ITopology2Group& iGroup;
    Watchable<ITopology3Sender*>* iSender;
    TBool iDisposed;
    ITopology3Sender* iCurrentSender;
};

/////////////////////////////////////////////////////////

class ReceiverWatcher : public IWatcher<Brn>, public IWatcher<IInfoMetadata*>, public IWatcher<ITopology2Source*>, public IDisposable, public INonCopyable
{
public:
    ReceiverWatcher(Topology3& aTopology, Topology3Group& aGroup);

    // IDisposable
    virtual void Dispose();
    virtual const Brx& ListeningToUri();
    virtual void SetSender(ITopology3Sender* aSender);

    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    virtual void ItemClose(const Brx& aId, Brn aValue);

    virtual void ItemOpen(const Brx& aId, IInfoMetadata* aValue);
    virtual void ItemUpdate(const Brx& aId, IInfoMetadata* aValue, IInfoMetadata* aPrevious);
    virtual void ItemClose(const Brx& aId, IInfoMetadata* aValue);

    virtual void ItemOpen(const Brx& aId, ITopology2Source* aValue);
    virtual void ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious);
    virtual void ItemClose(const Brx& aId, ITopology2Source* aValue);

private:
    void CreateCallback(void* aArgs);

private:
    TBool iDisposed;
    Topology3& iTopology;
    Topology3Group& iGroup;
    IProxyReceiver* iReceiver;
    Bws<100> iTransportState;
    IInfoMetadata* iMetadata;
};

/////////////////////////////////////////////////////////

class SenderWatcher : public IWatcher<ISenderMetadata*>, public IDisposable, public INonCopyable
{
public:
    SenderWatcher(Topology3& aTopology, ITopology2Group& aGroup);
    // IDisposable
    virtual void Dispose();
    virtual const Brx& Uri();
    virtual IDevice& Device();
    // IWatcher
    virtual void ItemOpen(const Brx& aId, ISenderMetadata* aValue);
    virtual void ItemUpdate(const Brx& aId, ISenderMetadata* aValue, ISenderMetadata* aPrevious);
    virtual void ItemClose(const Brx& aId, ISenderMetadata* aValue);

private:
    void CreateCallback(void* aArgs);


private:
    Topology3& iTopology;
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
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(ITopology2Group* aItem);
    virtual void UnorderedRemove(ITopology2Group* aItem);
    virtual void UnorderedClose();
    virtual void ReceiverChanged(ReceiverWatcher& aReceiver);
    virtual void SenderChanged(IDevice& aDevice, const Brx& aUri, const Brx& aPreviousUri);

private:
    void ScheduleCallback(void*);
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


#endif // HEADER_TOPOLOGYM
