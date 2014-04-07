#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Network.h>
#include <OpenHome/Net/Private/XmlParser.h>



using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;

SenderMetadata* SenderMetadata::iEmpty = new SenderMetadata();

SenderMetadata* SenderMetadata::Empty()
{
	return(iEmpty);
}


SenderMetadata::SenderMetadata()
{
}


SenderMetadata::SenderMetadata(const Brx& aMetadata)
{
    iMetadata.Replace(aMetadata);

    try
    {
        iName.Replace(XmlParserBasic::Find(Brn("title"), aMetadata));
		iUri.Replace(XmlParserBasic::Find(Brn("res"), aMetadata));
		iArtworkUri.Replace(XmlParserBasic::Find(Brn("albumArtURI"), aMetadata));

		if (iName.Equals(Brx::Empty()))
		{
				iName.Replace("No name element provided");
		}
    }
    catch(XmlError)
    {
	    iName.Replace("Invalid metadata XML");
    }

	
	
	
/*
    try
    {
        XmlDocument doc = new XmlDocument();
        XmlNamespaceManager nsManager = new XmlNamespaceManager(doc.NameTable);
        nsManager.AddNamespace("didl", "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
        nsManager.AddNamespace("upnp", "urn:schemas-upnp-org:metadata-1-0/upnp/");
        nsManager.AddNamespace("dc", "http://purl.org/dc/elements/1.1/");
        doc.LoadXml(aMetadata);

        XmlNode name = doc.FirstChild.SelectSingleNode("didl:item/dc:title", nsManager);
        if (name != null && name.FirstChild != null)
        {
            iName = name.FirstChild.Value;
        }
        else
        {
            iName = "No name element provided";
        }
        XmlNode uri = doc.FirstChild.SelectSingleNode("didl:item/didl:res", nsManager);
        if (uri != null && uri.FirstChild != null)
        {
            iUri = uri.FirstChild.Value;
        }
        XmlNode artworkUri = doc.FirstChild.SelectSingleNode("didl:item/upnp:albumArtURI", nsManager);
        if (artworkUri != null && artworkUri.FirstChild != null)
        {
            iArtworkUri = artworkUri.FirstChild.Value;
        }
    }
    catch (XmlException)
    {
        iName = "Invalid metadata XML";
    }
*/
}

const Brx& SenderMetadata::Name()
{
    return iName;
}

const Brx& SenderMetadata::Uri()
{
    return iUri;
}

const Brx& SenderMetadata::ArtworkUri()
{
    return iArtworkUri;
}

const Brx& SenderMetadata::ToString()
{
    return iMetadata;
}

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
    iAudio = NULL;

    iMetadata->Dispose();
    iMetadata = NULL;

    iStatus->Dispose();
    iStatus = NULL;
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
    iMetadata->Update(aMetadata);
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
        iMetadata->Update(new SenderMetadata(aValue.Next()));
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
