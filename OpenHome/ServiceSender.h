#ifndef HEADER_OHTOPOLOGYC_SERVICE_SENDER
#define HEADER_OHTOPOLOGYC_SERVICE_SENDER

#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Job.h>


namespace OpenHome
{
namespace Net
{
class CpProxyAvOpenhomeOrgSender1;
}
namespace Av
{

////////////////////////////////////////////////////////////////////

class IProxySender : public IProxy
{
public:
    virtual IWatchable<TBool>& Audio() = 0;
    virtual IWatchable<ISenderMetadata*>& Metadata() = 0;
    virtual IWatchable<Brn>& Status() = 0;

    virtual const Brx& Attributes() = 0;
    virtual const Brx& PresentationUrl() = 0;
    virtual ~IProxySender() {};
};

/////////////////////////////////

class ServiceSender : public Service
{
public:
    ~ServiceSender();
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice& aDevice);
    virtual IWatchable<TBool>& Audio();
    virtual IWatchable<ISenderMetadata*>& Metadata();
    virtual IWatchable<Brn>& Status();
    virtual const Brx& Attributes();
    virtual const Brx& PresentationUrl();

protected:
    ServiceSender(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog);

protected:
    Bws<100> iAttributes;
    Bws<100> iPresentationUrl;
    Watchable<TBool>* iAudio;
    Watchable<ISenderMetadata*>* iMetadata;
    Watchable<Brn>* iStatus;
    ISenderMetadata* iCurrentMetadata;
    TBool iCurrentAudio;
    Bws<100>* iCurrentStatus; //FIXME:random capacity
};

/////////////////////////////////////////////////////////

class ServiceSenderNetwork : public ServiceSender
{
public:
    ServiceSenderNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, Net::CpDevice& aCpDevice, ILog& aLog);

    virtual void Dispose();

protected:
    virtual void OnSubscribe(ServiceCreateData& aServiceCreateData);
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void HandleInitialEvent();
    void HandleAudioChanged();
    void HandleMetadataChanged();
    void HandleStatusChanged();

    void AudioChangedCallback1(void* aAudio);
    void AudioChangedCallback2(void* aAudio);
    void MetadataChangedCallback1(void* aMetadata);
    void MetadataChangedCallback2(void* aMetadata);
    void StatusChangedCallback1(void* aStatus);
    void StatusChangedCallback2(void* aStatus);

private:
    Net::CpDevice& iCpDevice;
    ServiceCreateData* iSubscribedSource;
    Net::CpProxyAvOpenhomeOrgSender1* iService;
};


////////////////////////////////////////////////////////////////

class ServiceSenderMock : public ServiceSender
{
public:
    ServiceSenderMock(INetwork& aNetwork, IInjectorDevice& aDevice,  const Brx& aAttributes, const Brx& aPresentationUrl,
                      TBool aAudio, ISenderMetadata* aMetadata, const Brx& aStatus, ILog& aLog);

    virtual void Execute(ICommandTokens& aValue);
};

///////////////////////////////////////////////////////////////

class ProxySender : /*public Proxy<ServiceSender*>,*/ public IProxySender, public INonCopyable
{
public:
    ProxySender(ServiceSender& aService, IDevice& aDevice);

    virtual void Dispose();
    virtual IDevice& Device();

    virtual const Brx& Attributes();
    virtual const Brx& PresentationUrl();
    virtual IWatchable<TBool>& Audio();
    virtual IWatchable<ISenderMetadata*>& Metadata();
    virtual IWatchable<Brn>& Status();

private:
    ServiceSender& iService;
    IDevice& iDevice;
};



} // Av
} // OpenHome


#endif // HEADER_OHTOPOLOGYC_SERVICE_SENDER
