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


class Topologym;

/*
class IInfoMetadata
{
public:
    //IMediaMetadata& Metadata = 0;
    virtual Brn Uri() = 0;
};
*/

/////////////////////////////////////////////////////////

class ITopologymSender
{
public:
    virtual TBool Enabled() = 0;
    virtual IDevice& Device() = 0;
};

/////////////////////////////////////////////////////////

class TopologymSender : public ITopologymSender
{
public:
    static TopologymSender* Empty();

    TopologymSender(IDevice& aDevice);

    // ITopologymSender
    virtual TBool Enabled();
    virtual IDevice& Device();

private:
    TopologymSender();

private:
    TBool iEnabled;
    IDevice* iDevice;
};

/////////////////////////////////////////////////////////

class ITopologymGroup
{
public:
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
    virtual IWatchable<Brn>& Registration() = 0;
    virtual IWatchable<ITopologymSender*>& Sender() = 0;

    virtual void SetStandby(TBool aValue) = 0;
    virtual void SetSourceIndex(TUint aValue) = 0;
    virtual void SetRegistration(const Brx& aValue) = 0;
};

/////////////////////////////////////////////////////////

class TopologymGroup : public ITopologymGroup
{
public:
    TopologymGroup(INetwork& aNetwork, ITopology2Group& aGroup);

    virtual void SetSender(ITopologymSender* aSender);

    // IDisposable
    virtual void Dispose();

    // ITopologymGroup
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
    virtual IWatchable<Brn>& Registration();
    virtual IWatchable<ITopologymSender*>& Sender();
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);
    virtual void SetRegistration(const Brx& aValue);


private:
    TBool iDisposed;
    ITopology2Group& iGroup;
    Watchable<ITopologymSender*>* iSender;
};

/////////////////////////////////////////////////////////

class ReceiverWatcher : public IWatcher<Brn>, public IWatcher<IInfoMetadata*>, public IWatcher<ITopology2Source*>, public IDisposable
{
public:
    ReceiverWatcher(Topologym& aTopology, TopologymGroup& aGroup);

    // IDisposable
    virtual void Dispose();
    virtual Brn ListeningToUri();
    virtual void SetSender(ITopologymSender* aSender);

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
    TBool iDisposed;
    Topologym& iTopology;
    TopologymGroup& iGroup;
    IProxyReceiver* iReceiver;
    Brn iTransportState;
    //IInfoMetadata& iMetadata;
};

/////////////////////////////////////////////////////////

class SenderWatcher : public IWatcher<ISenderMetadata*>, public IDisposable
{
public:
    SenderWatcher(Topologym& aTopology, ITopology2Group& aGroup);
    // IDisposable
    virtual void Dispose();
    virtual Brn Uri();
    virtual IDevice& Device();
    // IWatcher
    virtual void ItemOpen(const Brx& aId, ISenderMetadata* aValue);
    virtual void ItemUpdate(const Brx& aId, ISenderMetadata* aValue, ISenderMetadata* aPrevious);
    virtual void ItemClose(const Brx& aId, ISenderMetadata* aValue);

private:
    void CreateCallback(void* aSender);


private:
    Topologym& iTopology;
    DisposeHandler* iDisposeHandler;
    IDevice& iDevice;
    IProxySender* iSender;
    ISenderMetadata* iMetadata;
    TBool iDisposed;
};

/////////////////////////////////////////////////////////

class ITopologym
{
public:
    virtual IWatchableUnordered<ITopologymGroup*>& Groups() = 0;
    virtual INetwork& Network() = 0;
};

/////////////////////////////////////////////////////////

class Topologym : public ITopologym, public IWatcherUnordered<ITopology2Group*>, public IDisposable
{
public:
    Topologym(ITopology2* aTopology2, ILog& aLog);
    virtual void Dispose();
    virtual IWatchableUnordered<ITopologymGroup*>& Groups();
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
    std::map<ITopology2Group*, TopologymGroup*> iGroupLookup;
    std::map<ITopology2Group*, ReceiverWatcher*> iReceiverLookup;
    std::map<ITopology2Group*, SenderWatcher*> iSenderLookup;
    WatchableUnordered<ITopologymGroup*>* iGroups;
    TBool iDisposed;
};

} // Av
} // OpenHome


#endif // HEADER_TOPOLOGYM
