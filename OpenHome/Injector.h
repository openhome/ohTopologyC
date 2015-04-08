#ifndef HEADER_INJECTOR
#define HEADER_INJECTOR


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/CpDeviceUpnp.h>
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
    Injector(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove, const Brx& aDomain, const Brx& aType, TUint aVersion, ILog& aLog);
    ~Injector();

    void Added(/*Net::CpDeviceList& aList,*/ Net::CpDevice& aDevice);
    void Removed(/*Net::CpDeviceList& aList,*/ Net::CpDevice& aDevice);
    virtual TBool FilterOut(Net::CpDevice& aCpDevice);

protected:
    Net::CpDeviceListUpnpServiceType* iDeviceList;

private:
    FunctorGeneric<Net::CpDevice*> iAdd;
    FunctorGeneric<Net::CpDevice*> iRemove;
    //ILog& iLog;
    //std::map<Brn, IInjectorDevice*, BufferCmp> iDeviceLookup;
};

///////////////////////////////////////////////////////////////

class InjectorProduct : public Injector
{
public:
    InjectorProduct(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove, ILog& aLog);
};

///////////////////////////////////////////////////////////////

class InjectorSender : public Injector
{
public:
    InjectorSender(Net::CpStack& aCpStack, FunctorGeneric<Net::CpDevice*> aAdd, FunctorGeneric<Net::CpDevice*> aRemove, ILog& aLog);

protected:
    virtual TBool FilterOut(Net::CpDevice& aCpDevice);
};

/////////////////////////////////////////////////////////////////


class InjectorMock : public IMockable, public IDisposable, public INonCopyable
{
public:
    InjectorMock(Network& aNetwork, const Brx& aResourceRoot, ILog& aLog);
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
    //Bws<100> iResourceRoot; // FIXME: random capacity value
    ILog& iLog;
    std::map<Brn, InjectorDeviceMock*, BufferCmp> iMockDevices;
};

////////////////////////////////////////////////////////////////////

} // namespace Topology
} // namespace OpenHome

#endif
