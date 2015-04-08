#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/TagManager.h>
#include <OpenHome/IdCache.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Net/Core/CpDevice.h>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;



/////////////////////////////////////////////////////////////////

/**

 */
Network::Network(TUint aMaxCacheEntries, ILog&/* aLog*/)
    :iDisposeHandler(new DisposeHandler())
    ,iIdCache(new OpenHome::Topology::IdCache(aMaxCacheEntries))
    ,iTagManager(new TagManager())
    //,iEventSupervisor(new EventSupervisor(iWatchableThread))
    ,iAsyncAdaptorManager(new AsyncAdaptorManager())
    ,iSenderEmpty(new Sender())
    ,iInfoMetadataEmpty(new InfoMetadata())
    ,iSenderMetadataEmpty(new SenderMetadata())
    ,iInfoDetailsEmpty(new InfoDetails())
    ,iInfoMetatextEmpty(new InfoMetatext())
{
    iWatchableThread = new WatchableThread(*this);
}


/**

 */
Network::Network(IWatchableThread& aWatchableThread, TUint aMaxCacheEntries, ILog&)
    :iDisposeHandler(new DisposeHandler())
    ,iWatchableThread(&aWatchableThread)
    ,iIdCache(new OpenHome::Topology::IdCache(aMaxCacheEntries))
    ,iTagManager(new TagManager())
    //,iEventSupervisor(new EventSupervisor(iWatchableThread);)
    ,iAsyncAdaptorManager(new AsyncAdaptorManager())
    ,iSenderEmpty(new Sender())
    ,iInfoMetadataEmpty(new InfoMetadata())
    ,iSenderMetadataEmpty(new SenderMetadata())
    ,iInfoDetailsEmpty(new InfoDetails())
    ,iInfoMetatextEmpty(new InfoMetatext())
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

    delete iSenderEmpty;
    delete iInfoMetadataEmpty;
    delete iSenderMetadataEmpty;
    delete iInfoDetailsEmpty;
    delete iInfoMetatextEmpty;
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


Sender* Network::SenderEmpty()
{
    return(iSenderEmpty);
}


InfoMetadata* Network::InfoMetadataEmpty()
{
    return(iInfoMetadataEmpty);
}

SenderMetadata* Network::SenderMetadataEmpty()
{
    return(iSenderMetadataEmpty);
}


InfoDetails* Network::InfoDetailsEmpty()
{
    return(iInfoDetailsEmpty);
}


InfoMetatext* Network::InfoMetatextEmpty()
{
    return(iInfoMetatextEmpty);
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
    TBool complete = true;

    for(auto it = iDevices.begin(); it!=iDevices.end(); it++)
    {
        complete &= it->second->Wait();
    }

    return (complete);

}


/**

 */
void Network::Wait()
{
    //LOG(kTrace, "Network::Wait \n");
    for (;;)
    {
        Execute();

        if (!WaitDevices())
        {
            continue;
        }

        Execute();

        if (WaitDevices())
        {
            break;
        }
    }
}


void Network::Add(CpDevice* aDevice)
{
    Log::Print("Network::Add()  \n");

    Brh value;

    if (aDevice->GetAttribute("Upnp.Service.schemas-upnp-org.ContentDirectory", value))
    {
        //if (uint.Parse(value) == 1)
        if (Ascii::Uint(value) == 1)
        {
            //iInjectorMediaEndpoint.Add(aDevice);
            return;
        }
    }

    Add(Create(aDevice));
}

void Network::Remove(CpDevice* aDevice)
{
    Schedule(MakeFunctorGeneric(*this, &Network::RemoveCallback), aDevice);

    //if (!iInjectorMediaEndpoint.Remove(udn))
    //{
        //Schedule(() =>
        //{
        //    Remove(udn);
        //});
    //}
}

void Network::RemoveCallback(void* aDevice)
{
    auto cpDevice = (CpDevice*)aDevice;
    Remove(Brn(cpDevice->Udn()));
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
    Remove(aDevice->Udn());
}

void Network::Remove(const Brx& aUdn)
{
    Assert();
    DisposeLock lock(*iDisposeHandler);

    if (iDevices.count(Brn(aUdn))>0)
    {
        Device* device = iDevices[Brn(aUdn)];

        for(auto it = iDeviceLists.begin(); it!=iDeviceLists.end(); it++)
        {
            if (device->HasService(it->first))
            {
                it->second->Remove(device);
            }
        }

        iDevices.erase(device->Udn());
        device->Dispose();
        delete device;
        //iCache.Remove(string.Format(ServicePlaylist.kCacheIdFormat, aUdn));
        //iCache.Remove(string.Format(ServiceRadio.kCacheIdFormat, aUdn));
    }
}


IInjectorDevice* Network::Create(CpDevice* /*aDevice*/)
{
/*
    Brh value;

    InjectorDevice device = new InjectorDevice(*this, aDevice->Udn());

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Product"), value))
    {
        ICpProxyLinnCoUkVolkano1 service = null;
        if (uint.Parse(value) == 1)
        {
            if (aDevice.GetAttribute(Brn("Upnp.Service.linn-co-uk.Volkano"), value))
            {
                if (uint.Parse(value) == 1)
                {
                    service = new CpProxyLinnCoUkVolkano1(aDevice);
                }
            }
            device.Add<IProxyProduct>(new ServiceProductNetwork(device, new CpProxyAvOpenhomeOrgProduct1(aDevice), service, iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Info"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyInfo>(new ServiceInfoNetwork(device, new CpProxyAvOpenhomeOrgInfo1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Time"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyTime>(new ServiceTimeNetwork(device, new CpProxyAvOpenhomeOrgTime1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Sender"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxySender>(new ServiceSenderNetwork(device, new CpProxyAvOpenhomeOrgSender1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Volume"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyVolume>(new ServiceVolumeNetwork(device, new CpProxyAvOpenhomeOrgVolume1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Playlist"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyPlaylist>(new ServicePlaylistNetwork(device, new CpProxyAvOpenhomeOrgPlaylist1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Radio"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyRadio>(new ServiceRadioNetwork(device, new CpProxyAvOpenhomeOrgRadio1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Receiver"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyReceiver>(new ServiceReceiverNetwork(device, new CpProxyAvOpenhomeOrgReceiver1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.av-openhome-org.Credentials"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyCredentials>(new ServiceCredentialsNetwork(device, aDevice, iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.linn-co-uk.Sdp"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxySdp>(new ServiceSdpNetwork(device, new CpProxyLinnCoUkSdp1(aDevice), iLog));
        }
    }

    if (aDevice.GetAttribute(Brn("Upnp.Service.linn-co-uk.Volkano"), value))
    {
        if (uint.Parse(value) == 1)
        {
            device.Add<IProxyVolkano>(new ServiceVolkanoNetwork(device, new CpProxyLinnCoUkVolkano1(aDevice), iLog));
        }
    }

    return device;
*/
    return(NULL);
}


/*
Brn Network::GetDeviceElementValue(IEnumerable<XElement> aElements, string aName)
{
    var children = aElements.Descendants(XName.Get(aName, "urn:schemas-upnp-org:device-1-0"));

    if (children.Any())
    {
        return (children.First().Value);
    }

    return (null);
}

Brn Network::GetOpenHomeElementValue(IEnumerable<XElement> aElements, string aName)
{
    var children = aElements.Descendants(XName.Get(aName, "http://www.openhome.org"));

    if (children.Any())
    {
        return (children.First().Value);
    }

    return (null);
}
*/


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


void Network::Execute()
{
    iWatchableThread->Execute();
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
    //iIdCache.Dispose();
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



