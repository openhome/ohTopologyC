#ifndef HEADER_DEVICE
#define HEADER_DEVICE

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>

namespace OpenHome
{

namespace Av
{


class IDevice : public IJoinable
{
public:
    virtual const Brx& Udn() = 0;
    virtual void Create(FunctorGeneric<void*> aCallback) = 0;
};

/////////////////////////////////////////////////////////////////////////////////////

class IInjectorDevice : public IJoinable, public IMockable, public IDisposable
{
public:
    virtual const Brx& Udn() = 0;
    virtual void Create(Action aCallback, IDevice& aDevice) = 0;
    //virtual TBool HasService(Type aServiceType) = 0;
    virtual TBool Wait() = 0;
};

/////////////////////////////////////////////////////////////////////////////////////


class Device : public IDevice, public IDisposable
{
public:
    Device(IInjectorDevice& aDevice);
    virtual const Brx& Udn();
    virtual void Create(Action aCallback);
    virtual void Join(Action aAction);
    virtual void Unjoin(Action aAction);
    virtual void Dispose();
    //virtual TBool HasService(Type aServiceType);
    virtual TBool Wait();

private:
    //DisposeHandler& iDisposeHandler;
    IInjectorDevice& iDevice;
};

/////////////////////////////////////////////////////////////////////////////////////

class InjectorDevice : public IInjectorDevice
{
public:
    InjectorDevice(const Brx& aUdn);
    virtual void Join(Action aAction);
    virtual void Unjoin(Action aAction);
    virtual const Brx& Udn();
    //virtual void Add(Service& aService);
    //virtual bool HasService(Type& aServiceType);
    virtual void Create(Action aCallback, IDevice& aDevice);
    virtual TBool Wait();

    // IMockable
    virtual void Execute(ICommandTokens& aTokens);

    // IDisposable
    virtual void Dispose();

private:
    //IService& GetService(const Brx& aType);

protected:
    //Dictionary<Type, Service> iServices;

private:
    Brn iUdn;
    //DisposeHandler iDisposeHandler;
    std::vector<Action> iJoiners;
};

///////////////////////////////////////////////////////////////

class InjectorDeviceAdaptor : public IInjectorDevice
{
public:
    InjectorDeviceAdaptor(IInjectorDevice& aDevice);
    virtual void Join(Action aAction);
    virtual void Unjoin(Action aAction);
    virtual const Brx& Udn();
    virtual void Create(Action aCallback, IDevice& aDevice);
    //TBool HasService(Type aServiceType);
    virtual TBool Wait();
    virtual void Execute(ICommandTokens& aTokens);
    virtual void Dispose();

private :
    IInjectorDevice& iDevice;
};

///////////////////////////////////////////////////////////////

class InjectorDeviceMock : public IMockable, public IDisposable
{
public:
    InjectorDeviceMock(IInjectorDevice& aDevice);
    IInjectorDevice* On();
    IInjectorDevice* Off();
    virtual void Dispose();
    virtual void Execute(ICommandTokens& aTokens);

private:
    IInjectorDevice& iDevice;
    InjectorDeviceAdaptor iDeviceAdaptor;
    TBool iOn;
};

///////////////////////////////////////////////////////////////


} // Av

} // OpenHome

#endif // HEADER_DEVICE
