#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Service.h>
#include <OpenHome/Network.h>
#include <Generated/CpAvOpenhomeOrgReceiver1.h>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;


ServiceReceiver::ServiceReceiver(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iCurrentMetadata(iNetwork.InfoMetadataEmpty())
    ,iCurrentTransportState(NULL)
    ,iMetadata(new Watchable<IInfoMetadata*>(iNetwork, Brn("Metadata"), iCurrentMetadata))
    ,iTransportState(new Watchable<Brn>(iNetwork, Brn("TransportState"), Brx::Empty()))
{
}


ServiceReceiver::~ServiceReceiver()
{
    delete iMetadata;
    delete iTransportState;
    if (iCurrentMetadata!=iNetwork.InfoMetadataEmpty())
    {
        delete iCurrentMetadata;
    }
    delete iCurrentTransportState;

}

void ServiceReceiver::Dispose()
{
    Service::Dispose();
    iMetadata->Dispose();
    iTransportState->Dispose();
}


IProxy* ServiceReceiver::OnCreate(IDevice& aDevice)
{
    return(new ProxyReceiver(*this, aDevice));
}


IWatchable<IInfoMetadata*>& ServiceReceiver::Metadata()
{
    return(*iMetadata);
}


IWatchable<Brn>& ServiceReceiver::TransportState()
{
    return(*iTransportState);
}


const Brx& ServiceReceiver::ProtocolInfo()
{
    return iProtocolInfo;
}


////////////////////////////////////////////////////////////////


ServiceReceiverNetwork::ServiceReceiverNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgReceiver1* aService, ILog& aLog)
    :ServiceReceiver(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
{
    Functor f1 = MakeFunctor(*this, &ServiceReceiverNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceReceiverNetwork::HandleTransportStateChanged);
    iService->SetPropertyTransportStateChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceReceiverNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f3);
}

ServiceReceiverNetwork::~ServiceReceiverNetwork()
{
    delete iService;
}


void ServiceReceiverNetwork::Dispose()
{
    ServiceReceiver::Dispose();
}


TBool ServiceReceiverNetwork::OnSubscribe()
{
    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
}


void ServiceReceiverNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->iCancelled = true;
    }
*/
}


void ServiceReceiverNetwork::HandleInitialEvent()
{
    Brhz protocolInfo;
    iService->PropertyProtocolInfo(protocolInfo);
    iProtocolInfo.Replace(protocolInfo);

    //if (!iSubscribedSource->iCancelled)
    //{
        SubscribeCompleted();
    //}
}

void ServiceReceiverNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
    iSubscribed = false;
}

void ServiceReceiverNetwork::Play()
{
    FunctorAsync f;;
    iService->BeginPlay(f);
}


void ServiceReceiverNetwork::Play(ISenderMetadata& aMetadata)
{
    FunctorAsync f = MakeFunctorAsync(*this, &ServiceReceiverNetwork::BeginSetSenderCallback1);
    iService->BeginSetSender(aMetadata.Uri(), aMetadata.ToString(), f);

/*
    iService->BeginSetSender(aMetadata.Uri, aMetadata.ToString(), (ptr1) =>
    {
        try
        {
            iService->EndSetSender(ptr1);
            iService->BeginPlay((ptr2) =>
            {
                try
                {
                    iService->EndPlay(ptr2);
                    taskSource.SetResult(true);
                }
                catch (Exception e)
                {
                    taskSource.SetException(e);
                }
            });
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/
    //jobDone->GetJob()->ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    //return (jobDone->GetJob());
}


void ServiceReceiverNetwork::BeginSetSenderCallback1(IAsync& /*aAsync*/)
{
    // We cannot call iService.Begin...(anything) directly within an AsyncCallback (potential deadlock)
    // Execute it on the watchable thread and return immediately.
    auto f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::BeginSetSenderCallback2);
    Schedule(f, NULL);

    //FunctorAsync f;
    //iService->BeginPlay(f);
}


void ServiceReceiverNetwork::BeginSetSenderCallback2(void*)
{
    FunctorAsync f;
    iService->BeginPlay(f);
}


void ServiceReceiverNetwork::Stop()
{
    FunctorAsync f;
    iService->BeginStop(f);
}


void ServiceReceiverNetwork::HandleMetadataChanged()
{
    Brhz metadata;
    iService->PropertyMetadata(metadata);

    IMediaMetadata* mediaMetadata = iNetwork.GetTagManager().FromDidlLite(metadata);

    Brhz uri;
    iService->PropertyUri(uri);

    IInfoMetadata* infoMetadata = new InfoMetadata(mediaMetadata, Brn(uri)); // FIXME: is it ok to new this here rather than in the functor below ???

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::MetadataChangedCallback1);
    Schedule(f, infoMetadata);
}


void ServiceReceiverNetwork::MetadataChangedCallback1(void* aInfoMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::MetadataChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, aInfoMetadata);
}


void ServiceReceiverNetwork::MetadataChangedCallback2(void* aInfoMetadata)
{
    if (iSubscribed)
    {
        IInfoMetadata* infoMetadata = (IInfoMetadata*)aInfoMetadata;
        iMetadata->Update(infoMetadata);
        delete iCurrentMetadata;
        iCurrentMetadata = infoMetadata;
    }
}



