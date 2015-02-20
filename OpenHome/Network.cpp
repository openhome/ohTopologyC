#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/IdCache.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



/////////////////////////////////////////////////////////////////

/**

 */
Network::Network(TUint aMaxCacheEntries, ILog&/* aLog*/)
    :iDisposeHandler(new DisposeHandler())
    ,iIdCache(new OpenHome::Av::IdCache(aMaxCacheEntries))
    ,iTagManager(new TagManager())
    //,iEventSupervisor(new EventSupervisor(iWatchableThread))
    ,iAsyncAdaptorManager(new AsyncAdaptorManager())
    ,iTopology3SenderEmpty(new Topology3Sender())
    ,iInfoMetadataEmpty(new InfoMetadata())
    ,iSenderMetadataEmpty(new SenderMetadata())
{
    iWatchableThread = new WatchableThread(*this);
}


/**

 */
Network::Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog&)
    :iDisposeHandler(new DisposeHandler())
    ,iWatchableThread(&aWatchableThread)
    ,iIdCache(new OpenHome::Av::IdCache(aMaxCacheEntries))
    ,iTagManager(new TagManager())
    //,iEventSupervisor(new EventSupervisor(iWatchableThread);)
    ,iAsyncAdaptorManager(new AsyncAdaptorManager())
    ,iTopology3SenderEmpty(new Topology3Sender())
    ,iInfoMetadataEmpty(new InfoMetadata())
    ,iSenderMetadataEmpty(new SenderMetadata())
{
}


Network::~Network()
{
    delete iWatchableThread;
    delete iIdCache;
    delete iTagManager;
    delete iAsyncAdaptorManager;

    for(auto it=iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        delete it->second;
    }

    for(auto it2=iDevices.begin(); it2!=iDevices.end(); it2++)
    {
        delete it2->second;
    }

    delete iDisposeHandler;

    delete iTopology3SenderEmpty;
    delete iInfoMetadataEmpty;
    delete iSenderMetadataEmpty;
}


ITagManager& Network::GetTagManager()
{
    DisposeLock lock(*iDisposeHandler);
    return (*iTagManager);
}


AsyncAdaptorManager& Network::GetAsyncAdaptorManager()
{
    return(*iAsyncAdaptorManager);
}


Topology3Sender* Network::Topology3SenderEmpty()
{
    return(iTopology3SenderEmpty);
}


InfoMetadata* Network::InfoMetadataEmpty()
{
    return(iInfoMetadataEmpty);
}

SenderMetadata* Network::SenderMetadataEmpty()
{
    return(iSenderMetadataEmpty);
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

    for(auto it = iDevices.begin(); it!=iDevices.end(); it++)
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

    if ( iDevices.count(device->Udn()) > 0 ) // device (with same UDN) already exists in list
    {
        device->Dispose();
        delete device;
        return;
    }

    iDevices[device->Udn()] = device;

    for(auto it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
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

        for(auto it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
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
IWatchableUnordered<IDevice*>& Network::Create(EServiceType aServiceType)
{
    DisposeLock lock(*iDisposeHandler);
    Assert(); /// must be on watchable thread

    if (iDeviceLists.count(aServiceType)>0)
    {
        return(*iDeviceLists[aServiceType]);
    }
    else
    {
        WatchableUnordered<IDevice*>* watchables = new WatchableUnordered<IDevice*>(*iWatchableThread);

        iDeviceLists[aServiceType] = watchables;

        for(auto it=iDevices.begin(); it!=iDevices.end(); it++)
        {
            if (it->second->HasService(aServiceType))
            {
                Device* device = it->second;
                watchables->Add(device);
            }
        }

        return(*watchables);
    }
}


/**
    Assert that we're running on the watchable thread

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

    for(auto it=iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
    {
        it->second->Dispose();
    }

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &Network::DisposeCallback);
    Execute(f, NULL);

    //iEventSupervisor.Dispose();
    iDisposeHandler->Dispose();

    if (iExceptions.size() > 0)
    {
        //throw (new AggregateException(iExceptions.ToArray()));
    }
}


/**

 */
void Network::DisposeCallback(void*)
{
    for(auto it=iDevices.begin(); it!=iDevices.end(); it++)
    {
        Device* device = it->second;
        device->Dispose();
    }
}



/**

 */
IIdCache& Network::IdCache()
{
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



