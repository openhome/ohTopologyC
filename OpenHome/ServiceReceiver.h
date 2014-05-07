#ifndef HEADER_OHTOPOLOGYC_SERVICE_RECEIVER
#define HEADER_OHTOPOLOGYC_SERVICE_RECEIVER

#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>



namespace OpenHome
{
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
/*
    virtual Task Play() = 0;
    virtual Task Play(ISenderMetadata& aMetadata) = 0;
    virtual Task Stop() = 0;
*/
};

//////////////////////////////////////////////////////

class ServiceReceiver : public Service
{
public:
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice* aDevice);

    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<Brn>& TransportState();

    virtual const Brx& ProtocolInfo();
/*
    virtual Task Play() = 0;
    virtual Task Play(ISenderMetadata& aMetadata) = 0;
    virtual Task Stop() = 0;
*/
protected:
    ServiceReceiver(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

protected:
    Bws<100> iProtocolInfo;
    Watchable<IInfoMetadata*>* iMetadata;
    Watchable<Brn>* iTransportState;
    IInfoMetadata* iCurrentMetadata;
};

////////////////////////////////////////////////////////
/*
class ServiceReceiverNetwork : ServiceReceiver
{
public:
    ServiceReceiverNetwork(INetwork aNetwork, IInjectorDevice aDevice, CpDevice aCpDevice, ILog aLog)
    virtual void Dispose();
    virtual Task Play();
    virtual Task Play(ISenderMetadata aMetadata);
    virtual Task Stop();

protected:
    virtual Task OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleMetadataChanged();
    void HandleTransportStateChanged();
    void HandleInitialEvent();

private:
    CpDevice iCpDevice;
    TaskCompletionSource<TBool> iSubscribedSource;
    CpProxyAvOpenhomeOrgReceiver1 iService;
}
*/
//////////////////////////////////////////////////////////////

class ServiceReceiverMock : public ServiceReceiver
{
public:
    ServiceReceiverMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aMetadata,
                        const Brx& aProtocolInfo, const Brx& aTransportState, const Brx& aUri, ILog& aLog);

    virtual void Execute(ICommandTokens& aValue);
/*
    virtual Task Play();
    virtual Task Play(ISenderMetadata& aMetadata);
    virtual Task Stop();
*/
};

//////////////////////////////////////////////////////////////

class ProxyReceiver : /*public Proxy<ServiceReceiver*>,*/ public IProxyReceiver, public INonCopyable
{
public:
    ProxyReceiver(ServiceReceiver& aService, IDevice& aDevice);

    virtual const Brx& ProtocolInfo();
    virtual IWatchable<IInfoMetadata*>& Metadata();
    virtual IWatchable<Brn>& TransportState();
/*
    virtual Task Play();
    virtual Task Play(ISenderMetadata aMetadata);
    virtual Task Stop();
*/
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
