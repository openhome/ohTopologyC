#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Network.h>
#include <Generated/CpAvOpenhomeOrgSender1.h>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;


/////////////////////////////////////////////////////////

ServiceSender::ServiceSender(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iCurrentMetadata(iNetwork.SenderMetadataEmpty())
    ,iCurrentStatus(new Bws<100>())
    ,iAudio(new Watchable<TBool>(iNetwork, Brn("Audio"), false))
    ,iMetadata(new Watchable<ISenderMetadata*>(iNetwork, Brn("Metadata"), iCurrentMetadata))
    ,iStatus(new Watchable<Brn>(iNetwork, Brn("Status"), Brn(*iCurrentStatus) ))
{
}


ServiceSender::~ServiceSender()
{
    delete iAudio;
    delete iMetadata;
    delete iStatus;
    if (iCurrentMetadata!= iNetwork.SenderMetadataEmpty())
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



ServiceSenderNetwork::ServiceSenderNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgSender1* aService, ILog& aLog)
    :ServiceSender(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
{
    Functor f1 = MakeFunctor(*this, &ServiceSenderNetwork::HandleAudioChanged);
    iService->SetPropertyAudioChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceSenderNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceSenderNetwork::HandleStatusChanged);
    iService->SetPropertyStatusChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceSenderNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f4);
}

ServiceSenderNetwork::~ServiceSenderNetwork()
{
    delete iService;
}

void ServiceSenderNetwork::Dispose()
{
    ServiceSender::Dispose();
}

TBool ServiceSenderNetwork::OnSubscribe()
{
    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
}


void ServiceSenderNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->iCancelled = true;
    }
*/
}

void ServiceSenderNetwork::HandleInitialEvent()
{
    Brhz attributes;
    iService->PropertyAttributes(attributes);
    iAttributes.Replace(attributes);

    Brhz presentationUrl;
    iService->PropertyPresentationUrl(presentationUrl);
    iPresentationUrl.Replace(presentationUrl);

    //if (!iSubscribedSource->iCancelled)
    //{
        SubscribeCompleted();
    //}
}

void ServiceSenderNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
    iSubscribed = false;
}

void ServiceSenderNetwork::HandleAudioChanged()
{
    TBool audio;
    iService->PropertyAudio(audio);
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::AudioChangedCallback1);

    if (audio)
    {
        Schedule(f, (void*)1);
    }
    else
    {
        Schedule(f, (void*)0);
    }

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


void ServiceSenderNetwork::AudioChangedCallback1(void* aAudio)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::AudioChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, aAudio);
}


void ServiceSenderNetwork::AudioChangedCallback2(void* aAudio)
{
    if (iSubscribed)
    {
        iAudio->Update(aAudio>0);
    }
}


void ServiceSenderNetwork::HandleMetadataChanged()
{
    if (iSubscribed)
    {
        Brhz metadata;
        iService->PropertyMetadata(metadata);
        ISenderMetadata* senderMetadata = new SenderMetadata(metadata);

        FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallback1);
        Schedule(f, senderMetadata);
    }
}


void ServiceSenderNetwork::MetadataChangedCallback1(void* aMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, aMetadata);
}


void ServiceSenderNetwork::MetadataChangedCallback2(void* aMetadata)
{
    if (iSubscribed)
    {
        auto oldMetaData = iCurrentMetadata;
        iCurrentMetadata = (ISenderMetadata*)aMetadata;
        iMetadata->Update(iCurrentMetadata);
        if (oldMetaData!=iNetwork.SenderMetadataEmpty())
        {
            delete oldMetaData;
        }

/*
        ISenderMetadata* metadata = (ISenderMetadata*)aMetadata;
        iMetadata->Update(metadata);
        delete iCurrentMetadata;
        iCurrentMetadata = metadata;
*/
    }
}


void ServiceSenderNetwork::HandleStatusChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::StatusChangedCallback1);
    Schedule(f, NULL);
}


void ServiceSenderNetwork::StatusChangedCallback1(void* aStatus)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::StatusChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, aStatus);
}


void ServiceSenderNetwork::StatusChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz status;
        iService->PropertyStatus(status);

        auto oldStatus = iCurrentStatus;
        iCurrentStatus = new Bws<100>(status);
        iStatus->Update(Brn(*iCurrentStatus));
        delete oldStatus;
    }
}



///////////////////////////////////////////////////////////////////

ServiceSenderMock::ServiceSenderMock(IInjectorDevice& aDevice, const Brx& aAttributes, const Brx& aPresentationUrl,
                                     TBool aAudio, ISenderMetadata* aMetadata, const Brx& aStatus, ILog& aLog)
    :ServiceSender(aDevice, aLog)
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

/////////////////////////////////////////////////////////////////

Sender::Sender()
    :iEnabled(false)
    ,iDevice(nullptr)
{
}


Sender::Sender(IDevice& aDevice)
    :iEnabled(true)
    ,iDevice(&aDevice)
{
}


TBool Sender::Enabled()
{
    return iEnabled;
}


IDevice& Sender::Device()
{
    ASSERT(iEnabled)
    return *iDevice;
}
