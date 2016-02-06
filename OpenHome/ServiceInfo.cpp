#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceInfo.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <Generated/CpAvOpenhomeOrgInfo1.h>
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

ServiceInfo::ServiceInfo(IInjectorDevice& aDevice)
    :Service(aDevice)
    ,iCurrentDetails(iNetwork.InfoDetailsEmpty())
    ,iCurrentMetadata(iNetwork.InfoMetadataEmpty())
    ,iCurrentMetatext(iNetwork.InfoMetatextEmpty())
    ,iDetails(new Watchable<IInfoDetails*>(iNetwork, Brn("Details"), iCurrentDetails))
    ,iMetadata(new Watchable<IInfoMetadata*>(iNetwork, Brn("Metadata"), iCurrentMetadata))
    ,iMetatext(new Watchable<IInfoMetatext*>(iNetwork, Brn("Metatext"), iCurrentMetatext))
{
}

ServiceInfo::~ServiceInfo()
{
    delete iDetails;
    delete iMetadata;
    delete iMetatext;
    if (iCurrentDetails!= iNetwork.InfoDetailsEmpty())
    {
        delete iCurrentDetails;
    }
    if (iCurrentMetadata!= iNetwork.InfoMetadataEmpty())
    {
        delete iCurrentMetadata;
    }
    if (iCurrentMetatext!= iNetwork.InfoMetatextEmpty())
    {
        delete iCurrentMetatext;
    }
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

ServiceInfoNetwork::ServiceInfoNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgInfo1* aService)
    :ServiceInfo(aDevice)
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
    iService->SetPropertyChanged(f2);

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

        auto oldDetails = iCurrentDetails;
        iCurrentDetails = new InfoDetails(bitDepth, bitRate, codec, duration, lossless, sampleRate);
        iDetails->Update(iCurrentDetails);
        if (oldDetails!=iNetwork.InfoDetailsEmpty())
        {
            delete oldDetails;
        }

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

        Brhz uri;
        iService->PropertyUri(uri);

        auto oldMetadata = iCurrentMetadata;
        iCurrentMetadata = new InfoMetadata(iNetwork.GetTagManager().FromDidlLite(metadatastr), uri);
        iMetadata->Update(iCurrentMetadata);

        if(oldMetadata!=iNetwork.InfoMetadataEmpty())
        {
            delete oldMetadata;
        }
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

        auto oldMetatext = iCurrentMetatext;
        iCurrentMetatext = new InfoMetatext(iNetwork.GetTagManager().FromDidlLite(metatextstr));
        iMetatext->Update(iCurrentMetatext);
        if(oldMetatext!=iNetwork.InfoMetatextEmpty())
        {
            delete oldMetatext;
        }
    }
}


/////////////////////////////////////////////////////////////
ServiceInfoMock::ServiceInfoMock(IInjectorDevice& aDevice, IInfoDetails* aDetails, IInfoMetadata* aMetadata, IInfoMetatext* aMetatext)
    :ServiceInfo(aDevice)
    ,iCurrentDetails(new InfoDetails(aDetails->BitDepth(), aDetails->BitRate(), aDetails->CodecName(), aDetails->Duration(), aDetails->Lossless(), aDetails->SampleRate()))
    ,iCurrentMetadata(aMetadata)
    ,iCurrentMetatext(aMetatext)
{
    iDetails->Update(iCurrentDetails);
    iMetadata->Update(iCurrentMetadata);
    iMetatext->Update(iCurrentMetatext);
}

ServiceInfoMock::~ServiceInfoMock()
{
    delete iCurrentDetails;
    delete iCurrentMetadata;
    delete iCurrentMetatext;

}

void ServiceInfoMock::Execute(ICommandTokens& aValue)
{
    Brn command = aValue.Next();

    if (Ascii::CaseInsensitiveEquals(command, Brn("details")))
    {
        if (aValue.Count() != 6)
        {
            Log::Print("Needs 6 values");
            return;
        }
        auto oldDetails = iCurrentDetails;
        iCurrentDetails = new InfoDetails(Ascii::Uint(aValue.Next()), //BitDepth
                                            Ascii::Uint(aValue.Next()), //BitRate
                                            aValue.Next(), //CodecName
                                            Ascii::Uint(aValue.Next()), //Duration
                                            Ascii::CaseInsensitiveEquals(aValue.Next(), Brn("true")), //Lossless
                                            Ascii::Uint(aValue.Next())); //SampleRate

        iDetails->Update(iCurrentDetails);
        delete oldDetails;
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("metadata")))
    {
        if (aValue.Count() != 2)
        {
            Log::Print("Needs 2 values");
            return;
        }
        auto oldMetadata = iCurrentMetadata;
        iCurrentMetadata = new InfoMetadata(iNetwork.GetTagManager().FromDidlLite(aValue.Next()), aValue.Next());
        iMetadata->Update(iCurrentMetadata);
        if (oldMetadata)
        {
            delete oldMetadata;
        }
    }
    else if (Ascii::CaseInsensitiveEquals(command, Brn("metatext")))
    {
        if (aValue.Count() != 1)
        {
            Log::Print("Needs 1 value");
            return;
        }
        auto oldMetatext = iCurrentMetatext;
        iCurrentMetatext = new InfoMetatext(iNetwork.GetTagManager().FromDidlLite(aValue.Next()));
        iMetatext->Update(iCurrentMetatext);
        if (oldMetatext)
        {
            delete oldMetatext;
        }
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
