#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceInfo.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
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

ServiceInfo::ServiceInfo(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iDetails(new Watchable<IInfoDetails*>(iNetwork, Brn("Details"), iNetwork.InfoDetailsEmpty()))
    ,iMetadata(new Watchable<IInfoMetadata*>(iNetwork, Brn("Metadata"), iNetwork.InfoMetadataEmpty()))
    ,iMetatext(new Watchable<IInfoMetatext*>(iNetwork, Brn("Metatext"), iNetwork.InfoMetatextEmpty()))
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

ServiceInfoNetwork::ServiceInfoNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgInfo1* aService, ILog& aLog)
    :ServiceInfo(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
    ,iDetailsChanged(false)
{
    Functor f1 = MakeFunctor(*this, &ServiceInfoNetwork::HandlePendingDetailsChanged);
    iService->SetPropertyBitRateChanged(f1);
    iService->SetPropertyCodecNameChanged(f1);
    iService->SetPropertyDurationChanged(f1);
    iService->SetPropertyLosslessChanged(f1);
    iService->SetPropertySampleRateChanged(f1);
    iService->SetPropertyBitDepthChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceInfoNetwork::HandleDetailsChanged);
    iService->SetPropertyBitDepthChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceInfoNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceInfoNetwork::HandleMetatextChanged);
    iService->SetPropertyMetatextChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServiceInfoNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f5);
}

ServiceInfoNetwork::~ServiceInfoNetwork()
{
    delete iService;
}

void ServiceInfoNetwork::Dispose()
{
    ServiceInfo::Dispose();
}

TBool ServiceInfoNetwork::OnSubscribe()
{
    iService->Subscribe();
    iSubscribed = true;
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

		//if (!iSubscribedSource->iCancelled)
   // {
        SubscribeCompleted();
   // }

}

void ServiceInfoNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }
    iSubscribed = false;
    //iSubscribedSource = NULL;
}



void ServiceInfoNetwork::HandlePendingDetailsChanged()
{
    iDetailsChanged = true;
}

void ServiceInfoNetwork::HandleDetailsChanged()
{
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
    if((iDetailsChanged) && (iSubscribed))
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

        iDetailsChanged = false;
    }

}

void ServiceInfoNetwork::HandleMetadataChanged()
{
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
    if (iSubscribed)
    {
        Brhz metadatastr;
        iService->PropertyMetadata(metadatastr);

        IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metadatastr);
        Brhz uri;
        iService->PropertyUri(uri);

        InfoMetadata* data = new InfoMetadata(metadata, uri);
        iMetadata->Update(data);
    }
}


void ServiceInfoNetwork::HandleMetatextChanged()
{
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
    if (iSubscribed)
    {
        Brhz metatextstr;
        iService->PropertyMetatext(metatextstr);
        IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metatextstr);
        iMetatext->Update(new InfoMetatext(metadata));
    }
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


