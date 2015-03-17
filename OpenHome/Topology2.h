#ifndef HEADER_TOPOLOGY2
#define HEADER_TOPOLOGY2

#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Topology1.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Topology
{

class ITopology2Source
{
public:
    virtual TUint Index() = 0;
    virtual Brn Name() = 0;
    virtual Brn Type() = 0;
    virtual TBool Visible() = 0;
    virtual ~ITopology2Source() {};
};

////////////////////////////////////////

class Topology2Source : public ITopology2Source
{
public:
    Topology2Source(TUint aIndex, const Brx& aName, const Brx& aType, TBool aVisible);

    virtual TUint Index();
    virtual Brn Name();
    virtual Brn Type();
    virtual TBool Visible();

private:
    TUint iIndex;
    Bws<100> iName; //FIXME: random capacity value
    Bws<100> iType; //FIXME: random capacity value
    TBool iVisible;
};

////////////////////////////////

class ITopology2Group
{
public:
    virtual Brn Id() = 0;
    virtual Brn Attributes() = 0;
    virtual Brn ModelName() = 0;
    virtual Brn ManufacturerName() = 0;
    virtual Brn ProductId() = 0;
    virtual IDevice& Device() = 0;

    virtual IWatchable<Brn>& RoomName() = 0;
    virtual IWatchable<Brn>& Name() = 0;
    virtual IWatchable<TBool>& Standby() = 0;
    virtual IWatchable<TUint>& SourceIndex() = 0;

    virtual std::vector<Watchable<ITopology2Source*>*>& Sources() = 0;
    virtual void SetStandby(TBool aValue) = 0;
    virtual void SetSourceIndex(TUint aValue) = 0;
};

////////////////////////////////////////////////////////

class Topology2Group : public ITopology2Group, public IWatcher<Brn>, public INonCopyable
{
public:
    Topology2Group(IWatchableThread& aThread, const Brx& aId, IProxyProduct& aProduct);
    ~Topology2Group();

    virtual void Dispose();

    // ITopology2Group
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
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);
    virtual std::vector<Watchable<ITopology2Source*>*>& Sources();

    // IWatcher<Brn>
    virtual void ItemOpen(const Brx& aId, Brn aValue);
    virtual void ItemClose(const Brx& aId, Brn aValue);
    virtual void ItemUpdate(const Brx& aId, Brn, Brn aPrevious);


private:
    void ProcessSourceXml(const Brx& aSourceXml, TBool aInitial);

private:
    IWatchableThread& iThread;
    TBool iDisposed;
    Bws<100> iId; //FIXME: random capacity value
    IProxyProduct& iProduct;
    std::vector<ITopology2Source*> iSources;
    std::vector<Watchable<ITopology2Source*>*> iWatchableSources;
};

///////////////////////////

class ITopology2 : public IDisposable
{
public:
    virtual IWatchableUnordered<ITopology2Group*>& Groups() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology2() {}
};

//////////////////////////////////////////////////////////////////////////////////

class Topology2 : public ITopology2, public IWatcherUnordered<IProxyProduct*>, public INonCopyable
{
public:
    Topology2(ITopology1* aTopology1, ILog& aLog);
    ~Topology2();

    void Dispose();

    // ITopology2
    virtual IWatchableUnordered<ITopology2Group*>& Groups();
    virtual INetwork& Network();

    // IWatcherUnordered<IProxyProduct*>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(IProxyProduct* aItem);
    virtual void UnorderedRemove(IProxyProduct* aItem);
    virtual void UnorderedClose();

private:
    void WatchT1Products(void*);
    void DisposeCallback(void*);


private:
    ITopology1* iTopology1;
    INetwork& iNetwork;
    std::map<IProxyProduct*, Topology2Group*> iGroupLookup;
    WatchableUnordered<ITopology2Group*>* iGroups;
    TBool iDisposed;
};


} // namespace Topology
} // namespace OpenHome

#endif // HEADER_TOPOLOGY1
