#include<OpenHome/Device.h>
#include<OpenHome/OhTopologyC.h>
#include<OpenHome/Buffer.h>
#include<OpenHome/Service.h>
#include <algorithm>
#include <vector>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



Device::Device(IInjectorDevice& aDevice)
    :iDevice(aDevice)
    ,iDisposeHandler(new DisposeHandler())
{
}


Brn Device::Udn()
{
    //using (iDisposeHandler.Lock())
    //{
        return iDevice.Udn();
    //}

}


void Device::Create(FunctorGeneric<void*> aCallback, EServiceType aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        iDevice.Create(aCallback, aServiceType, *this);
    //}
}


void Device::Join(Functor aAction)
{
    //using (iDisposeHandler.Lock())
    //{
        iDevice.Join(aAction);
    //}
}


void Device::Unjoin(Functor aAction)
{
/*    using (iDisposeHandler.Lock())
    {
        iDevice.Unjoin(aAction);
    }
*/
    iDevice.Unjoin(aAction);
}


void Device::Dispose()
{
    //iDisposeHandler.Dispose();

    iDevice.Dispose();
}


TBool Device::HasService(EServiceType aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        return iDevice.HasService(aServiceType);
    //}
}


TBool Device::Wait()
{
    //using (iDisposeHandler.Lock())
    //{
        return iDevice.Wait();
    //}
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


void InjectorDeviceAdaptor::Create(FunctorGeneric<void*> aCallback, EServiceType aServiceType, IDevice& aDevice)
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

InjectorDeviceMock::InjectorDeviceMock(IInjectorDevice& aDevice)
    :iDevice(aDevice)
    ,iDeviceAdaptor(*(new InjectorDeviceAdaptor(iDevice)))
    ,iOn(false)
{

}


IInjectorDevice& InjectorDeviceMock::On()
{
    ASSERT(!iOn);
    iOn = true;
    return (iDeviceAdaptor);
}


IInjectorDevice& InjectorDeviceMock::Off()
{
    ASSERT(iOn);
    iOn = false;
    return (iDeviceAdaptor);
}


void InjectorDeviceMock::Dispose()
{
    iDevice.Dispose();
}


void InjectorDeviceMock::Execute(ICommandTokens& aTokens)
{
    iDevice.Execute(aTokens);
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


InjectorDevice::InjectorDevice(const Brx& aUdn)
    :iUdn(aUdn)
    ,iDisposeHandler(new DisposeHandler())
{

}

void InjectorDevice::Join(Functor aAction)
{
    //using (iDisposeHandler.Lock())
    //{
        iJoiners.push_back(aAction);
    //}
}

void InjectorDevice::Unjoin(Functor aAction)
{
/*
    using (iDisposeHandler.Lock())
    {
        iJoiners.Remove(aAction);
    }
*/
    vector<Functor>::iterator  it = find(iJoiners.begin(), iJoiners.end(), aAction);
    ASSERT(it != iJoiners.end());
    iJoiners.erase(it);
}

Brn InjectorDevice::Udn()
{
    //using (iDisposeHandler.Lock())
    //{
        return iUdn;
    //}
}


void InjectorDevice::Add(EServiceType aServiceType, Service* aService)
{
    //using (iDisposeHandler.Lock())
    //{
        iServices[aServiceType] = aService;
    //}
}



TBool InjectorDevice::HasService(EServiceType aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        return (iServices.count(aServiceType)!=0);
    //}
}


void InjectorDevice::Create(FunctorGeneric<void*> aCallback, EServiceType aServiceType, IDevice& aDevice)
{

/*
    using (iDisposeHandler.Lock())
    {
        if (!iServices.ContainsKey(typeof(T)))
        {
            throw new ServiceNotFoundException("Cannot find service of type " + typeof(T) + " on " + iUdn);
        }

        iServices[typeof(T)].Create<T>(aCallback, aDevice);
    }
*/
    if (iServices.count(aServiceType)==0)
    {
        THROW(ServiceNotFoundException);
        //throw new ServiceNotFoundException("Cannot find service of type " + typeof(T) + " on " + iUdn);
    }

    iServices[aServiceType]->Create(aCallback, aServiceType, &aDevice);
}


IService& InjectorDevice::GetService(const Brx& aType)
{
    if (aType == Brn("product"))
    {
        return *iServices[EProxyProduct];
    }
    else if (aType == Brn("info"))
    {
        return *iServices[EProxyInfo];
    }
    else if (aType == Brn("time"))
    {
        return *iServices[EProxyTime];
    }
    else if (aType == Brn("sender"))
    {
        return *iServices[EProxySender];
    }
    else if (aType == Brn("volume"))
    {
        return *iServices[EProxyVolume];
    }
    else if (aType == Brn("playlist"))
    {
        return *iServices[EProxyPlaylist];
    }
    else if (aType == Brn("radio"))
    {
        return *iServices[EProxyRadio];
    }
    else if (aType == Brn("receiver"))
    {
        return *iServices[EProxyReceiver];
    }
    //else
    //{
        THROW(ServiceNotFoundException);
    //}

    //ASSERTS();
    //return *iServices[EProxyProduct];
}



TBool InjectorDevice::Wait()
{
    TBool complete = true;

    map<EServiceType, Service*>::iterator it;
    for (it=iServices.begin(); it!=iServices.end(); it++)
    {
        complete &= it->second->Wait();
    }

/*    foreach (Service service in iServices.Values)
    {
        complete &= service.Wait();
    }
*/
    return complete;
}

// IMockable

void InjectorDevice::Execute(ICommandTokens& aCommands)
{
    //GetService(aValue.First()).Execute(aValue.Skip(1));
    Brn command = aCommands.Next();
    GetService(command).Execute(aCommands);
}

// IDisposable

void InjectorDevice::Dispose()
{
    iDisposeHandler->Dispose();

    for(TUint i=0; i<iJoiners.size(); i++)
    {
        iJoiners[i]();
    }


    map<EServiceType, Service*>::iterator it;
    for (it=iServices.begin(); it!=iServices.end(); it++)
    {
        it->second->Dispose();
    }
/*
    foreach (IService s in iServices.Values)
    {
        s.Dispose();
    }
*/
}


////////////////////////////////////////////////////////////

//ServiceNotFoundException::ServiceNotFoundException()
//    :Exception()
//{
//}

/*
ServiceNotFoundException::ServiceNotFoundException(const Brx& aMessage)
    : Exception(aMessage.Ptr())
{
}

ServiceNotFoundException::ServiceNotFoundException(const Brx& aMessage, Exception aInnerException)
    : Exception(aMessage, aInnerException)
{
}
*/

