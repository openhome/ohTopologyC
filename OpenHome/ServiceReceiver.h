#pragma once

#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>
//#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>

namespace OpenHome
{
namespace Net
{
    class CpProxyAvOpenhomeOrgReceiver1;
}
namespace Topology
{

extern const Brn kReceiverTransportStatePlaying;
extern const Brn kReceiverTransportStateStopped;
extern const Brn kReceiverTransportStatePaused;

class IProxyReceiver : public IProxy
{
public:
    virtual const Brx& ProtocolInfo() = 0;
    virtual IWatchable<IInfoMetadata*>& Metadata() = 0;
    virtual IWatchable<Brn>& TransportState() = 0;

    virtual void Play() = 0;
    virtual void Play(ISenderMetadata& aMetadata) = 0;
    virtual void Stop() = 0;

};

//////////////////////////////////////////////////////

class ServiceReceiver : public Service
{
public:
    ~ServiceReceiver();
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice& aDevice);

    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<Brn>& TransportState();

    virtual const Brx& ProtocolInfo();

    virtual void Play() = 0;
    virtual void Play(ISenderMetadata& aMetadata) = 0;
    virtual void Stop() = 0;

protected:
    ServiceReceiver(IInjectorDevice& aDevice);

protected:
    Bws<100> iProtocolInfo; // FIXME: random capacity value
    IInfoMetadata* iCurrentMetadata;
    Bws<100>* iCurrentTransportState; // FIXME: random capacity value
    Watchable<IInfoMetadata*>* iMetadata;
    Watchable<Brn>* iTransportState;
};

////////////////////////////////////////////////////////

class ServiceReceiverNetwork : public ServiceReceiver
{
public:
    ServiceReceiverNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgReceiver1* aService);
    ~ServiceReceiverNetwork();

    virtual void Dispose();
    virtual void Play();
    virtual void Play(ISenderMetadata& aMetadata);
    virtual void Stop();

protected:
    virtual TBool OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleMetadataChanged();
    void HandleTransportStateChanged();
    void HandleInitialEvent();
    void MetadataChangedCallback1(void* aMetadata);
    void MetadataChangedCallback2(void* aMetadata);
    void TransportChangedCallback1(void* aTransportState);
    void TransportChangedCallback2(void* aTransportState);

    void BeginSetSenderCallback1(Net::IAsync& aAsync);
    void BeginSetSenderCallback2(void*);

private:
    Net::CpProxyAvOpenhomeOrgReceiver1* iService;
    TBool iSubscribed;
};

//////////////////////////////////////////////////////////////

class ServiceReceiverMock : public ServiceReceiver
{
public:
    ServiceReceiverMock(IInjectorDevice& aDevice, const Brx& aMetadata,
                        const Brx& aProtocolInfo, const Brx& aTransportState, const Brx& aUri);

    virtual void Execute(ICommandTokens& aValue);

    virtual void Play();
    virtual void Play(ISenderMetadata& aMetadata);
    virtual void Stop();

private:
    void PlayCallback(void*);
    void PlayMetaCallback(void* aMetadata);
    void StopCallback(void*);
};

//////////////////////////////////////////////////////////////

class ProxyReceiver : public IProxyReceiver, public INonCopyable
{
public:
    ProxyReceiver(ServiceReceiver& aService, IDevice& aDevice);

    virtual const Brx& ProtocolInfo();
    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<Brn>& TransportState();

    virtual void Play();
    virtual void Play(ISenderMetadata& aMetadata);
    virtual void Stop();

    // IProxyReceiver
    virtual IDevice& Device();
    virtual void Dispose();

private:
    ServiceReceiver& iService;
    IDevice& iDevice;
};

} // Topology
} // OpenHome
