#ifndef HEADER_INJECTOR
#define HEADER_INJECTOR


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Mockable.h>
#include <map>


EXCEPTION(NotImplementedException);


namespace OpenHome
{

namespace Av
{


class Network;


/*
class Injector : public IDisposable
{
public :
    void Refresh();

    // IDisposable
    virtual void Dispose();

protected :
    Injector(Network aNetwork, const Brx& aDomain, const Brx& aType, TUint aVersion, ILog aLog);
    void Added(CpDeviceList aList, CpDevice aDevice);
    void Removed(CpDeviceList aList, CpDevice aDevice);
    virtual IInjectorDevice Create(INetwork& aNetwork, CpDevice aDevice);
    virtual TBool FilterOut(CpDevice aCpDevice);

protected :
    DisposeHandler iDisposeHandler;
    CpDeviceListUpnpServiceType iDeviceList;

private:
    INetwork& iNetwork;
    ILog iLog;
    Dictionary<const Brx&, IInjectorDevice> iDeviceLookup;
};

///////////////////////////////////////////////////////////////

class InjectorProduct : public Injector
{
    public InjectorProduct(Network aNetwork, ILog aLog);
};

///////////////////////////////////////////////////////////////

public class InjectorSender : public Injector
{
    public InjectorSender(Network aNetwork, ILog aLog);

protected:
    virtual TBool FilterOut(CpDevice aCpDevice);
};

/////////////////////////////////////////////////////////////////
*/

class InjectorMock : public IMockable, public IDisposable, public INonCopyable
{
public:
    InjectorMock(Network& aNetwork, const Brx& aResourceRoot, ILog& aLog);
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
