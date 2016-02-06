#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>

#include <vector>
#include <memory>

namespace OpenHome
{

namespace Net
{
    class CpProxyAvOpenhomeOrgTime1;
}

namespace Topology
{

class ServiceTime : public Service
{
protected:
    ServiceTime(IInjectorDevice& aDevice);
    ~ServiceTime();

public:
    void Dispose();
    IProxy* OnCreate(IDevice& aDevice);
    IWatchable<TUint>& Duration();

public:
    IWatchable<TUint>& Seconds();

protected :
    Watchable<TUint>* iDuration;
    Watchable<TUint>* iSeconds;
};

/////////////////////////////////////////////////

class ServiceTimeNetwork : public ServiceTime
{
public:
    ServiceTimeNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgTime1* aService);
    ~ServiceTimeNetwork();
    void Dispose();

protected:
    TBool OnSubscribe();
    void OnCancelSubscribe();
    void OnUnsubscribe();

private:
    void HandleDurationChanged();
    void HandleSecondsChanged();
    void HandleInitialEvent();

    void DurationChangedCallback1(void*);
    void DurationChangedCallback2(void*);
    void SecondsChangedCallback1(void*);
    void SecondsChangedCallback2(void*);

private:
    Net::CpProxyAvOpenhomeOrgTime1* iService;
    TBool iSubscribed;
    //TBool iSubscribedSource;
};

class ServiceTimeMock : public ServiceTime
{
public:
    ServiceTimeMock(IInjectorDevice& aDevice, TUint aSeconds, TUint aDuration);
    ~ServiceTimeMock();
public:
    void Execute(ICommandTokens& aValue) override;

};


class IProxyTime : public IProxy
{
public:
    virtual IWatchable<TUint>& Duration() = 0; // { get; }
    virtual IWatchable<TUint>& Seconds() = 0; // { get; }
};

//////////////////////////////////////

class ProxyTime : public IProxyTime, public INonCopyable
{
public:
    ProxyTime(ServiceTime& aService, IDevice& aDevice);
    IWatchable<TUint>& Duration();
    IWatchable<TUint>& Seconds();

    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

protected:
    ServiceTime& iService;

private:
    IDevice& iDevice;
};


} // Topology
} // OpeNHome
