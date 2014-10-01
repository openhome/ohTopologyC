#include <OpenHome/ServiceReceiver.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Service.h>
#include <OpenHome/Network.h>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;


ServiceReceiver::ServiceReceiver(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, &aDevice, aLog)
    ,iMetadata(new Watchable<IInfoMetadata*>(aNetwork, Brn("Metadata"), InfoMetadata::Empty()))
    ,iTransportState(new Watchable<Brn>(aNetwork, Brn("TransportState"), Brx::Empty()))
    ,iCurrentMetadata(NULL)
{
}

void ServiceReceiver::Dispose()
{
    Service::Dispose();
    iMetadata->Dispose();
    iTransportState->Dispose();
    delete iMetadata;
    delete iTransportState;
    delete iCurrentMetadata;
    iMetadata = NULL;
    iTransportState = NULL;
}

IProxy* ServiceReceiver::OnCreate(IDevice* aDevice)
{
    return(new ProxyReceiver(*this, *aDevice));
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


ServiceReceiverNetwork::ServiceReceiverNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceReceiver(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgReceiver1(aCpDevice);

    Functor fHmc = MakeFunctor(*this, &ServiceReceiverNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(fHmc);

    Functor fHtsc = MakeFunctor(*this, &ServiceReceiverNetwork::HandleTransportStateChanged);
    iService->SetPropertyTransportStateChanged(fHtsc);

    Functor fHie = MakeFunctor(*this, &ServiceReceiverNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(fHie);
}


void ServiceReceiverNetwork::Dispose()
{
    ServiceReceiver::Dispose();

    delete iService;
    iService = NULL;

    iCpDevice.RemoveRef();
}


Job* ServiceReceiverNetwork::OnSubscribe()
{
    ASSERT(iSubscribedSource == NULL);

    iSubscribedSource = new JobDone();

    iService->Subscribe();

    return(iSubscribedSource->GetJob());

    //FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::OnSubscribeCallback);

    //Job* job = iSubscribedSource->GetJob()->ContinueWith(f, NULL);
    //return(job);
    //return iSubscribedSource->GetJob()->ContinueWith((t) => { });
}

/*
void ServiceReceiverNetwork::OnSubscribeCallback(void* aObj)
{

}
*/
void ServiceReceiverNetwork::OnCancelSubscribe()
{
    if (iSubscribedSource != NULL)
    {
        //iSubscribedSource->TrySetCancelled();
        iSubscribedSource->Cancel();
    }
}

void ServiceReceiverNetwork::HandleInitialEvent()
{
    Brhz protocolInfo;
    iService->PropertyProtocolInfo(protocolInfo);
    iProtocolInfo.Replace(protocolInfo);


    if (!iSubscribedSource->GetJob()->IsCancelled())
    {
        iSubscribedSource->SetResult(true);
    }

}

void ServiceReceiverNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribedSource = NULL;
}

void ServiceReceiverNetwork::Play()
{
    Job2* job = new Job2();
    FunctorAsync f = job->AsyncCb();
    iService->BeginPlay(f);

/*
    iService->BeginPlay((ptr) =>
    {
        try
        {
            iService->EndPlay(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/

    //jobDone->GetJob().ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
}





void ServiceReceiverNetwork::Play(ISenderMetadata& aMetadata)
{
    FunctorAsync f = MakeFunctorAsync(*this, &ServiceReceiverNetwork::BeginSetSenderCallback);
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


void ServiceReceiverNetwork::BeginSetSenderCallback(IAsync& aAsync)
{
    Job2* job = new Job2();
    FunctorAsync f = job->AsyncCb();
    iService->BeginPlay(f);
}



void ServiceReceiverNetwork::Stop()
{
    Job2* job = new Job2();
    FunctorAsync f = job->AsyncCb();
    iService->BeginStop(f);

/*
    iService->BeginStop((ptr) =>
    {
        try
        {
            iService->EndStop(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
*/
    //jobDone->GetJob().ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    //return (jobDone->GetJob());
}




void ServiceReceiverNetwork::HandleMetadataChanged()
{
    Brhz metadata;
    iService->PropertyMetadata(metadata);

    IMediaMetadata* mediaMetadata = iNetwork.TagManager().FromDidlLite(metadata);

    Brhz uri;
    iService->PropertyUri(uri);

    IInfoMetadata* infoMetadata = new InfoMetadata(mediaMetadata, Brn(uri)); // FIXME: is it ok to new this here rather than in the functor below ???


    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::MetadataChangedCallback);
    iNetwork.Schedule(f, infoMetadata);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMetadata->Update(new InfoMetadata(metadata, uri));
        });
    });
*/
}


void ServiceReceiverNetwork::MetadataChangedCallback(void* aInfoMetadata)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::MetadataChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, aInfoMetadata);
}


void ServiceReceiverNetwork::MetadataChangedCallbackCallback(void* aInfoMetadata)
{
    IInfoMetadata* infoMetadata = (IInfoMetadata*)aInfoMetadata;
    iMetadata->Update(infoMetadata);
    delete iCurrentMetadata;
    iCurrentMetadata = infoMetadata;
}



void ServiceReceiverNetwork::HandleTransportStateChanged()
{
    Brhz transportState;
    iService->PropertyTransportState(transportState);
    Bws<100>* newTransportState = new Bws<100>(transportState);

    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::TransportChangedCallback);
    iNetwork.Schedule(f, newTransportState);
/*
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iTransportState.Update(transportState);
        });
    });
*/
}


void ServiceReceiverNetwork::TransportChangedCallback(void* aTransportState)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceReceiverNetwork::TransportChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, aTransportState);
}

void ServiceReceiverNetwork::TransportChangedCallbackCallback(void* aTransportState)
{
    Bws<100>* transportState = (Bws<100>*)aTransportState;
    iTransportState->Update(Brn(*transportState));
    delete iCurrentTransportState;
    iCurrentTransportState = transportState;
}


////////////////////////////////////////////////////////////////

ServiceReceiverMock::ServiceReceiverMock(INetwork& aNetwork, IInjectorDevice& aDevice, const Brx& aMetadata, const Brx& aProtocolInfo,
                                         const Brx& aTransportState, const Brx& aUri, ILog& aLog)
    :ServiceReceiver(aNetwork, aDevice, aLog)
{
    iProtocolInfo.Replace(aProtocolInfo);

    iCurrentMetadata = new InfoMetadata(aNetwork.TagManager().FromDidlLite(aMetadata), aUri);
    iMetadata->Update(iCurrentMetadata);
    iTransportState->Update(Brn(aTransportState));
}


void ServiceReceiverMock::Play()
{
    //return(0);

/*
    return Start(() =>
    {
        iTransportState.Update(Brn("Playing"));
    });
*/
}


void ServiceReceiverMock::Play(ISenderMetadata& aMetadata)
{
    //return(0); // FIXME
/*
    return Start(() =>
    {
        iMetadata.Update(new InfoMetadata(iNetwork.TagManager.FromDidlLite(aMetadata.ToString()), aMetadata.Uri));
        iTransportState.Update(Brn("Playing"));
    });
*/
}

void ServiceReceiverMock::Stop()
{
    //return(0); // FIXME
/*
    return Start(() =>
    {
        iTransportState.Update(Brn("Stopped"));
    });
*/
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
        IInfoMetadata* metadata = new InfoMetadata(iNetwork.TagManager().FromDidlLite(allButLastToken), lastToken);
        iMetadata->Update(metadata);

        delete iCurrentMetadata;

        iCurrentMetadata = metadata;
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


