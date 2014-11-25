#ifndef HEADER_OHTOPOLOGYC_SERVICE_RECEIVER
#define HEADER_OHTOPOLOGYC_SERVICE_RECEIVER

#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/Job.h>



namespace OpenHome
{
namespace Net
{
class CpProxyAvOpenhomeOrgReceiver1;
}
namespace Av
{

static const Brn kTransportStatePlaying("Playing");
static const Brn kTransportStateStopped("Stopped");
static const Brn kTransportStatePaused("Paused");

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
    ServiceReceiver(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

protected:
    Bws<100> iProtocolInfo; // FIXME: random capacity value
    Watchable<IInfoMetadata*>* iMetadata;
    Watchable<Brn>* iTransportState;
    IInfoMetadata* iCurrentMetadata;
    Bws<100>* iCurrentTransportState; // FIXME: random capacity value
};

////////////////////////////////////////////////////////

class ServiceReceiverNetwork : public ServiceReceiver
{
public:
    ServiceReceiverNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, Net::CpDevice& aCpDevice, ILog& aLog);
    ~ServiceReceiverNetwork();

    virtual void Dispose();
    virtual void Play();
    virtual void Play(ISenderMetadata& aMetadata);
    virtual void Stop();

protected:
    virtual Job* OnSubscribe();
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

    void BeginSetSenderCallback(Net::IAsync& aAsync);

private:
    Net::CpDevice& iCpDevice;
    JobDone* iSubscribedSource;
    Net::CpProxyAvOpenhomeOrgReceiver1* iService;
};

//////////////////////////////////////////////////////////////

class ServiceReceiverMock : public ServiceReceiver
{
public:
    ServiceReceiverMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aMetadata,
                        const Brx& aProtocolInfo, const Brx& aTransportState, const Brx& aUri, ILog& aLog);

    virtual void Execute(ICommandTokens& aValue);

    virtual void Play();
    virtual void Play(ISenderMetadata& aMetadata);
    virtual void Stop();
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

} // Av

} // OpenHome


#endif
