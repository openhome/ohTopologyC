#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Debug.h>
//#include <OpenHome/TagManager.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



/////////////////////////////////////////////////////////////////

/**

 */
Network::Network(TUint /*aMaxCacheEntries*/, ILog&/* aLog*/)
    :iDisposeHandler(new DisposeHandler())
{
    iWatchableThread = new WatchableThread(*this);
    //iCache = new IdCache(aMaxCacheEntries);
    iTagManager = new OpenHome::Av::TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
}


/**

 */
Network::Network(IWatchableThread& aWatchableThread, TUint /*aMaxCacheEntries*/, ILog&)
    :iDisposeHandler(new DisposeHandler())
    ,iWatchableThread(&aWatchableThread)
{
    //iCache = new IdCache(aMaxCacheEntries);
    iTagManager = new OpenHome::Av::TagManager();
    //iEventSupervisor = new EventSupervisor(iWatchableThread);
}


Network::~Network()
{
    delete iWatchableThread;
    delete iTagManager;
}


ITagManager& Network::TagManager()
{
    DisposeLock lock(*iDisposeHandler);
    return (*iTagManager);
}


/**

 */
/*
void Network::ReportException(Exception& aException)
{
    //lock (iExceptions)
    //{
        iExceptions.push_back(aException);
    //}
}
*/

/**

 */
void Network::Report(Exception& aException)
{
    //lock (iExceptions)
    //{
        iExceptions.push_back(aException);
    //}
}


/**

 */
void Network::Report(std::exception& /*aException*/)
{
    //lock (iExceptions)
    //{
        //iExceptions.push_back(aException);
    //}
}


/**

 */
TBool Network::WaitDevices()
{
    //LOG(kTrace, "Network::WaitDevices \n");
    TBool complete = true;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::WaitDevicesCallback);
    Execute(f, &complete);
    return (complete);
}


/**

 */
void Network::WaitDevicesCallback(void* aObj)
{
    //LOG(kTrace, "Network::WaitDevicesCallback \n");
    Assert(); /// must be on watchable thread
    TBool* complete = (TBool*)aObj;

    std::map<Brn, Device*, BufferCmp>::iterator it;

    for(it = iDevices.begin(); it!=iDevices.end(); it++)
    {
        (*complete) &= it->second->Wait();
    }
}



/**

 */
void Network::Wait()
{
    //LOG(kTrace, "Network::Wait \n");

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DoNothing);

    for (;;)
    {
        while (!WaitDevices()) ;

        Execute(f, NULL);

        if (WaitDevices())
        {
            break;
        }
    }
}


/**

 */
void Network::DoNothing(void*)
{
    //LOG(kTrace, "Network::DoNothing \n");
}


/**

 */
void Network::Add(IInjectorDevice* aDevice)
{
    DisposeLock lock(*iDisposeHandler);
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::AddCallback);
    Schedule(f, aDevice);
}


/**

 */
void Network::AddCallback(void* aObj)
{
    Assert(); /// must be on watchable thread

    IInjectorDevice* injDevice = (IInjectorDevice*)aObj;
    Device* device = new Device(injDevice);

    if ( iDevices.count(device->Udn()) > 0 )
    {
        device->Dispose();

        delete device;

        return;
    }

    iDevices[device->Udn()] = device;

    std::map<EServiceType, WatchableUnordered<IDevice*>*>::iterator it;

    for(it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        if (injDevice->HasService(it->first))
        {
            it->second->Add(device);
        }
    }

}


/**

 */
void Network::Remove(IInjectorDevice* aDevice)
{
    DisposeLock lock(*iDisposeHandler);
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::RemoveCallback);
    Schedule(f, aDevice);
}


/**

 */
void Network::RemoveCallback(void* aObj)
{
    IInjectorDevice* injDevice = (IInjectorDevice*)aObj;

    if (iDevices.count(injDevice->Udn())>0)
    {
        Device* device = iDevices[injDevice->Udn()];

        map<EServiceType, WatchableUnordered<IDevice*>*>::iterator it;

        for(it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
        {
            if (injDevice->HasService(it->first))
            {
                it->second->Remove(device);
            }
        }

        iDevices.erase(device->Udn());
        //iCache.Remove(handler->Udn());
        device->Dispose();
        delete device;
    }

}


/**

 */
IWatchableUnordered<IDevice*>* Network::Create(EServiceType aServiceType)
{
    DisposeLock lock(*iDisposeHandler);
    Assert(); /// must be on watchable thread

    if (iDeviceLists.count(aServiceType)>0)
    {
        return(iDeviceLists[aServiceType]);
    }
    else
    {
        WatchableUnordered<IDevice*>* watchables = new WatchableUnordered<IDevice*>(*iWatchableThread);

        iDeviceLists[aServiceType] = watchables;

        std::map<Brn, Device*, BufferCmp>::iterator it;

        for(it=iDevices.begin(); it!=iDevices.end(); it++)
        {
            if (it->second->HasService(aServiceType))
            {
                Device* device = it->second;
                watchables->Add(device);
            }
        }

        return(watchables);
    }
}


/**
    Assert that we running on the watchable thread

 */
void Network::Assert()
{
    iWatchableThread->Assert();
}


/**
    Schedule a callback on the watchable thread

    @param[in] aCallback   The callback functor to schedule
    @param[in] aObj  A pointer to the callback function's arguments

 */
void Network::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread->Schedule(aCallback, aObj);
}


/**
    Execute a callback on the watchable thread

    @param[in] aCallback   The callback functor to execute
    @param[in] aObj  A pointer to the callback function's arguments
 */
void Network::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread->Execute(aCallback, aObj);
}


/**

 */
void Network::Dispose()
{
    Wait();

    std::map<EServiceType , WatchableUnordered<IDevice*>*>::iterator it;

    for(it=iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        it->second->Dispose();
        delete it->second;
    }

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DisposeCallback);
    Execute(f, NULL);

    //iEventSupervisor.Dispose();
    iDisposeHandler->Dispose();
    delete iDisposeHandler;

    if (iExceptions.size() > 0)
    {
        //throw (new AggregateException(iExceptions.ToArray()));
    }
}


/**

 */
void Network::DisposeCallback(void*)
{
    std::map<Brn, Device*, BufferCmp>::iterator it;

    for(it=iDevices.begin(); it!=iDevices.end(); it++)
    {
        it->second->Dispose();
    }

}



/**

 */
IIdCache& Network::IdCache()
{
    // IdCache not implemented yet
    ASSERTS();
    return(*iIdCache);
}


/**

 */
IEventSupervisor& Network::EventSupervisor()
{
    // EventSupervisor not implemented yet
    ASSERTS();
    return(*iEventSupervisor);
}



/////////////////////////////////////////////////////////////////

