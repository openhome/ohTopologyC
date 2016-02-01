#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Network.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Device.h>
#include <OpenHome/DeviceFactory.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Net/Core/CpDeviceDv.h>
#include <map>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


Injector::Injector( CpStack& aCpStack,
                    DvDevice& aDvDevice,
                    FunctorGeneric<CpDevice*> aAdd,
                    FunctorGeneric<CpDevice*> aRemove,
                    const Brx& aDomain, const Brx& aType, TUint aVersion, ILog& /*aLog*/)

    :iCpDevice(CpDeviceDv::New(aCpStack, aDvDevice))
    ,iAdd(aAdd)
    ,iRemove(aRemove)
{
    iAdd(iCpDevice);
    Construct(aCpStack, aDomain, aType, aVersion);
}

Injector::Injector( CpStack& aCpStack,
                    FunctorGeneric<CpDevice*> aAdd,
                    FunctorGeneric<CpDevice*> aRemove,
                    const Brx& aDomain, const Brx& aType, TUint aVersion, ILog& /*aLog*/)

    :iCpDevice(nullptr)
    ,iAdd(aAdd)
    ,iRemove(aRemove)
{
    Construct(aCpStack, aDomain, aType, aVersion);
}

void Injector::Construct(CpStack& aCpStack, const Brx& aDomain, const Brx& aType, TUint aVersion)
{
    FunctorCpDevice fAdded = Net::MakeFunctorCpDevice(*this, &Injector::Added);
    FunctorCpDevice fRemoved = Net::MakeFunctorCpDevice(*this, &Injector::Removed);
    iDeviceList = new CpDeviceListUpnpServiceType(aCpStack, aDomain, aType, aVersion, fAdded, fRemoved);
}


Injector::~Injector()
{
    delete iDeviceList;
}


void Injector::Added(CpDevice& aDevice)
{
    LOG(kApplication7, ">Injector::Added\n");
    if (!FilterOut(aDevice))
    {
        iAdd(&aDevice);
    }
    LOG(kApplication7, "<Injector::Added\n");
}


void Injector::Removed(CpDevice& aDevice)
{
    if (!FilterOut(aDevice))
    {
        iRemove(&aDevice);
    }
}


TBool Injector::FilterOut(CpDevice& aDevice)
{
    if (iCpDevice!=nullptr)
    {
        if(iCpDevice->Udn() == aDevice.Udn())
        {
            return true;
        }
    }
    return false;
}


void Injector::Refresh()
{
    iDeviceList->Refresh();
}


void Injector::Dispose()
{
    delete iDeviceList;
    iDeviceList = nullptr;
}

/////////////////////////////////////////////////////////////////

InjectorProduct::InjectorProduct(CpStack& aCpStack, FunctorGeneric<CpDevice*> aAdd, FunctorGeneric<CpDevice*> aRemove, ILog& aLog)
    : Injector(aCpStack, aAdd, aRemove, Brn("av.openhome.org"), Brn("Product"), 1, aLog)
{
}


InjectorProduct::InjectorProduct(CpStack& aCpStack, DvDevice& aDvDevice, FunctorGeneric<CpDevice*> aAdd, FunctorGeneric<CpDevice*> aRemove, ILog& aLog)
    : Injector(aCpStack, aDvDevice, aAdd, aRemove, Brn("av.openhome.org"), Brn("Product"), 1, aLog)
{
}

/////////////////////////////////////////////////////////////////
/*
InjectorSender::InjectorSender(CpStack& aCpStack, FunctorGeneric<CpDevice*> aAdd, FunctorGeneric<CpDevice*> aRemove, ILog& aLog)
    : Injector(aCpStack, aAdd, aRemove, Brn("av.openhome.org"), Brn("Sender"), 1, aLog)
{
}


TBool InjectorSender::FilterOut(CpDevice& aCpDevice)
{
    Brh value;
    return aCpDevice.GetAttribute("Upnp.Service.av-openhome-org.Product", value);
}
*/

/////////////////////////////////////////////////////////////////

InjectorMock::InjectorMock(Network& aNetwork, const Brx& /*aResourceRoot*/, ILog& aLog)
    :iNetwork(aNetwork)
    //,iResourceRoot(aResourceRoot)
    ,iLog(aLog)
{
}


