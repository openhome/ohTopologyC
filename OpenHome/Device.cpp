#include<OpenHome/Device.h>
#include<OpenHome/Buffer.h>
#include <vector>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Av;



Device::Device(IInjectorDevice& aDevice)
    :iDevice(aDevice)
{
    //iDisposeHandler = new DisposeHandler();
}


const Brx& Device::Udn()
{

    //using (iDisposeHandler.Lock())
    //{
        return iDevice.Udn();
    //}

}


void Device::Create(Action aCallback)
{

    //using (iDisposeHandler.Lock())
    //{
        iDevice.Create(aCallback, *this);
    //}

}


void Device::Join(Action aAction)
{
    //using (iDisposeHandler.Lock())
    //{
        iDevice.Join(aAction);
    //}

}


void Device::Unjoin(Action aAction)
{
/*    using (iDisposeHandler.Lock())
    {
        iDevice.Unjoin(aAction);
    }
*/
}


void Device::Dispose()
{
    //iDisposeHandler.Dispose();

    iDevice.Dispose();
}

/*
TBool Device::HasService(Type aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        return iDevice.HasService(aServiceType);
    //}
}
*/

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


void InjectorDeviceAdaptor::Join(Action aAction)
{
    iDevice.Join(aAction);
}


void InjectorDeviceAdaptor::Unjoin(Action aAction)
{
    iDevice.Unjoin(aAction);
}


const Brx& InjectorDeviceAdaptor::Udn()
{
    return iDevice.Udn();
}


void InjectorDeviceAdaptor::Create(Action aCallback, IDevice& aDevice)
{
    iDevice.Create(aCallback, aDevice);
}

/*
TBool InjectorDeviceAdaptor::HasService(Type aServiceType)
{
    return iDevice.HasService(aServiceType);
}
*/

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
    ,iDeviceAdaptor(iDevice)
    ,iOn(false)
{

}


IInjectorDevice* InjectorDeviceMock::On()
{
    ASSERT(!iOn);
    iOn = true;
    return (&iDeviceAdaptor);
}


IInjectorDevice* InjectorDeviceMock::Off()
{
    ASSERT(iOn);
    iOn = false;
    return (&iDeviceAdaptor);
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
    //,iDisposeHandler(new DisposeHandler())
    //,iServices = new Dictionary<Type, Service>();
{

}

void InjectorDevice::Join(Action aAction)
{
    //using (iDisposeHandler.Lock())
    //{
        iJoiners.push_back(aAction);
    //}
}

void InjectorDevice::Unjoin(Action aAction)
{
    //using (iDisposeHandler.Lock())
    //{
        //std::vector<Action>::iterator joiner = find(iJoiners.begin(), iJoiners.end(), aAction);
        //ASSERT(joiner != iJoiners.end());
        //iJoiners.erase(joiner);
    //}
}

const Brx& InjectorDevice::Udn()
{
    //using (iDisposeHandler.Lock())
    //{
        return iUdn;
    //}
}

/*
template <class T>
void InjectorDevice::Add<T>(Service aService)
{
    //using (iDisposeHandler.Lock())
    //{
        iServices.Add(typeof(T), aService);
    //}
}
*/

/*
TBool InjectorDevice::HasService(Type aServiceType)
{
    //using (iDisposeHandler.Lock())
    //{
        return iServices.ContainsKey(aServiceType);
    //}
}
*/

void InjectorDevice::Create(Action aCallback, IDevice& aDevice)
{
    //using (iDisposeHandler.Lock())
    //{
/*
        if (!iServices.ContainsKey(typeof(T)))
        {
            throw new ServiceNotFoundException("Cannot find service of type " + typeof(T) + " on " + iUdn);
        }

        iServices[typeof(T)].Create<T>(aCallback, aDevice);
*/
    //}
}

/*
IService InjectorDevice::GetService(const Brx& aType)
{
    if (aType == "product")
    {
        return iServices[typeof(IProxyProduct)];
    }
    else if (aType == "info")
    {
        return iServices[typeof(IProxyInfo)];
    }
    else if (aType == "time")
    {
        return iServices[typeof(IProxyTime)];
    }
    else if (aType == "sender")
    {
        return iServices[typeof(IProxySender)];
    }
    else if (aType == "volume")
    {
        return iServices[typeof(IProxyVolume)];
    }
    else if (aType == "playlist")
    {
        return iServices[typeof(IProxyPlaylist)];
    }
    else if (aType == "radio")
    {
        return iServices[typeof(IProxyRadio)];
    }
    else if (aType == "receiver")
    {
        return iServices[typeof(IProxyReceiver)];
    }
    else
    {
        throw new ServiceNotFoundException();
    }
}
*/


TBool InjectorDevice::Wait()
{

    TBool complete = true;

/*    foreach (Service service in iServices.Values)
    {
        complete &= service.Wait();
    }
*/
    return complete;
}

// IMockable

void InjectorDevice::Execute(ICommandTokens& aTokens)
{
    //GetService(aValue.First()).Execute(aValue.Skip(1));
    //GetService(aTokens.Next()).Execute(aTokens.Next(1));
}

// IDisposable

void InjectorDevice::Dispose()
{
/*
    iDisposeHandler.Dispose();

    foreach (Action action in iJoiners)
    {
        action();
    }

    foreach (IService s in iServices.Values)
    {
        s.Dispose();
    }
*/}


