#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceInfo.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;


InfoDetails::InfoDetails()
    :iBitDepth(0)
    ,iBitRate(0)
    ,iCodecName(Brx::Empty())
    ,iDuration(0)
    ,iLossless(false)
    ,iSampleRate(0)
{

}

InfoDetails::InfoDetails(TUint aBitDepth, TUint aBitRate, const Brx& aCodecName, TUint aDuration, TBool aLossless, TUint aSampleRate)
    :iBitDepth(aBitDepth)
    ,iBitRate(aBitRate)
    ,iCodecName(aCodecName)
    ,iDuration(aDuration)
    ,iLossless(aLossless)
    ,iSampleRate(aSampleRate)
{
}

TUint InfoDetails::BitDepth()
{
    return iBitDepth;
}

TUint InfoDetails::BitRate()
{
    return iBitRate;
}

const Brx& InfoDetails::CodecName()
{
    return iCodecName;
}

TUint InfoDetails::Duration()
{
    return iDuration;
}

TBool InfoDetails::Lossless()
{
    return iLossless;
}

TUint InfoDetails::SampleRate()
{
    return iSampleRate;
}


//////////////////////////////////////////////////////////

/*
InfoMetadata* InfoMetadata::iEmpty = NULL;

IInfoMetadata* InfoMetadata::Empty()
{
    if(iEmpty==NULL)
    {
        iEmpty = new InfoMetadata();
    }
    return(iEmpty);
}

InfoMetadata::InfoMetadata()
    :iMetadata(NULL)
    ,iUri(NULL)
{
}

InfoMetadata::InfoMetadata(IMediaMetadata& aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
}

IMediaMetadata InfoMetadata::Metadata()
{
    return iMetadata;
}

const Brx& InfoMetadata::Uri()
{
    return iUri;
}

*/

//////////////////////////////////////////////////////////


InfoMetatext::InfoMetatext()
    :iMetatext(NULL)
{
}

InfoMetatext::InfoMetatext(IMediaMetadata& aMetatext)
    :iMetatext(&aMetatext)
{
}

IMediaMetadata& InfoMetatext::Metatext()
{
    return *iMetatext;
}

//////////////////////////////////////////////////////////

ServiceInfo::ServiceInfo(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
{
    iDetails = new Watchable<IInfoDetails*>(aNetwork, Brn("Details"), new InfoDetails());
    iMetadata = new Watchable<IInfoMetadata*>(aNetwork, Brn("Metadata"), InfoMetadata::Empty());
    iMetatext = new Watchable<IInfoMetatext*>(aNetwork, Brn("Metatext"), new InfoMetatext());
}

void ServiceInfo::Dispose()
{
    Service::Dispose();
    iDetails->Dispose();
    iMetadata->Dispose();
    iMetatext->Dispose();
}

IProxy* ServiceInfo::OnCreate(IDevice& aDevice)
{
    return (new ProxyInfo(*this, aDevice));
}

IWatchable<IInfoDetails*>& ServiceInfo::Details()
{
    return (*iDetails);
}

IWatchable<IInfoMetadata*>& ServiceInfo::Metadata()
{
    return (*iMetadata);
}

IWatchable<IInfoMetatext*>& ServiceInfo::Metatext()
{
    return (*iMetatext);
}

//////////////////////////////////////////////////////////

ServiceInfoNetwork::ServiceInfoNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServiceInfo(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgInfo1(aCpDevice);

    Functor f1 = MakeFunctor(*this, &ServiceInfoNetwork::HandleDetailsChanged);
    iService->SetPropertyBitDepthChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceInfoNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceInfoNetwork::HandleMetatextChanged);
    iService->SetPropertyMetatextChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceInfoNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f4);
}

ServiceInfoNetwork::~ServiceInfoNetwork()
{
    delete iService;
}

void ServiceInfoNetwork::Dispose()
{
    ServiceInfo::Dispose();

    //iService.Dispose();
    //iService = NULL;

    iCpDevice.RemoveRef();
}

Job* ServiceInfoNetwork::OnSubscribe()
{
    ASSERT(iSubscribedSource == NULL);
    iSubscribedSource = new JobDone();
    iService->Subscribe();
    return(iSubscribedSource->GetJob());
/*
    ASSERT(iSubscribedSource == NULL);

    iSubscribedSource = new TaskCompletionSource<TBool>();

    iService.Subscribe();

    return iSubscribedSource.Task.ContinueWith((t) => { });
*/
}

