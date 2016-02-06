#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/IWatcher.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Network.h>
#include <OpenHome/ServiceProduct.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>

#include <vector>
#include <map>

namespace OpenHome
{
namespace Topology
{

class ITopology1 : public IDisposable
{
public:
    virtual IWatchableUnordered<IProxyProduct*>& Products() = 0;
    virtual INetwork& Network() = 0;
    virtual void Dispose() = 0;
    virtual ~ITopology1() {}
};


///////////////////////////////////////////////////////

class Topology1 : public INonCopyable, public ITopology1, public IWatcherUnordered<IDevice*>
{
public:
    Topology1(INetwork& aNetwork);
    ~Topology1();

    // IDisposable
    virtual void Dispose();

    // ITopology1
    virtual IWatchableUnordered<IProxyProduct*>& Products();
    virtual INetwork& Network();

    // IWatcherUnordered
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose();
    virtual void UnorderedAdd(IDevice* aItem);
    virtual void UnorderedRemove(IDevice* aItem);

private:
    void WatchDevices(void* aObj);
    void UnorderedAddCallback(IProxy* aProxyProduct);
    void DisposeCallback(void*);

private:
    TBool iDisposed;
    INetwork& iNetwork;
    std::vector<IDevice*> iPendingSubscriptions;
    std::map<IDevice*, IProxyProduct*> iProductLookup;
    WatchableUnordered<IProxyProduct*>* iProducts;
    IWatchableUnordered<IDevice*>* iDevices;
};

} // namespace Topology
} // namespace OpenHome
