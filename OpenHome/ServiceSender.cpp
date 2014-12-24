#include <OpenHome/ServiceSender.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Network.h>
#include <Generated/CpAvOpenhomeOrgSender1.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;


/////////////////////////////////////////////////////////

ServiceSender::ServiceSender(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
    ,iAudio(new Watchable<TBool>(aNetwork, Brn("Audio"), false))
    ,iMetadata(new Watchable<ISenderMetadata*>(aNetwork, Brn("Metadata"), aNetwork.SenderMetadataEmpty()))
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



ServiceSenderNetwork::ServiceSenderNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceSender(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
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

TBool ServiceSenderNetwork::OnSubscribe()
{
    iService->Subscribe();
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
    iAudio->Update(aAudio>0);
}


void ServiceSenderNetwork::HandleMetadataChanged()
{
    Brhz metadata;
    iService->PropertyMetadata(metadata);
    ISenderMetadata* senderMetadata = new SenderMetadata(metadata);

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallback1);
    Schedule(f, senderMetadata);
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


void ServiceSenderNetwork::MetadataChangedCallback1(void* aMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceSenderNetwork::MetadataChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, aMetadata);
}


void ServiceSenderNetwork::MetadataChangedCallback2(void* aMetadata)
{
    ISenderMetadata* metadata = (ISenderMetadata*)aMetadata;
    iMetadata->Update(metadata);
    delete iCurrentMetadata;
    iCurrentMetadata = metadata;
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

/////////////////////////////////////////////////////////////////

Topology3Sender::Topology3Sender()
    :iEnabled(false)
    ,iDevice(NULL)
{
}


Topology3Sender::Topology3Sender(IDevice& aDevice)
    :iEnabled(true)
    ,iDevice(&aDevice)
{
}


TBool Topology3Sender::Enabled()
{
    return iEnabled;
}


IDevice& Topology3Sender::Device()
{
    ASSERT(iEnabled)
    return *iDevice;
}