InjectorMock::~InjectorMock()
{
    for(auto it=iMockDevices.begin();it!=iMockDevices.end();it++)
    {
        delete it->second;
    }
}


void InjectorMock::Dispose()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &InjectorMock::DisposeCallback);
    iNetwork.Execute(f, NULL);
}


void InjectorMock::DisposeCallback(void*)
{
    for(auto it=iMockDevices.begin();it!=iMockDevices.end();it++)
    {
        it->second->Dispose();
    }
}


void InjectorMock::Execute(ICommandTokens& aTokens)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &InjectorMock::ExecuteCallback);
    iNetwork.Execute(f, &aTokens);
}


void InjectorMock::ExecuteCallback(void* aObj)
{
    ICommandTokens& val = *((ICommandTokens*)aObj);
    ASSERT(val.Count()>0);

    Brn command(val.Next());


    if (Ascii::CaseInsensitiveEquals(command, Brn("small")))
    {
        //CreateAndAdd(DeviceFactory.CreateDsm(iNetwork, "4c494e4e-0026-0f99-1112-ef000004013f", "Sitting Room", "Klimax DSM", "Info Time Volume Sender", iLog));
        //CreateAndAdd(DeviceFactory.CreateMediaServer(iNetwork, "4c494e4e-0026-0f99-0000-000000000000", iResourceRoot, iLog));
        return;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("medium")))
    {
        CreateAndAdd(DeviceFactory::CreateDs(iNetwork, Brn("4c494e4e-0026-0f99-1111-ef000004013f"), Brn("Kitchen"), Brn("Sneaky Music DS"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDsm(iNetwork, Brn("4c494e4e-0026-0f99-1112-ef000004013f"), Brn("Sitting Room"), Brn("Klimax DSM"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDsm(iNetwork, Brn("4c494e4e-0026-0f99-1113-ef000004013f"), Brn("Bedroom"), Brn("Kiko DSM"), Brn("Info Time Volume Sender"), iLog));
        CreateAndAdd(DeviceFactory::CreateDs(iNetwork, Brn("4c494e4e-0026-0f99-1114-ef000004013f"), Brn("Dining Room"), Brn("Majik DS"), Brn("Info Time Volume Sender"), iLog));
        //CreateAndAdd(DeviceFactory.CreateMediaServer(iNetwork, "4c494e4e-0026-0f99-0000-000000000000", iResourceRoot, iLog));
        return;
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("large")))
    {
        ASSERTS();
        THROW(NotImplementedException);
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("create")))
    {
        ASSERT(val.Count()>1);

        Brn type(val.Next());
        Brn udn(val.Next());

        if (type == Brn("ds"))
        {
            Create(DeviceFactory::CreateDs(iNetwork, udn, iLog));
            return;
        }
        else if (type == Brn("dsm"))
        {
            Create(DeviceFactory::CreateDsm(iNetwork, udn, iLog));
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("add")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            auto deviceOn = device->On();
            iNetwork.Add(deviceOn);
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("remove")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            iNetwork.Remove(device->Off());
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("destroy")))
    {
        ASSERT(val.Count()>0);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            iNetwork.Remove(device->Off());
            device->Dispose();
            return;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command,Brn("update")))
    {
        ASSERT(val.Count()>1);
        Brn udn(val.Next());

        if (iMockDevices.count(udn) > 0)
        {
            InjectorDeviceMock* device = iMockDevices[udn];
            device->Execute(val);
            return;
        }
    }

    ASSERTS();
}


InjectorDeviceMock* InjectorMock::Create(IInjectorDevice* aDevice)
{
    InjectorDeviceMock* device = new InjectorDeviceMock(aDevice);
    iMockDevices[Brn(aDevice->Udn())] = device;
    return(device);
}


void InjectorMock::CreateAndAdd(IInjectorDevice* aDevice)
{
    InjectorDeviceMock* device = Create(aDevice);
    IInjectorDevice* dev = device->On();
    iNetwork.Add(dev);
}

/////////////////////////////////////////////////////////////////






