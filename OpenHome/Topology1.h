#ifndef HEADER_TOPOLOGY1
#define HEADER_TOPOLOGY1

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

namespace Av
{


class ITopology1
{
public:
    virtual IWatchableUnordered<IProxyProduct*>& Products() = 0;
    virtual INetwork& Network() = 0;
    virtual ~ITopology1() {}
};


///////////////////////////////////////////////////////

class Topology1 : public ITopology1, public IWatcherUnordered<IDevice*>, public IDisposable
{
public:
    Topology1(INetwork* aNetwork, ILog& aLog);

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
    void ExecuteCallback(void* aObj);
    void UnorderedAddCallback(void* aObj);
    void DisposeCallback(void*);


private:
    TBool iDisposed;
    INetwork* iNetwork;
    //private readonly ILog iLog;
    std::vector<IDevice*> iPendingSubscriptions;
    std::map<IDevice*, IProxyProduct*> iProductLookup;
    WatchableUnordered<IProxyProduct*>* iProducts;
    IWatchableUnordered<IDevice*>* iDevices;
};


} // namespace Av

} // namespace OpenHome


#endif //HEADER_TOPOLOGY1
