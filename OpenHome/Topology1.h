#ifndef HEADER_TOPOLOGY1
#define HEADER_TOPOLOGY1

#include <OpenHome/OhNetTypes.h>
//#include <OpenHome/WatchableThread.h>
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


EXCEPTION(ServiceNotFoundException)



namespace OpenHome
{

namespace Av
{


class ITopology1
{
public:
    IWatchableUnordered<IProxyProduct>& Products();
    INetwork& Network();
};


///////////////////////////////////////////////////////

class Topology1 : public ITopology1, public IWatcherUnordered<IDevice>//, public IDisposable
{
public:
    Topology1(INetwork* aNetwork, ILog& aLog);
    void Dispose();
    IWatchableUnordered<IProxyProduct>& Products();
    INetwork& Network();
    void UnorderedOpen();
    void UnorderedInitialised();
    void UnorderedClose();
    void UnorderedAdd(IDevice& aItem);
    void UnorderedRemove(IDevice& aItem);

private:
    void ExecuteCallback(void* aObj);
    void UnorderedAddCallback(void* aObj);


private:
    TBool iDisposed;
    INetwork* iNetwork;

// private readonly ILog iLog;
    std::vector<IDevice*> iPendingSubscriptions;
    std::map<IDevice*, IProxyProduct*> iProductLookup;
    WatchableUnordered<IProxyProduct>* iProducts;
    IWatchableUnordered<IDevice>* iDevices;
};


} // namespace Av

} // namespace OpenHome


#endif //HEADER_TOPOLOGY1
