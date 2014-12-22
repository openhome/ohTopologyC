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

ServiceInfo::ServiceInfo(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
    ,iDetails(new Watchable<IInfoDetails*>(aNetwork, Brn("Details"), new InfoDetails()))
    ,iMetadata(new Watchable<IInfoMetadata*>(aNetwork, Brn("Metadata"), iNetwork.InfoMetadataEmpty()))
    ,iMetatext(new Watchable<IInfoMetatext*>(aNetwork, Brn("Metatext"), new InfoMetatext()))
{
}

ServiceInfo::~ServiceInfo()
{
    delete iDetails;
    delete iMetadata;
    delete iMetatext;
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
    iCpDevice.RemoveRef();
}

TBool ServiceInfoNetwork::OnSubscribe()
{
    iService->Subscribe();
    return(false); // false = not mock
}

void ServiceInfoNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->iCancelled = true;
    }
*/
}

void ServiceInfoNetwork::HandleInitialEvent()
{
/*
    if (!iSubscribedSource->iCancelled)
    {
        SubscribeCompleted();
    }
*/
}

void ServiceInfoNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    //iSubscribedSource = NULL;
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
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleDetailsChangedCallback1);
    Schedule(f, NULL);
}


void ServiceInfoNetwork::HandleDetailsChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleDetailsChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceInfoNetwork::HandleDetailsChangedCallback2(void*)
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
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetadataChangedCallback1);
    Schedule(f, NULL);

}

void ServiceInfoNetwork::HandleMetadataChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetadataChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);

}


void ServiceInfoNetwork::HandleMetadataChangedCallback2(void*)
{
    Brhz metadatastr;
    iService->PropertyMetadata(metadatastr);

    IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metadatastr);
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
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetatextChangedCallback1);
    Schedule(f, NULL);
}

void ServiceInfoNetwork::HandleMetatextChangedCallback1(void*)
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ServiceInfoNetwork::HandleMetatextChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceInfoNetwork::HandleMetatextChangedCallback2(void*)
{
    Brhz metatextstr;
    iService->PropertyMetatext(metatextstr);
    IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metatextstr);
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


