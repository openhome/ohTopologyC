#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Network.h>
//#include <OpenHome/Net/Private/XmlParser.h>
#include <Generated/CpAvOpenhomeOrgSender1.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;


/////////////////////////////////////////////////////////

ServiceSender::ServiceSender(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
    ,iAudio(new Watchable<TBool>(aNetwork, Brn("Audio"), false))
    ,iMetadata(new Watchable<ISenderMetadata*>(aNetwork, Brn("Metadata"), SenderMetadata::Empty()))
    ,iStatus(new Watchable<Brn>(aNetwork, Brn("Status"), Brx::Empty()))
    ,iCurrentMetadata(NULL)
	,iCurrentStatus(NULL)
{
}


ServiceSender::~ServiceSender()
{
    delete iAudio;
    delete iMetadata;
    delete iStatus;
    if (iCurrentMetadata!= SenderMetadata::Empty())
	{
		delete iCurrentMetadata;
	}
    delete iCurrentStatus;
}


void ServiceSender::Dispose()
{
    Service::Dispose();
    iAudio->Dispose();
    iMetadata->Dispose();
    iStatus->Dispose();
}


IProxy* ServiceSender::OnCreate(IDevice& aDevice)
{
    return(new ProxySender(*this, aDevice));
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



ServiceSenderNetwork::ServiceSenderNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceSender(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
    ,iSubscribedSource(NULL)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgSender1(aCpDevice);


    Functor f1 = MakeFunctor(*this, &ServiceSenderNetwork::HandleAudioChanged);
    iService->SetPropertyAudioChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceSenderNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceSenderNetwork::HandleStatusChanged);
    iService->SetPropertyStatusChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceSenderNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f4);
}


void ServiceSenderNetwork::Dispose()
{
    ServiceSender::Dispose();

    delete iService;
    iService = NULL;

    iCpDevice.RemoveRef();
}


Job* ServiceSenderNetwork::OnSubscribe()
{
    ASSERT(iSubscribedSource == NULL);

    iSubscribedSource = new JobDone();

    iService->Subscribe();

    return(iSubscribedSource->GetJob());


    //FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::OnSubscribeCallback);
    //Job* job = iSubscribedSource->GetJob()->ContinueWith(f, NULL);
    //return iSubscribedSource->GetJob()->ContinueWith((t) => { });
    //return(job);
}


void ServiceSenderNetwork::OnCancelSubscribe()
{
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->Cancel();
    }
}

void ServiceSenderNetwork::HandleInitialEvent()
{
    Brhz attributes;
    iService->PropertyAttributes(attributes);
    iAttributes.Replace(attributes);

    Brhz presentationUrl;
    iService->PropertyPresentationUrl(presentationUrl);
    iPresentationUrl.Replace(presentationUrl);

    if (!iSubscribedSource->GetJob()->IsCancelled())
    {
        iSubscribedSource->SetResult(true);
    }
}

void ServiceSenderNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribedSource = NULL;
}

void ServiceSenderNetwork::HandleAudioChanged()
{
    iService->PropertyAudio(iAudioValue);
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::AudioChangedCallback);
    iNetwork.Schedule(f, &iAudioValue);

/*
    Network.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iAudio->Update(audio);
        });
    });
*/
}


void ServiceSenderNetwork::AudioChangedCallback(void* aAudio)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::AudioChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, aAudio);
}


void ServiceSenderNetwork::AudioChangedCallbackCallback(void* aAudio)
{
    iAudio->Update(*(TBool*)aAudio);
}


void ServiceSenderNetwork::HandleMetadataChanged()
{
    Brhz metadata;
    iService->PropertyMetadata(metadata);
    ISenderMetadata* senderMetadata = new SenderMetadata(metadata);

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallback);
    iNetwork.Schedule(f, senderMetadata);
/*
    Network.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMetadata->Update(new SenderMetadata(metadata));
        });
    });
*/
}


void ServiceSenderNetwork::MetadataChangedCallback(void* aMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, aMetadata);
}


void ServiceSenderNetwork::MetadataChangedCallbackCallback(void* aMetadata)
{
    ISenderMetadata* metadata = (ISenderMetadata*)aMetadata;
    iMetadata->Update(metadata);
    delete iCurrentMetadata;
    iCurrentMetadata = metadata;
}


void ServiceSenderNetwork::HandleStatusChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::StatusChangedCallback);
    iNetwork.Schedule(f, NULL);
}


void ServiceSenderNetwork::StatusChangedCallback(void* aStatus)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::StatusChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, aStatus);
}


void ServiceSenderNetwork::StatusChangedCallbackCallback(void*)
{
    Brhz status;
    iService->PropertyStatus(status);

	Bws<100>* oldStatus = iCurrentStatus;
	Bws<100>* iCurrentStatus = new Bws<100>(status);
	
	iStatus->Update(Brn(*iCurrentStatus));
    delete oldStatus;
}



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

        delete iCurrentMetadata;

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
