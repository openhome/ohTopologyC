#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Network.h>


using namespace OpenHome;
using namespace OpenHome::Av;

/////////////////////////////////////////////////////////////////

Network::Network(TUint aMaxCacheEntries, ILog& aLog)
    :iWatchableThread(new MockThread(ReportException))
{
    //iExceptions = new List<Exception>();
    //iWatchableThread = new WatchableThread(ReportException);
    //iDispose = () => { iWatchableThread.Dispose(); };
    //iDisposeHandler = new DisposeHandler();
    //iCache = new IdCache(aMaxCacheEntries);
    //iTagManager = new TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
    //iDevices = new Dictionary<string, Device>();
    //iDeviceLists = new Dictionary<Type, WatchableUnordered<IDevice>>();
}


Network::Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog& aLog)
    :iWatchableThread(&aWatchableThread)
{
    //iExceptions = new List<Exception>();
    //iWatchableThread = aWatchableThread;
    //iDispose = () => { };
    //iDisposeHandler = new DisposeHandler();
    //iCache = new IdCache(aMaxCacheEntries);
    //iTagManager = new TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
    //iDevices = new Dictionary<string, Device>();
    //iDeviceLists = new Dictionary<Type, WatchableUnordered<IDevice>>();
}


void Network::ReportException(Exception& aException)
{
    //lock (iExceptions)
    //{
        iExceptions.push_back(aException);
    //}
}


TBool Network::WaitDevices()
{
    TBool complete = true;
    FunctorGeneric f = MakeFunctorGeneric(*this, &Network::WaitDevicesCallback);
    Execute(f, &complete);
    return (complete);
}


void Network::WaitDevicesCallback(void* aObj)
{
    Assert();
    TBool* complete = (TBool*)aObj;

    foreach (var device in iDevices.Values)
    {
        complete &= device.Wait();
    }
}


void Network::Wait()
{
    FunctorGeneric f = MakeFunctorGeneric(*this, &Network::DoNothing);

    while (true)
    {
        while (!WaitDevices()) ;

        TUint dummy;
        Execute(f, &dummy);

        if (WaitDevices())
        {
            break;
        }
    }
}


void Network::DoNothing(void*)
{

}


void Network::Add(IInjectorDevice& aDevice)
{
    //using (iDisposeHandler.Lock())
    //{
        FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::AddCallback);
        Schedule(f, &aDevice);
    //}
}


void Network::AddCallback(void* aObj)
{
    Assert();

    IInjectorDevice* device = (IInjectorDevice*)aObj;

    Device* handler = new Device(device);

    if (iDevices.count(handler.Udn())>0)
    //if (iDevices.ContainsKey(handler.Udn()))
    {
        handler.Dispose();
        return;
    }

    iDevices[handler.Udn()] = handler;

/*
    foreach (KeyValuePair<Type, WatchableUnordered<IDevice>> kvp in iDeviceLists)
    {
        if (device.HasService(kvp.Key))
        {
            kvp.Value.Add(handler);
        }
    }
*/
}


void Network::Remove(IInjectorDevice& aDevice)
{
    //using (iDisposeHandler.Lock())
    //{
        FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::RemoveCallback);
        Schedule(f, &aDevice);
    //}
}


void Network::RemoveCallback(void* aObj)
{
    Assert();

    IInjectorDevice* device = (IInjectorDevice*)aObj;

    Device* handler = NULL;

    //if (iDevices.TryGetValue(aDevice.Udn(), handler))
    if (iDevices.count(aDevice.Udn())>0)
    {
        handler = &iDevices[aDevice.Udn()];

/*
        foreach (KeyValuePair<Type, WatchableUnordered<IDevice>> kvp in iDeviceLists)
        {
            if (device.HasService(kvp.Key))
            {
                kvp.Value.Remove(handler);
            }
        }
*/
        iDevices.erase(handler.Udn());
        //iDevices.Remove(handler.Udn());

        //iCache.Remove(handler.Udn());

        handler.Dispose();
    }
}


IWatchableUnordered<IDevice*> Network::Create<T>()
{
    //using (iDisposeHandler.Lock())
    //{
        Assert();

//        Type key = typeof(T);

        WatchableUnordered<IDevice> list;

/*
        if (iDeviceLists.TryGetValue(key, out list))
        {
            return list;
        }
        else
        {
            list = new WatchableUnordered<IDevice>(iWatchableThread);
            iDeviceLists.Add(key, list);
            foreach (Device d in iDevices.Values)
            {
                if (d.HasService(key))
                {
                    list.Add(d);
                }
            }
            return list;
        }
*/
    //}
}


IIdCache& Network::IdCache()
{
    //using (iDisposeHandler.Lock())
    //{
        return iCache;
    //}
}


ITagManager& Network::TagManager()
{
    //using (iDisposeHandler.Lock())
    //{
        return (iTagManager);
    //}
}


IEventSupervisor& Network::EventSupervisor()
{
    return (iEventSupervisor);
}


void Network::Assert()
{
    iWatchableThread.Assert();
}


void Network::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Schedule(aCallback, aObj);
}


void Network::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Execute(aCallback, aObj);
}


void Network::Dispose()
{
    Wait();

    foreach (WatchableUnordered<IDevice> list in iDeviceLists.Values)
    {
        list.Dispose();
    }

    TUint dummy;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DisposeCallback);
    Execute(f, &dummy);

    iEventSupervisor.Dispose();

    iDisposeHandler.Dispose();

    iDispose();

    if (iExceptions.size() > 0)
    {
        throw (new AggregateException(iExceptions.ToArray()));
    }
}


void Network::DisposeCallback(void*)
{
    //foreach (var device in iDevices.Values)
    //{
        device.Dispose();
    //}
}


/////////////////////////////////////////////////////////////////