void ServiceInfoNetwork::OnCancelSubscribe()
{
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->Cancel();
    }
}

void ServiceInfoNetwork::HandleInitialEvent()
{
    if (!iSubscribedSource->GetJob()->IsCancelled())
    {
        iSubscribedSource->SetResult(true);
    }
}

void ServiceInfoNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    iSubscribedSource = NULL;
}

void ServiceInfoNetwork::HandleDetailsChanged()
{
/*
    TUint bitDepth = iService.PropertyBitDepth();
    TUint bitRate = iService.PropertyBitRate();
    string codec = iService.PropertyCodecName();
    TUint duration = iService.PropertyDuration();
    TBool lossless = iService.PropertyLossless();
    TUint sampleRate = iService.PropertySampleRate();


    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iDetails.Update(
                new InfoDetails(
                    bitDepth,
                    bitRate,
                    codec,
                    duration,
                    lossless,
                    sampleRate
                ));
        });
    });
*/
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleDetailsChangedCallback);
    iNetwork.Schedule(f, NULL);
}


void ServiceInfoNetwork::HandleDetailsChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleDetailsChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceInfoNetwork::HandleDetailsChangedCallbackCallback(void*)
{
    TUint bitDepth;
    iService->PropertyBitDepth(bitDepth);

    TUint bitRate;
    iService->PropertyBitRate(bitRate);

    Brhz codec;
    iService->PropertyCodecName(codec);

    TUint duration;
    iService->PropertyDuration(duration);

    TBool lossless;
    iService->PropertyLossless(lossless);

    TUint sampleRate;
    iService->PropertySampleRate(sampleRate);

    InfoDetails* details = new InfoDetails(bitDepth, bitRate, codec, duration, lossless, sampleRate);
    iDetails->Update(details);

}

void ServiceInfoNetwork::HandleMetadataChanged()
{
/*
    IMediaMetadata metadata = iNetwork.TagManager.FromDidlLite(iService.PropertyMetadata());
    string uri = iService.PropertyUri();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMetadata.Update(
                new InfoMetadata(
                    metadata,
                    uri
                ));
        });
    });
*/
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetadataChangedCallback);
    iNetwork.Schedule(f, NULL);

}

void ServiceInfoNetwork::HandleMetadataChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetadataChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);

}


void ServiceInfoNetwork::HandleMetadataChangedCallbackCallback(void*)
{
    Brhz metadatastr;
    iService->PropertyMetadata(metadatastr);

    IMediaMetadata* metadata = iNetwork.TagManager().FromDidlLite(metadatastr);
    Brhz uri;
    iService->PropertyUri(uri);

    InfoMetadata* data = new InfoMetadata(metadata, uri);
    iMetadata->Update(data);
}


void ServiceInfoNetwork::HandleMetatextChanged()
{
/*
    IMediaMetadata metadata = iNetwork.TagManager.FromDidlLite(iService.PropertyMetatext());
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iMetatext.Update(new InfoMetatext(metadata));
        });
    });
*/
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetatextChangedCallback);
    iNetwork.Schedule(f, NULL);
}

void ServiceInfoNetwork::HandleMetatextChangedCallback(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetatextChangedCallbackCallback);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceInfoNetwork::HandleMetatextChangedCallbackCallback(void*)
{
    Brhz metatextstr;
    iService->PropertyMetatext(metatextstr);
    IMediaMetadata* metadata = iNetwork.TagManager().FromDidlLite(metatextstr);
    iMetatext->Update(new InfoMetatext(*metadata));
}



/////////////////////////////////////////////////////////////

ProxyInfo::ProxyInfo(ServiceInfo& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}


IWatchable<IInfoDetails*>& ProxyInfo::Details()
{
    return iService.Details();
}


IWatchable<IInfoMetadata*>& ProxyInfo::Metadata()
{
    return iService.Metadata();
}


IWatchable<IInfoMetatext*>& ProxyInfo::Metatext()
{
    return iService.Metatext();
}


IDevice& ProxyInfo::Device()
{
    return(iDevice);
}


void ProxyInfo::Dispose()
{
    iService.Unsubscribe();
}

