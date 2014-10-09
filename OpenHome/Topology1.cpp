#include <OpenHome/Topology1.h>
#include <OpenHome/OhNetTypes.h>
#include <algorithm>
#include <OpenHome/MetaData.h>
//#include <OpenHome/ServiceSender.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


Topology1::Topology1(INetwork* aNetwork, ILog& /*aLog*/)
    :iDisposed(false)
    ,iNetwork(aNetwork)
    //,iLog = aLog;
    ,iProducts(new WatchableUnordered<IProxyProduct*>(*iNetwork))
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::ExecuteCallback);
    iNetwork->Execute(f, 0);
}


Topology1::~Topology1()
{
    InfoMetadata::DestroyStatics();  // FIXME: should probably live elsewhere
    SenderMetadata::DestroyStatics(); // FIXME: should probably live elsewhere
}


void Topology1::ExecuteCallback(void*)
{
    LOG(kTrace, "Topology1::ExecuteCallback \n");
    iDevices = iNetwork->Create(eProxyProduct);
    iDevices->AddWatcher(*this);
}


void Topology1::Dispose()
{

    if (iDisposed)
    {
        //throw new ObjectDisposedException("Topology1.Dispose");
    }


    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::DisposeCallback);
    iNetwork->Execute(f, NULL);

    //iDevices = null;

    // dispose of all products, which will in turn unsubscribe
    map<IDevice*, IProxyProduct*>::iterator it;
    for(it=iProductLookup.begin(); it!=iProductLookup.end(); it++)
    {
        it->second->Dispose();
        delete it->second;
    }

    //iProductLookup = null;

    iProducts->Dispose();
    delete iProducts;
    iProducts = NULL;

    iDisposed = true;

}


void Topology1::DisposeCallback(void*)
{
    iDevices->RemoveWatcher(*this);
    iPendingSubscriptions.empty();
}


IWatchableUnordered<IProxyProduct*>& Topology1::Products()
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


void Topology1::UnorderedAdd(IDevice* aDevice)
{
    iPendingSubscriptions.push_back(aDevice);

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Topology1::UnorderedAddCallback);

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


void Topology1::UnorderedAddCallback(void* aObj)
{
    ArgsTwo<IDevice*, IProxyProduct*>* args = ((ArgsTwo<IDevice*, IProxyProduct*>*)aObj);

    IDevice* device = args->Arg1();
    IProxyProduct* product = args->Arg2();
    delete args;

    vector<IDevice*>::iterator it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), device);

    if ((!iDisposed) && (it!=iPendingSubscriptions.end()))
    {
        try
        {
            iProducts->Add(product);
        }
        catch (ServiceNotFoundException)
        {
            // NOTE: we need to log the fact that product is not added due to a service not being found
            product->Dispose();
            delete product;
            return;
        }
        iProductLookup[device] = product;
        iPendingSubscriptions.erase(it);
    }
    else
    {
        product->Dispose();
        delete product;
    }
}


void Topology1::UnorderedRemove(IDevice* aDevice)
{
    vector<IDevice*>::iterator it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), aDevice);
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


