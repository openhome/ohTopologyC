#ifndef HEADER_OHTOPOLOGYC_SERVICE_SENDER
#define HEADER_OHTOPOLOGYC_SERVICE_SENDER

#include <OpenHome/Buffer.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Service.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>


namespace OpenHome
{
namespace Av
{


class ISenderMetadata
{
public:
    virtual const Brx& Uri() = 0;
    virtual const Brx& ArtworkUri() = 0;
    virtual ~ISenderMetadata() {};
};

/////////////////////////////////

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

class SenderMetadata : public ISenderMetadata
{
public:
    SenderMetadata(const Brx& aMetadata);

    static SenderMetadata* Empty();

    virtual const Brx& Name();
    virtual const Brx& Uri();
    virtual const Brx& ArtworkUri();
    virtual const Brx& ToString();

private:
    SenderMetadata();

private:
    Bws<100> iName;
    Bws<100> iUri;
    Bws<100> iArtworkUri;
    Bws<1000> iMetadata;
    static SenderMetadata* iEmpty;
};

////////////////////////////////////////////////////////////////////

class ServiceSender : public Service
{
public:
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice* aDevice);
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
};

/////////////////////////////////////////////////////////

/*
    class ServiceSenderNetwork : ServiceSender
    {
        public ServiceSenderNetwork(INetwork aNetwork, IInjectorDevice aDevice, CpDevice aCpDevice, ILog aLog)
            : base(aNetwork, aDevice, aLog)
        {
            iCpDevice = aCpDevice;
            iCpDevice.AddRef();

            iService = new CpProxyAvOpenhomeOrgSender1(aCpDevice);

            iService.SetPropertyAudioChanged(HandleAudioChanged);
            iService.SetPropertyMetadataChanged(HandleMetadataChanged);
            iService.SetPropertyStatusChanged(HandleStatusChanged);

            iService.SetPropertyInitialEvent(HandleInitialEvent);
        }

        public override void Dispose()
        {
            base.Dispose();

            iService.Dispose();
            iService = null;

            iCpDevice.RemoveRef();
        }

        protected override Task OnSubscribe()
        {
            Do.Assert(iSubscribedSource == null);

            iSubscribedSource = new TaskCompletionSource<TBool>();

            iService.Subscribe();

            return iSubscribedSource.Task.ContinueWith((t) => { });
        }

        protected override void OnCancelSubscribe()
        {
            if (iSubscribedSource != null)
            {
                iSubscribedSource.TrySetCanceled();
            }
        }

        private void HandleInitialEvent()
        {
            iAttributes = iService.PropertyAttributes();
            iPresentationUrl = iService.PropertyPresentationUrl();

            if (!iSubscribedSource.Task.IsCanceled)
            {
                iSubscribedSource.SetResult(true);
            }
        }

        protected override void OnUnsubscribe()
        {
            if (iService != null)
            {
                iService.Unsubscribe();
            }

            iSubscribedSource = null;
        }

        private void HandleAudioChanged()
        {
            TBool audio = iService.PropertyAudio();
            Network.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iAudio.Update(audio);
                });
            });
        }

        private void HandleMetadataChanged()
        {
            string metadata = iService.PropertyMetadata();
            Network.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iMetadata.Update(new SenderMetadata(metadata));
                });
            });
        }

        private void HandleStatusChanged()
        {
            string status = iService.PropertyStatus();
            Network.Schedule(() =>
            {
                iDisposeHandler.WhenNotDisposed(() =>
                {
                    iStatus.Update(status);
                });
            });
        }

        private readonly CpDevice iCpDevice;
        private TaskCompletionSource<TBool> iSubscribedSource;
        private CpProxyAvOpenhomeOrgSender1 iService;
    }
*/

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
