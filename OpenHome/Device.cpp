#include<OpenHome/Device.h>
#include<OpenHome/OhTopologyC.h>
#include<OpenHome/Buffer.h>
#include<OpenHome/Service.h>
#include <algorithm>
#include <vector>
#include <map>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;



Device::Device(IInjectorDevice* aDevice)
    :iDevice(aDevice)
    ,iDisposeHandler(new DisposeHandler())
{
}


Device::~Device()
{
    delete iDevice;
    delete iDisposeHandler;
}


Brn Device::Udn()
{
    DisposeLock lock(*iDisposeHandler);
    Brn udn(iDevice->Udn());

    return(udn);
}


void Device::Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType)
{
    DisposeLock lock(*iDisposeHandler);
    iDevice->Create(aCallback, aServiceType, *this);
}


void Device::Join(Functor aAction)
{
    DisposeLock lock(*iDisposeHandler);
    iDevice->Join(aAction);
}


void Device::Unjoin(Functor aAction)
{
    DisposeLock lock(*iDisposeHandler);
    iDevice->Unjoin(aAction);
}


void Device::Dispose()
{
    iDisposeHandler->Dispose();
    iDevice->Dispose();
}


TBool Device::HasService(EServiceType aServiceType)
{
    DisposeLock lock(*iDisposeHandler);
    return iDevice->HasService(aServiceType);
}


TBool Device::Wait()
{
    DisposeLock lock(*iDisposeHandler);
    return iDevice->Wait();
}

////////////////////////////////////////////////////////////////////////////


InjectorDeviceAdaptor::InjectorDeviceAdaptor(IInjectorDevice& aDevice)
    :iDevice(aDevice)
{
}


void InjectorDeviceAdaptor::Join(Functor aAction)
{
    iDevice.Join(aAction);
}


void InjectorDeviceAdaptor::Unjoin(Functor aAction)
{
    iDevice.Unjoin(aAction);
}


Brn InjectorDeviceAdaptor::Udn()
{
    return iDevice.Udn();
}


INetwork& InjectorDeviceAdaptor::Network() const
{
    return iDevice.Network();
}

void InjectorDeviceAdaptor::Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType, IDevice& aDevice)
{
    iDevice.Create(aCallback, aServiceType, aDevice);
}


TBool InjectorDeviceAdaptor::HasService(EServiceType aServiceType)
{
    return iDevice.HasService(aServiceType);
}


TBool InjectorDeviceAdaptor::Wait()
{
    return iDevice.Wait();
}


void InjectorDeviceAdaptor::Execute(ICommandTokens& aTokens)
{
    iDevice.Execute(aTokens);
}


void InjectorDeviceAdaptor::Dispose()
{
}


/////////////////////////////////////////////////////////////////

InjectorDeviceMock::InjectorDeviceMock(IInjectorDevice* aDevice)
    :iDevice(aDevice)
    ,iOn(NULL)
{
}


InjectorDeviceMock::~InjectorDeviceMock()
{
    delete iDevice;
}



IInjectorDevice* InjectorDeviceMock::On()
{
    ASSERT(iOn==NULL);
    iOn = new InjectorDeviceAdaptor(*iDevice);
    return (iOn);
}


IInjectorDevice* InjectorDeviceMock::Off()
{
    ASSERT(iOn!=NULL);
    auto on = iOn;
    iOn = NULL;
    return (on);
}


void InjectorDeviceMock::Dispose()
{
    iDevice->Dispose();
}


void InjectorDeviceMock::Execute(ICommandTokens& aTokens)
{
    iDevice->Execute(aTokens);
}

/////////////////////////////////////////////////////////////////////////////////////
/*
ServiceNotFoundException::ServiceNotFoundException()
{

}

ServiceNotFoundException::ServiceNotFoundException(const Brx& aMessage)
    :Exception(aMessage)
{
}

ServiceNotFoundException::ServiceNotFoundException(const Brx& aMessage, Exception aInnerException)
    :Exception(aMessage, aInnerException)
{
}
*/
/////////////////////////////////////////////////////////////////////////////////////


InjectorDevice::InjectorDevice(INetwork& aNetwork, const Brx& aUdn)
    :iNetwork(aNetwork)
    ,iUdn(aUdn)
    ,iDisposeHandler(new DisposeHandler())
{

}

InjectorDevice::~InjectorDevice()
{
    delete iDisposeHandler;
    for (auto it=iServices.begin(); it!=iServices.end(); it++)
    {
        Service* service = it->second;
        delete service;
    }
}

void InjectorDevice::Join(Functor aAction)
{
    DisposeLock lock(*iDisposeHandler);
    iJoiners.push_back(aAction);
}


void InjectorDevice::Unjoin(Functor aAction)
{
    DisposeLock lock(*iDisposeHandler);
    auto it = find(iJoiners.begin(), iJoiners.end(), aAction);
    ASSERT(it != iJoiners.end());
    iJoiners.erase(it);
}


Brn InjectorDevice::Udn()
{
    DisposeLock lock(*iDisposeHandler);
    return (Brn(iUdn));
}


INetwork& InjectorDevice::Network() const
{
    return iNetwork;
}

void InjectorDevice::Add(EServiceType aServiceType, Service* aService)
{
    DisposeLock lock(*iDisposeHandler);
    iServices[aServiceType] = aService;
}



TBool InjectorDevice::HasService(EServiceType aServiceType)
{
    DisposeLock lock(*iDisposeHandler);
    return (iServices.count(aServiceType)!=0);
}



void InjectorDevice::Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType, IDevice& aDevice)
{
    DisposeLock lock(*iDisposeHandler);

    if (iServices.count(aServiceType)==0)
    {
        THROW(ServiceNotFoundException);
        //throw new ServiceNotFoundException("Cannot find service of type " + typeof(T) + " on " + iUdn);
    }

    iServices[aServiceType]->Create(aCallback, &aDevice);
}



IService& InjectorDevice::GetService(const Brx& aType)
{
    if (aType == Brn("product"))
    {
        return *iServices[eProxyProduct];
    }
    else if (aType == Brn("info"))
    {
        return *iServices[eProxyInfo];
    }
    else if (aType == Brn("time"))
    {
        return *iServices[eProxyTime];
    }
    else if (aType == Brn("sender"))
    {
        return *iServices[eProxySender];
    }
    else if (aType == Brn("volume"))
    {
        return *iServices[eProxyVolume];
    }
    else if (aType == Brn("playlist"))
    {
        return *iServices[eProxyPlaylist];
    }
    else if (aType == Brn("radio"))
    {
        return *iServices[eProxyRadio];
    }
    else if (aType == Brn("receiver"))
    {
        return *iServices[eProxyReceiver];
    }
    THROW(ServiceNotFoundException);
}


TBool InjectorDevice::Wait()
{
    TBool complete = true;

    for (auto it=iServices.begin(); it!=iServices.end(); it++)
    {
        complete &= it->second->Wait();
    }

    return complete;
}


void InjectorDevice::Execute(ICommandTokens& aCommands)
{
    Brn command = aCommands.Next();
    GetService(command).Execute(aCommands);
}


void InjectorDevice::Dispose()
{
    iDisposeHandler->Dispose();

    for(TUint i=0; i<iJoiners.size(); i++)
    {
        iJoiners[i]();
    }

    for (auto it=iServices.begin(); it!=iServices.end(); it++)
    {
        Service* service = it->second;
        service->Dispose();
    }
}