void ServiceReceiverNetwork::HandleTransportStateChanged()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::TransportChangedCallback1);
    Schedule(f, NULL);
}


void ServiceReceiverNetwork::TransportChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::TransportChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceReceiverNetwork::TransportChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz transportState;
        iService->PropertyTransportState(transportState);

        Bws<100>* oldTransportState = iCurrentTransportState;
        Bws<100>* iCurrentTransportState = new Bws<100>(transportState);

        iTransportState->Update(Brn(*iCurrentTransportState));
        delete oldTransportState;
    }
}


////////////////////////////////////////////////////////////////

ServiceReceiverMock::ServiceReceiverMock(IInjectorDevice& aDevice, const Brx& aMetadata, const Brx& aProtocolInfo,
                                         const Brx& aTransportState, const Brx& aUri, ILog& aLog)
    :ServiceReceiver(aDevice, aLog)
{
    iProtocolInfo.Replace(aProtocolInfo);
    iCurrentMetadata = new InfoMetadata(iNetwork.GetTagManager().FromDidlLite(aMetadata), aUri);
    iMetadata->Update(iCurrentMetadata);
    iTransportState->Update(Brn(aTransportState));
}


void ServiceReceiverMock::Play()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverMock::PlayCallback);
    Start(f, NULL);
/*
    return Start(() =>
    {
        iTransportState.Update(Brn("Playing"));
    });
*/
}


void ServiceReceiverMock::PlayCallback(void* /*aArg*/)
{
    iTransportState->Update(Brn("Playing"));
}


void ServiceReceiverMock::Play(ISenderMetadata& aMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverMock::PlayMetaCallback);
    Start(f, &aMetadata);
/*
    return Start(() =>
    {
        iMetadata.Update(new InfoMetadata(iNetwork.TagManager.FromDidlLite(aMetadata.ToString()), aMetadata.Uri));
        iTransportState.Update(Brn("Playing"));
    });
*/
}


void ServiceReceiverMock::PlayMetaCallback(void* aMetadata)
{
    auto metadata = (ISenderMetadata*)aMetadata;
    iMetadata->Update(new InfoMetadata(iNetwork.GetTagManager().FromDidlLite(metadata->ToString()), metadata->Uri()));
    iTransportState->Update(Brn("Playing"));
}


void ServiceReceiverMock::Stop()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverMock::StopCallback);
    Start(f, NULL);
/*
    return Start(() =>
    {
        iTransportState.Update(Brn("Stopped"));
    });
*/
}


void ServiceReceiverMock::StopCallback(void*)
{
    iTransportState->Update(Brn("Stopped"));
}


void ServiceReceiverMock::Execute(ICommandTokens& aValue)
{
    Brn command = aValue.Next();

    if (Ascii::CaseInsensitiveEquals(command, Brn("protocolinfo")))
    {
        iProtocolInfo.Replace(aValue.RemainingTrimmed());
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("metadata")))
    {
        if (aValue.Count() < 2)
        {
            //throw new NotSupportedException();
        }

        // Get the remaining tokens
        Brn remaining(aValue.RemainingTrimmed());

        // Get the last token
        TUint count = aValue.Count()-1;

        for(TUint i=0; i<count; i++)
        {
            aValue.Next();
        }

        Brn lastToken(aValue.Next());

        // Remove last token from remaining tokens
        TUint allButLastTokenBytes = remaining.Bytes()-lastToken.Bytes();
        Brn allButLastToken(remaining.Split(0, allButLastTokenBytes));

        // FIXME : use a static function to do the new here (allowing implementation of a fixed size pool of objects in future)
        auto metadata = iCurrentMetadata;
        iCurrentMetadata = new InfoMetadata(iNetwork.GetTagManager().FromDidlLite(allButLastToken), lastToken);
        iMetadata->Update(iCurrentMetadata);

        if (metadata!=iNetwork.InfoMetadataEmpty())
        {
            delete metadata;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("transportstate")))
    {
        Brn state(aValue.Next());

        if (state.Equals(kTransportStatePlaying))
        {
            iTransportState->Update(kTransportStatePlaying);
        }
        else if (state.Equals(kTransportStateStopped))
        {
            iTransportState->Update(kTransportStateStopped);
        }
        else if (state.Equals(kTransportStatePaused))
        {
            iTransportState->Update(kTransportStatePaused);
        }
    }
    else
    {
        //throw new NotSupportedException();
    }
}


///////////////////////////////////////////////////////////////////////


ProxyReceiver::ProxyReceiver(ServiceReceiver& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

const Brx& ProxyReceiver::ProtocolInfo()
{
    return iService.ProtocolInfo();
}

IWatchable<IInfoMetadata*>& ProxyReceiver::Metadata()
{
    return iService.Metadata();
}

IWatchable<Brn>& ProxyReceiver::TransportState()
{
    return iService.TransportState();
}


void ProxyReceiver::Play()
{
    iService.Play();
    //return (iService.Play());
}

void ProxyReceiver::Play(ISenderMetadata& aMetadata)
{
    iService.Play(aMetadata);
//    return (iService.Play(aMetadata));
}

void ProxyReceiver::Stop()
{
    iService.Stop();
//    return (iService.Stop());
}



void ProxyReceiver::Dispose()
{
    iService.Unsubscribe();
}


IDevice& ProxyReceiver::Device()
{
    return(iDevice);
}


