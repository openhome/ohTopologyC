#include <OpenHome/Topology1.h>
#include <OpenHome/OhNetTypes.h>
#include <algorithm>
#include <OpenHome/MetaData.h>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;


Topology1::Topology1(INetwork& aNetwork)
    :iDisposed(false)
    ,iNetwork(aNetwork)
    ,iProducts(new WatchableUnordered<IProxyProduct*>(iNetwork))
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::WatchDevices);
    iNetwork.Schedule(f, 0);
}


Topology1::~Topology1()
{
    for(auto it=iProductLookup.begin(); it!=iProductLookup.end(); it++)
    {
        delete it->second;
    }

    delete iProducts;
}


void Topology1::WatchDevices(void*)
{
    LOG(kApplication7, "Topology1::ExecuteCallback \n");
    iDevices = &iNetwork.Create(eProxyProduct);
    iDevices->AddWatcher(*this);
}


void Topology1::Dispose()
{
    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology1.Dispose");
    }

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::DisposeCallback);
    iNetwork.Execute(f, NULL);

    iProducts->Dispose();
    iDisposed = true;
}


void Topology1::DisposeCallback(void*)
{
    iDevices->RemoveWatcher(*this);
    iPendingSubscriptions.empty();

    // dispose of all products, which will in turn unsubscribe
    for(auto it=iProductLookup.begin(); it!=iProductLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<IProxyProduct*>& Topology1::Products()
{
    return(*iProducts);
}


INetwork& Topology1::Network()
{
    return(iNetwork);
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


void Topology1::UnorderedAdd(IDevice* aDevice)
{
    iPendingSubscriptions.push_back(aDevice);
    FunctorGeneric<IProxy*> f = MakeFunctorGeneric(*this, &Topology1::UnorderedAddCallback);
    aDevice->Create(f, eProxyProduct);

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


void Topology1::UnorderedAddCallback(IProxy* aProxyProduct)
{
    IDevice* device = &aProxyProduct->Device();

    auto it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), device);

    if ((!iDisposed) && (it!=iPendingSubscriptions.end()))
    {
        try
        {
            iProducts->Add((IProxyProduct*)aProxyProduct);
        }
        catch (ServiceNotFoundException)
        {
            // NOTE: we need to log the fact that product is not added due to a service not being found
            aProxyProduct->Dispose();
            delete aProxyProduct;
            return;
        }
        iProductLookup[device] = (IProxyProduct*)aProxyProduct;
        iPendingSubscriptions.erase(it);
    }
    else
    {
        aProxyProduct->Dispose();
        delete aProxyProduct;
    }
}


void Topology1::UnorderedRemove(IDevice* aDevice)
{
    auto it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), aDevice);
    if (it!=iPendingSubscriptions.end())
    {
        iPendingSubscriptions.erase(it);
        return;
    }

    if (iProductLookup.count(aDevice)>0)
    {
        IProxyProduct* product = iProductLookup[aDevice];
        iProducts->Remove(product);
        iProductLookup.erase(aDevice);
        product->Dispose();
        delete product;
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


