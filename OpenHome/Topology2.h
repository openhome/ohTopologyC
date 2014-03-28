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

namespace Av
{

class ITopology2Source
{
public:
    virtual TUint Index() = 0;
    virtual Brn Name() = 0;
    virtual Brn Type() = 0;
    virtual TBool Visible() = 0;
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

    virtual IWatchable<Brn>& Room() = 0;
    virtual IWatchable<Brn>& Name() = 0;
    virtual IWatchable<TBool>& Standby() = 0;
    virtual IWatchable<TUint>& SourceIndex() = 0;
    //virtual IEnumerable<IWatchable<ITopology2Source>>& Sources() = 0;
    virtual const std::vector<Watchable<ITopology2Source*>*> Sources() = 0;
    virtual IWatchable<Brn>& Registration() = 0;

    virtual void SetStandby(TBool aValue) = 0;
    virtual void SetSourceIndex(TUint aValue) = 0;
    virtual void SetRegistration(const Brx& aValue) = 0;
};

////////////////////////////////////////////////////////

class Topology2Group : public ITopology2Group, public IWatcher<Brn>, public INonCopyable
{
public:
    Topology2Group(IWatchableThread& aThread, const Brx& aId, IProxyProduct& aProduct);

    virtual void Dispose();

    // ITopology2Group
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
    virtual IWatchable<Brn>& Registration();
    virtual void SetStandby(TBool aValue);
    virtual void SetSourceIndex(TUint aValue);
    virtual void SetRegistration(const Brx& aValue);
    //virtual IEnumerable<IWatchable<ITopology2Source>>& Sources();
    virtual const std::vector<Watchable<ITopology2Source*>*> Sources();

    // IWatcher
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

class ITopology2
{
public:
    virtual IWatchableUnordered<ITopology2Group*>& Groups() = 0;
    virtual INetwork& Network() = 0;
    virtual ~ITopology2() {}
};

//////////////////////////////////////////////////////////////////////////////////

class Topology2 : public ITopology2, public IWatcherUnordered<IProxyProduct*>, public IDisposable, public INonCopyable
{
public:
    Topology2(ITopology1* aTopology1, ILog& aLog);
    void Dispose();

    // ITopology2
    virtual IWatchableUnordered<ITopology2Group*>& Groups();
    virtual INetwork& Network();

    // IWatcherUnordered
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(IProxyProduct* aItem);
    virtual void UnorderedRemove(IProxyProduct* aItem);
    virtual void UnorderedClose();

private:
    void ScheduleCallback(void*);
    void ExecuteCallback(void*);


private:
    ITopology1* iTopology1;
    INetwork& iNetwork;
    std::map<IProxyProduct*, Topology2Group*> iGroupLookup;
    WatchableUnordered<ITopology2Group*>* iGroups;
    TBool iDisposed;
};


} // namespace Av
} // namespace OpenHome

#endif // HEADER_TOPOLOGY1
