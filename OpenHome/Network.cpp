#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Network.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;


/////////////////////////////////////////////////////////////////

Network::Network(TUint aMaxCacheEntries, ILog& aLog)
    :iWatchableThread(new WatchableThread(*this))
    ,iDisposable(true)
{
    //iDispose = () => { iWatchableThread.Dispose(); };
    //iDisposeHandler = new DisposeHandler();
    //iCache = new IdCache(aMaxCacheEntries);
    //iTagManager = new TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
}


Network::Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog& aLog)
    :iWatchableThread(&aWatchableThread)
    ,iDisposable(false)
{
    //iDispose = () => { };
    //iDisposeHandler = new DisposeHandler();
    //iCache = new IdCache(aMaxCacheEntries);
    //iTagManager = new TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
}

/*
void Network::ReportException(Exception& aException)
{
    //lock (iExceptions)
    //{
        iExceptions.push_back(aException);
    //}
}
*/

void Network::Report(Exception& aException)
{
    //lock (iExceptions)
    //{
        //iExceptions.push_back(aException);
    //}
}


void Network::Report(std::exception& aException)
{
    //lock (iExceptions)
    //{
        //iExceptions.push_back(aException);
    //}
}


TBool Network::WaitDevices()
{
    TBool complete = true;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::WaitDevicesCallback);
    Execute(f, &complete);
    return (complete);
}


void Network::WaitDevicesCallback(void* aObj)
{
    Assert();
    TBool* complete = (TBool*)aObj;

    std::map<Brn, Device*, BufferCmp>::iterator it;

    for(it = iDevices.begin(); it!=iDevices.end(); it++)
    {
        (*complete) &= it->second->Wait();
    }
}



void Network::Wait()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DoNothing);

    for (;;)
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
    Device* handler = new Device(*device);

    //if (iDevices.ContainsKey(handler.Udn()))
    if ( iDevices.count(handler->Udn()) > 0 )
    {
        handler->Dispose();
        return;
    }

    iDevices[handler->Udn()] = handler;

    std::map<EServiceType, WatchableUnordered<IDevice>*>::iterator it;

    for(it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        if (device->HasService(it->first))
        {
            it->second->Add(*handler);
        }
    }


//    foreach (KeyValuePair<Type, WatchableUnordered<IDevice>> kvp in iDeviceLists)
//    {
//        if (device.HasService(kvp.Key))
//        {
//            kvp.Value.Add(handler);
//        }
//    }

}


void Network::Remove(IInjectorDevice& aDevice)
{
/*
    using (iDisposeHandler.Lock())
    {
        Schedule(() =>
        {
            Device handler;

            if (iDevices.TryGetValue(aDevice.Udn, out handler))
            {
                foreach (KeyValuePair<Type, WatchableUnordered<IDevice>> kvp in iDeviceLists)
                {
                    if (aDevice.HasService(kvp.Key))
                    {
                        kvp.Value.Remove(handler);
                    }
                }

                iDevices.Remove(handler.Udn);

                iCache.Remove(handler.Udn);

                handler.Dispose();
            }
        });
    }
*/

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::RemoveCallback);
    Schedule(f, &aDevice);
}


void Network::RemoveCallback(void* aObj)
{
    IInjectorDevice* device = (IInjectorDevice*)aObj;

    if (iDevices.count(device->Udn())>0)
    {
        Device* handler = iDevices[device->Udn()];

        map<EServiceType, WatchableUnordered<IDevice>*>::iterator it;

        for(it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
        {
            if (device->HasService(it->first))
            {
                it->second->Remove(*handler);
            }
        }

        iDevices.erase(handler->Udn());
        //iCache.Remove(handler->Udn());
        handler->Dispose();
    }

}

//template <class T>
IWatchableUnordered<IDevice>* Network::Create(EServiceType aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        Assert();

//        EServiceType key = typeof(T);
        EServiceType key = aServiceType;

        //WatchableUnordered<IDevice> list;

//      if (iDeviceLists.TryGetValue(key, out list))
        if (iDeviceLists.count(aServiceType)>0)
        {
            return(iDeviceLists[key]);
        }
        else
        {
            WatchableUnordered<IDevice>* list = new WatchableUnordered<IDevice>(*iWatchableThread);

            //iDeviceLists.Add(key, list);
            iDeviceLists[key] = list;

            std::map<Brn, Device*, BufferCmp>::iterator it;

            for(it=iDevices.begin(); it!=iDevices.end(); it++)
            {
                if (it->second->HasService(key))
                {
                    Device* device = it->second;
                    list->Add(*device);
                }
            }
/*
            foreach (Device d in iDevices.Values)
            {
                if (d.HasService(key))
                {
                    list.Add(d);
                }
            }
*/
            return(list);
        }
    //}
}


void Network::Assert()
{
    iWatchableThread->Assert();
}


void Network::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread->Schedule(aCallback, aObj);
}


void Network::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread->Execute(aCallback, aObj);
}


void Network::Dispose()
{
    Wait();

    //foreach (WatchableUnordered<IDevice> list in iDeviceLists.Values)
    std::map<EServiceType , WatchableUnordered<IDevice>*>::iterator it;

    for(it=iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        it->second->Dispose();
        //list.Dispose();
    }


    TUint dummy;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DisposeCallback);
    Execute(f, &dummy);

    //iEventSupervisor.Dispose();
    //iDisposeHandler.Dispose();
    //iDispose();
    if(iDisposable)
    {
        //iWatchableThread->Dispose();
    }

    if (iExceptions.size() > 0)
    {
        //throw (new AggregateException(iExceptions.ToArray()));
    }
}


void Network::DisposeCallback(void*)
{
    std::map<Brn, Device*, BufferCmp>::iterator it;

    for(it=iDevices.begin(); it!=iDevices.end(); it++)
    {
        it->second->Dispose();
    }
    //foreach (var device in iDevices.Values)
    //{
        //device.Dispose();
    //}
}



IIdCache& Network::IdCache()
{
    // IdCache not implemented yet
    ASSERTS();
    return(*iIdCache);
}


ITagManager& Network::TagManager()
{
    // TagManager not implemented yet
    ASSERTS();
    return(*iTagManager);
}


IEventSupervisor& Network::EventSupervisor()
{
    // EventSupervisor not implemented yet
    ASSERTS();
    return(*iEventSupervisor);
}



/////////////////////////////////////////////////////////////////

