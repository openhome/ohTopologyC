#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/CpDeviceUpnp.h>
#include <OpenHome/Net/Core/DvDevice.h>
#include <OpenHome/Net/Private/CpiStack.h>

#include <map>

namespace OpenHome
{
namespace Topology
{

class Network;

class Injector : public IDisposable, public INonCopyable
{
public:
    void Refresh();

    // IDisposable
    virtual void Dispose();

protected:
    Injector(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove, const Brx& aDomain, const Brx& aType, TUint aVersion);
    Injector(Net::CpStack& aCpStack, Net::DvDevice& aDvDevice, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove, const Brx& aDomain, const Brx& aType, TUint aVersion);
    ~Injector();

    void Added(Net::CpDevice& aDevice);
    void Removed(Net::CpDevice& aDevice);
    virtual TBool FilterOut(Net::CpDevice& aCpDevice);

private:
    void Construct(Net::CpStack& aCpStack, const Brx& aDomain, const Brx& aType, TUint aVersion);


protected:
    Net::CpDeviceListUpnpServiceType* iDeviceList;

private:
    Net::CpDevice* iCpDevice;
    FunctorGeneric<Net::CpDevice*> iAdd;
    FunctorGeneric<Net::CpDevice*> iRemove;
};

///////////////////////////////////////////////////////////////

class InjectorProduct : public Injector
{
public:
    InjectorProduct(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove);
    InjectorProduct(Net::CpStack& aCpStack, Net::DvDevice& aDvDevice, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove);
};

///////////////////////////////////////////////////////////////

class InjectorSender : public Injector
{
public:
    InjectorSender(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove);

protected:
    virtual TBool FilterOut(Net::CpDevice& aCpDevice);
};

/////////////////////////////////////////////////////////////////


class InjectorMock : public IMockable, public IDisposable, public INonCopyable
{
public:
    InjectorMock(Network& aNetwork, const Brx& aResourceRoot);
    ~InjectorMock();
    virtual void Dispose();
    virtual void Execute(ICommandTokens& aTokens);

private:
    void DisposeCallback(void*);
    void ExecuteCallback(void* aObj);
    InjectorDeviceMock* Create(IInjectorDevice* aDevice);
    void CreateAndAdd(IInjectorDevice* aDevice);

private:
    Network& iNetwork;
    std::map<Brn, InjectorDeviceMock*, BufferCmp> iMockDevices;
};

////////////////////////////////////////////////////////////////////

} // namespace Topology
} // namespace OpenHome
