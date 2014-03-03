#include <OpenHome/Topology1.h>
#include <OpenHome/OhNetTypes.h>
#include <algorithm>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


Topology1::Topology1(INetwork* aNetwork, ILog& /*aLog*/)
    :iDisposed(false)
    ,iNetwork(aNetwork)
    //,iLog = aLog;
    ,iProducts(new WatchableUnordered<IProxyProduct>(*iNetwork))
{

/*
    iNetwork.Execute(() =>
    {
        iDevices = iNetwork.Create<IProxyProduct>();
        iDevices.AddWatcher(this);
    });
*/

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::ExecuteCallback);
    iNetwork->Execute(f, 0);
}


void Topology1::ExecuteCallback(void*)
{
    LOG(kTrace, "Topology1::ExecuteCallback \n");
    iDevices = iNetwork->Create(EProxyProduct);
    iDevices->AddWatcher(*this);
}


void Topology1::Dispose()
{
/*
    if (iDisposed)
    {
        throw new ObjectDisposedException("Topology1.Dispose");
    }

    iNetwork.Execute(() =>
    {
        iDevices.RemoveWatcher(this);
        iPendingSubscriptions.Clear();
    });
    iDevices = null;

    // dispose of all products, which will in turn unsubscribe
    foreach (var p in iProductLookup.Values)
    {
        p.Dispose();
    }
    iProductLookup = null;

    iProducts->Dispose();
    iProducts = null;

    iDisposed = true;
*/
}


IWatchableUnordered<IProxyProduct>& Topology1::Products()
{
    return(*iProducts);
}


INetwork& Topology1::Network()
{
    return(*iNetwork);
}


void Topology1::UnorderedOpen()
{
}


void Topology1::UnorderedInitialised()
{
}


void Topology1::UnorderedClose()
{
}


void Topology1::UnorderedAdd(IDevice& aDevice)
{
    iPendingSubscriptions.push_back(&aDevice);

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::UnorderedAddCallback);

    aDevice.Create(f, EProxyProduct);


/*
    aDevice.Create<IProxyProduct>(product) =>
    {
        if (!iDisposed && iPendingSubscriptions.Contains(aDevice))
        {
            try
            {
                iProducts->Add(product);
            }
            catch (ServiceNotFoundException)
            {
                // NOTE: we need to log the fact that product is not added due to a service not being found
                product.Dispose();
                return;
            }
            iProductLookup.Add(aDevice, product);
            iPendingSubscriptions.Remove(aDevice);
        }
        else
        {
            product.Dispose();
        }
    });
*/
}


void Topology1::UnorderedAddCallback(void* aObj)
{
    IDevice* device = (IDevice*)aObj;
    IProxyProduct* product = ((IProxyProduct*)aObj)+1;

    vector<IDevice*>::iterator it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), device);

    if (it!=iPendingSubscriptions.end())
    //if (!iDisposed && iPendingSubscriptions.Contains(aDevice))
    {
        try
        {
            iProducts->Add(*product);
        }
        catch (ServiceNotFoundException)
        {
            // NOTE: we need to log the fact that product is not added due to a service not being found
            //product.Dispose();
            return;
        }
        iProductLookup[device] = product;
        iPendingSubscriptions.erase(it);
    }
    else
    {
        //product->Dispose();
    }
}


void Topology1::UnorderedRemove(IDevice& aDevice)
{
    vector<IDevice*>::iterator it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), &aDevice);
    if (it!=iPendingSubscriptions.end())
    {
        iPendingSubscriptions.erase(it);
        return;
    }

    if (iProductLookup.count(&aDevice)>0)
    {
        IProxyProduct* product = iProductLookup[&aDevice];
        iProducts->Remove(*product);
        iProductLookup.erase(&aDevice);
        //product->Dispose();
    }

/*
    if (iPendingSubscriptions.Contains(aItem))
    {
        iPendingSubscriptions.Remove(aItem);
        return;
    }

    IProxyProduct product;
    if (iProductLookup.TryGetValue(aItem, out product))
    {
        // schedule higher layer notification
        iProducts->Remove(product);
        iProductLookup.Remove(aItem);
        product.Dispose();
    }
*/
}


