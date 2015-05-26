#ifndef HEADER_DEVICE
#define HEADER_DEVICE

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/DisposeHandler.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Topology
{

class IDevice : public IJoinable
{
public:
    virtual Brn Udn() = 0;
    virtual void Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

class IInjectorDevice : public IJoinable, public IMockable, public IDisposable
{
public:
    virtual Brn Udn() = 0;
    virtual INetwork& Network() const = 0;
    virtual void Create(FunctorGeneric<IProxy*>, EServiceType aServiceType, IDevice& aDevice) = 0;
    virtual TBool HasService(EServiceType aServiceType) = 0;
    virtual TBool Wait() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////


class Device : public IDevice, public IDisposable, public INonCopyable
{
public:
    Device(IInjectorDevice* aDevice);
    ~Device();

    virtual Brn Udn();

    // IDevice
    virtual void Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType);

    // IJoinable
    virtual void Join(Functor aAction);
    virtual void Unjoin(Functor aAction);


    virtual void Dispose();
    virtual TBool HasService(EServiceType aServiceType);
    virtual TBool Wait();

private:
    IInjectorDevice* iDevice;
    DisposeHandler* iDisposeHandler;
};

/////////////////////////////////////////////////////////////////////////////////////

class InjectorDevice : public IInjectorDevice
{
public:
    InjectorDevice(INetwork& aNetwork, const Brx& aUdn);
    ~InjectorDevice();

    virtual void Join(Functor aAction);
    virtual void Unjoin(Functor aAction);
    virtual Brn Udn();
    virtual INetwork& Network() const;
    virtual void Add(EServiceType aServiceType, Service* aService);
    virtual bool HasService(EServiceType aServiceType);
    virtual void Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType, IDevice& aDevice);
    virtual TBool Wait();

    // IMockable
    virtual void Execute(ICommandTokens& aTokens);

    // IDisposable
    virtual void Dispose();

private:
    IService& GetService(const Brx& aType);

protected:
    std::map<EServiceType, Service*> iServices;

private:
    INetwork& iNetwork;
    Bws<100> iUdn;
    DisposeHandler* iDisposeHandler;
    std::vector<Functor> iJoiners;
};

///////////////////////////////////////////////////////////////
// only used for mocking
class InjectorDeviceAdaptor : public IInjectorDevice, public INonCopyable
{
public:
    InjectorDeviceAdaptor(IInjectorDevice& aDevice);
    virtual void Join(Functor aAction);
    virtual void Unjoin(Functor aAction);
    virtual Brn Udn();
    virtual INetwork& Network() const;
    virtual void Create(FunctorGeneric<IProxy*> aCallback, EServiceType aServiceType, IDevice& aDevice);
    virtual TBool HasService(EServiceType aServiceType);
    virtual TBool Wait();
    virtual void Execute(ICommandTokens& aTokens);
    virtual void Dispose();

private :
    IInjectorDevice& iDevice;
};

///////////////////////////////////////////////////////////////

class InjectorDeviceMock : public IMockable, public IDisposable, public INonCopyable
{
public:
    InjectorDeviceMock(IInjectorDevice* aDevice);
    ~InjectorDeviceMock();

    IInjectorDevice* On();
    IInjectorDevice* Off();
    virtual void Dispose();
    virtual void Execute(ICommandTokens& aTokens);

private:
    IInjectorDevice* iDevice;
    IInjectorDevice* iOn;
};

///////////////////////////////////////////////////////////////
/*
class ServiceNotFoundException : public Exception
{
public:
    //ServiceNotFoundException();
    ServiceNotFoundException(const Brx& aMessage);
    ServiceNotFoundException(const Brx& aMessage, Exception aInnerException);
};

*/


} // Topology
} // OpenHome

#endif // HEADER_DEVICE
