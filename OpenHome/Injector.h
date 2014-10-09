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

namespace Av
{


class Network;



class Injector : public IDisposable, public INonCopyable
{
public:
    void Refresh();

    // IDisposable
    virtual void Dispose();

protected:
    Injector(Network& aNetwork, Net::CpStack& aCpStack, const Brx& aDomain, const Brx& aType, TUint aVersion, ILog& aLog);
    void Added(/*Net::CpDeviceList& aList,*/ Net::CpDevice& aDevice);
    void Removed(/*Net::CpDeviceList& aList,*/ Net::CpDevice& aDevice);
    virtual IInjectorDevice* Create(INetwork& aNetwork, Net::CpDevice& aDevice);
    virtual TBool FilterOut(Net::CpDevice& aCpDevice);

protected:
    DisposeHandler* iDisposeHandler;
    Net::CpDeviceListUpnpServiceType* iDeviceList;

private:
    Network& iNetwork;
    ILog& iLog;
    std::map<Brn, IInjectorDevice*, BufferCmp> iDeviceLookup;
};

///////////////////////////////////////////////////////////////

class InjectorProduct : public Injector
{
public:
    InjectorProduct(Network& aNetwork, Net::CpStack& aCpStack, ILog& aLog);
};

///////////////////////////////////////////////////////////////

class InjectorSender : public Injector
{
public:
    InjectorSender(Network& aNetwork, Net::CpStack& aCpStack, ILog& aLog);

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

} // namespace Av
} // namespace OpenHome

#endif
