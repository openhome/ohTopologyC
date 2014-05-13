#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Network.h>
//#include <OpenHome/Net/Private/XmlParser.h>



using namespace OpenHome;
using namespace OpenHome::Av;
//using namespace OpenHome::Net;


/////////////////////////////////////////////////////////

ServiceSender::ServiceSender(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, &aDevice, aLog)
    ,iAudio(new Watchable<TBool>(aNetwork, Brn("Audio"), false))
    ,iMetadata(new Watchable<ISenderMetadata*>(aNetwork, Brn("Metadata"), SenderMetadata::Empty()))
    ,iStatus(new Watchable<Brn>(aNetwork, Brn("Status"), Brx::Empty()))
{
}


void ServiceSender::Dispose()
{
    Service::Dispose();
    iAudio->Dispose();
    iMetadata->Dispose();
    iStatus->Dispose();
    delete iAudio;
    delete iMetadata;
    delete iStatus;
    delete iCurrentMetadata;

    iAudio = NULL;
    iMetadata = NULL;
    iStatus = NULL;
    iCurrentMetadata = NULL;
}

IProxy* ServiceSender::OnCreate(IDevice* aDevice)
{
    return(new ProxySender(*this, *aDevice));
}

IWatchable<TBool>& ServiceSender::Audio()
{
    return(*iAudio);
}

IWatchable<ISenderMetadata*>& ServiceSender::Metadata()
{
    return(*iMetadata);
}

IWatchable<Brn>& ServiceSender::Status()
{
    return(*iStatus);
}

const Brx& ServiceSender::Attributes()
{
    return iAttributes;
}

const Brx& ServiceSender::PresentationUrl()
{
    return iPresentationUrl;
}

///////////////////////////////////////////////////////////////


/*
    class ServiceSenderNetwork : ServiceSender
    {
        ServiceSenderNetwork(INetwork aNetwork, IInjectorDevice aDevice, CpDevice aCpDevice, ILog aLog)
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

        override void Dispose()
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
                    iAudio->Update(audio);
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
                    iMetadata->Update(new SenderMetadata(metadata));
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
                    iStatus->Update(status);
                });
            });
        }

        private readonly CpDevice iCpDevice;
        private TaskCompletionSource<TBool> iSubscribedSource;
        private CpProxyAvOpenhomeOrgSender1 iService;
    }
*/

///////////////////////////////////////////////////////////////////

ServiceSenderMock::ServiceSenderMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aAttributes, const Brx& aPresentationUrl,
                                     TBool aAudio, ISenderMetadata* aMetadata, const Brx& aStatus, ILog& aLog)
    :ServiceSender(aNetwork, aDevice, aLog)
{
    iAttributes.Replace(aAttributes);
    iPresentationUrl.Replace(aPresentationUrl);
    iAudio->Update(aAudio);
    iCurrentMetadata = aMetadata;
    iMetadata->Update(iCurrentMetadata);
    iStatus->Update(Brn(aStatus));
}

void ServiceSenderMock::Execute(ICommandTokens& aValue)
{
    Brn command = aValue.Next();

    if (Ascii::CaseInsensitiveEquals(command, Brn("attributes")))
    {
        iAttributes.Replace(aValue.Next());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("presentationurl")))
    {
        iPresentationUrl.Replace(aValue.Next());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("audio")))
    {
        iAudio->Update(aValue.Next().Equals(Brn("True")));
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("metadata")))
    {
        ISenderMetadata* metadata = new SenderMetadata(aValue.Next());

        iMetadata->Update(metadata);

        //if (iCurrentMetadata!=NULL)
        //{
            delete iCurrentMetadata;
        //}

        iCurrentMetadata = metadata;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("status")))
    {
        iStatus->Update(aValue.Next());
    }
    else
    {
        //throw new NotSupportedException();
    }
}


////////////////////////////////////////////////////////

ProxySender::ProxySender(ServiceSender& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

const Brx& ProxySender::Attributes()
{
    return iService.Attributes();
}

const Brx& ProxySender::PresentationUrl()
{
    return iService.PresentationUrl();
}

IWatchable<TBool>& ProxySender::Audio()
{
    return iService.Audio();
}

IWatchable<ISenderMetadata*>& ProxySender::Metadata()
{
    return iService.Metadata();
}

IWatchable<Brn>& ProxySender::Status()
{
   return iService.Status();
}


void ProxySender::Dispose()
{
    iService.Unsubscribe();
}


IDevice& ProxySender::Device()
{
    return(iDevice);
}
