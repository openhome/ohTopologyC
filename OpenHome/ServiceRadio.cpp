#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServiceRadio.h>
#include <OpenHome/Network.h>
#include <OpenHome/AsyncAdaptor.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <Generated/CpAvOpenhomeOrgRadio1.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


MediaPresetRadio::MediaPresetRadio(IWatchableThread& aThread, TUint aIndex, TUint aId, IMediaMetadata& aMetadata, const Brx& aUri, ServiceRadio& aRadio)
    :iIndex(aIndex)
    ,iId(aId)
    ,iMetadata(aMetadata)
    ,iUri(aUri)
    ,iRadio(aRadio)
    ,iBuffering(new Watchable<TBool>(aThread, Brn("Buffering"), false))
    ,iPlaying(new Watchable<TBool>(aThread, Brn("Playing"), false))
    ,iSelected(new Watchable<TBool>(aThread, Brn("Selected"), false))
{
    iRadio.Id().AddWatcher(*this);
    iRadio.TransportState().AddWatcher(*this);
}

MediaPresetRadio::~MediaPresetRadio()
{
    delete iBuffering;
    delete iPlaying;
    delete iSelected;
}


void MediaPresetRadio::Dispose()
{
    iRadio.Id().RemoveWatcher(*this);
    iRadio.TransportState().RemoveWatcher(*this);
    iBuffering->Dispose();
    iPlaying->Dispose();
    iSelected->Dispose();
}

TUint MediaPresetRadio::Index()
{
    return(iIndex);
}

IMediaMetadata& MediaPresetRadio::Metadata()
{
    return(iMetadata);
}

IWatchable<TBool>& MediaPresetRadio::Buffering()
{
    return(*iBuffering);
}

IWatchable<TBool>& MediaPresetRadio::Playing()
{
    return(*iPlaying);
}

IWatchable<TBool>& MediaPresetRadio::Selected()
{
    return(*iSelected);
}

void MediaPresetRadio::Play()
{
    iBuffering->Update(iCurrentTransportState == Brn("Buffering"));

    if (iId > 0)
    {
        iRadio.SetId(iId, iUri);
        iRadio.Play();
    }
}

void MediaPresetRadio::EvaluatePlaying()
{
    iBuffering->Update((iCurrentId == iId) && (iCurrentTransportState == Brn("Buffering")));
    iPlaying->Update((iCurrentId == iId) && (iCurrentTransportState == Brn("Playing")));
    iSelected->Update(iCurrentId == iId);
}

void MediaPresetRadio::ItemOpen(const Brx& /*aId*/, TUint aValue)
{
    iCurrentId = aValue;
    EvaluatePlaying();
}

void MediaPresetRadio::ItemUpdate(const Brx& /*aId*/, TUint aValue, TUint /*aPrevious*/)
{
    iCurrentId = aValue;
    EvaluatePlaying();
}

void MediaPresetRadio::ItemClose(const Brx& /*aId*/, TUint /*aValue*/)
{
    iPlaying->Update(false);
}

void MediaPresetRadio::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iCurrentTransportState.Replace(aValue);
    EvaluatePlaying();
}

void MediaPresetRadio::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iCurrentTransportState.Replace(aValue);
    EvaluatePlaying();
}

void MediaPresetRadio::ItemClose(const Brx& /*aId*/, Brn aValue)
{
    iPlaying->Update(false);
}

////////////////////////////////////////////////////////////////////////////////


ServiceRadio::ServiceRadio(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iId(new Watchable<TUint>(iNetwork, Brn("Id"), 0))
    ,iTransportState(new Watchable<Brn>(iNetwork, Brn("TransportState"), Brx::Empty()))
    ,iMetadata(new Watchable<IInfoMetadata*>(iNetwork, Brn("Metadata"), iNetwork.InfoMetadataEmpty()))
    ,iCurrentTransportState(NULL)
{
}

ServiceRadio::~ServiceRadio()
{
    delete iId;
    delete iTransportState;
    delete iMetadata;
    delete iCurrentTransportState;

}

void ServiceRadio::Dispose()
{
    Service::Dispose();
    iId->Dispose();
    iTransportState->Dispose();
    iMetadata->Dispose();
}

IProxy* ServiceRadio::OnCreate(IDevice& aDevice)
{
    return(new ProxyRadio(*this, aDevice));
}

IWatchable<TUint>& ServiceRadio::Id()
{
    return(*iId);
}

IWatchable<Brn>& ServiceRadio::TransportState()
{
    return(*iTransportState);
}

IWatchable<IInfoMetadata*>& ServiceRadio::Metadata()
{
    return(*iMetadata);
}

IWatchable<IWatchableSnapshot<IMediaPreset*>*>& ServiceRadio::Snapshot()
{
    return(iMediaSupervisor->Snapshot());
}

TUint ServiceRadio::ChannelsMax()
{
    return(iChannelsMax);
}

const Brx& ServiceRadio::ProtocolInfo()
{
    return(iProtocolInfo);
}


////////////////////////////////////////////////////////////////////////////////

ServiceRadioNetwork::ServiceRadioNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgRadio1* aService, ILog& aLog)
    :ServiceRadio(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
{
    Functor f1 = MakeFunctor(*this, &ServiceRadioNetwork::HandleIdChanged);
    iService->SetPropertyIdChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServiceRadioNetwork::HandleIdArrayChanged);
    iService->SetPropertyIdArrayChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServiceRadioNetwork::HandleMetadataChanged);
    iService->SetPropertyMetadataChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServiceRadioNetwork::HandleTransportStateChanged);
    iService->SetPropertyTransportStateChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServiceRadioNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f5);
}

ServiceRadioNetwork::~ServiceRadioNetwork()
{
}

void ServiceRadioNetwork::Dispose()
{
    ServiceRadio::Dispose();
    ASSERT(iCacheSession == NULL);
}

TBool ServiceRadioNetwork::OnSubscribe()
{
    TUint id = IdCache::Hash(kCacheIdPrefixRadio, Device().Udn());
    iCacheSession = iNetwork.IdCache().CreateSession(id, MakeFunctorGeneric<ReadListData*>(*this, &ServiceRadioNetwork::ReadList));

    auto snapshot = new RadioSnapshot(iNetwork, *iCacheSession, new vector<TUint>(), *this);
    iMediaSupervisor = new MediaSupervisor<IMediaPreset*>(iNetwork, snapshot);
    iService->Subscribe();
    iSubscribed = true;
    return(false); // false = not mock
}

void ServiceRadioNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->iCancelled = true;
    }
*/
}

void ServiceRadioNetwork::HandleInitialEvent()
{
    TUint channelsMax;
    iService->PropertyChannelsMax(channelsMax);
    iChannelsMax = channelsMax;

    Brhz protocolInfo;
    iService->PropertyProtocolInfo(protocolInfo);
    iProtocolInfo.Replace(protocolInfo);

    //if (!iSubscribedSource->iCancelled)
    //{
        SubscribeCompleted();
    //}
}

void ServiceRadioNetwork::OnUnsubscribe()
{
    if (iService != NULL)
    {
        iService->Unsubscribe();
    }

    if (iMediaSupervisor != NULL)
    {
        iMediaSupervisor->Dispose();
    }


    if (iCacheSession != NULL)
    {
        iCacheSession->Dispose();
    }

    iSubscribed = false;
    //iSubscribedSource = NULL;
}

void ServiceRadioNetwork::Play()
{
    FunctorAsync f;
    iService->BeginPlay(f);

}

void ServiceRadioNetwork::Pause()
{
    FunctorAsync f;
    iService->BeginPause(f);
}

void ServiceRadioNetwork::Stop()
{
    FunctorAsync f;
    iService->BeginStop(f);
}

void ServiceRadioNetwork::SeekSecondAbsolute(TUint aValue)
{
    FunctorAsync f;
    iService->BeginSeekSecondAbsolute(aValue, f);
}

void ServiceRadioNetwork::SeekSecondRelative(TInt aValue)
{
    FunctorAsync f;
    iService->BeginSeekSecondRelative(aValue, f);
}

void ServiceRadioNetwork::SetId(TUint aId, const Brx& aUri)
{
    FunctorAsync f;
    iService->BeginSetId(aId, aUri, f);
}

void ServiceRadioNetwork::SetChannel(const Brx& aUri, IMediaMetadata& aMetadata)
{
    Bwh didlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, didlLite);
    FunctorAsync f;
    iService->BeginSetChannel(aUri, didlLite, f);
}



void ServiceRadioNetwork::ReadList(ReadListData* aReadListData)
{
    // called by IdCacheSession::CreateJobCallback - iFunction(payload);

    auto requiredIds = aReadListData->iMissingIds;

    Bwh idList;
    for (TUint i=0;i<requiredIds->size(); i++)
    {
        if (i>0)
        {
            idList.Append(Brn(" "));
        }
        Ascii::AppendDec(idList, (*requiredIds)[i]);
    }

    AsyncAdaptor& asyncAdaptor = iNetwork.GetAsyncAdaptorManager().GetAdaptor();
    auto f = MakeFunctorGeneric<AsyncCbArg*>(*this, &ServiceRadioNetwork::ReadListCallback);
    asyncAdaptor.SetCallback(f, aReadListData);
    FunctorAsync fa = asyncAdaptor.AsyncCb();

    iService->BeginReadList(idList, fa);
}


void ServiceRadioNetwork::ReadListCallback(AsyncCbArg* aArg)
{
    Brh channelList;
    iService->EndReadList(*aArg->iAsync, channelList);

    ReadListData* readListData = (ReadListData*)aArg->iArg;

    auto entries = new vector<IIdCacheEntry*>();

    // Parse XML here and populate entries
    auto requiredIds = readListData->iMissingIds;
    for(TUint i=0; i<requiredIds->size(); i++)
    {
        TUint id = (*requiredIds)[i];

        if (id > 0)
        {

            Bwh xmlNodeName;
            xmlNodeName.Replace(Brn("/ChannelList/Entry[Id="));
            Ascii::AppendDec(xmlNodeName, id);
            xmlNodeName.Append(Brn("]/Metadata"));

            Brn innerText = XmlParserBasic::Find(xmlNodeName, channelList);

            IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(innerText);

            auto mvs = metadata->Values();
            auto mv = mvs[iNetwork.GetTagManager().Audio().Uri()];
            Brn uri = mv->Value();
            entries->push_back(new IdCacheEntry(metadata, uri));

        }
    }

    readListData->iEntries = entries;
    readListData->iCallback(readListData);
}


void ServiceRadioNetwork::HandleIdChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleIdChangedCallback1);
    Schedule(f, NULL);
}


void ServiceRadioNetwork::HandleIdChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleIdChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceRadioNetwork::HandleIdChangedCallback2(void*)
{
    if (iSubscribed)
    {
        TUint id;
        iService->PropertyId(id);
        iId->Update(id);
    }
}


void ServiceRadioNetwork::HandleIdArrayChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleIdArrayChangedCallback1);
    Schedule(f, NULL);
}

void ServiceRadioNetwork::HandleIdArrayChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleIdArrayChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceRadioNetwork::HandleIdArrayChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brh idArrayStr;
        iService->PropertyIdArray(idArrayStr);

        auto idArray = new vector<TUint>();
        IdCache::UnpackIdArray(idArrayStr, *idArray);

        auto idNonZeroItems = IdCache::NonZeroItems(*idArray);
        iCacheSession->SetValid(idNonZeroItems);
        iMediaSupervisor->Update(new RadioSnapshot(iNetwork, *iCacheSession, idArray, *this));
    }
}



void ServiceRadioNetwork::HandleMetadataChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleMetadataChangedCallback1);
    Schedule(f, NULL);
}


void ServiceRadioNetwork::HandleMetadataChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleMetadataChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServiceRadioNetwork::HandleMetadataChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz metaDataStr;
        iService->PropertyMetadata(metaDataStr);

        IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metaDataStr);

        Brhz uri;
        iService->PropertyUri(uri);

        iMetadata->Update(new InfoMetadata(metadata, Brn(uri)));
    }
}


void ServiceRadioNetwork::HandleTransportStateChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleTransportStateChangedCallback1);
    Schedule(f, NULL);
}


void ServiceRadioNetwork::HandleTransportStateChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServiceRadioNetwork::HandleTransportStateChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServiceRadioNetwork::HandleTransportStateChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz transportState;
        iService->PropertyTransportState(transportState);

        Bws<100>* oldTransportState = iCurrentTransportState;
        iCurrentTransportState = new Bws<100>(transportState);
        iTransportState->Update(Brn(*iCurrentTransportState));
        delete oldTransportState;
    }
}


////////////////////////////////////////////////////////////////////////



RadioSnapshot::RadioSnapshot(INetwork& aNetwork, IIdCacheSession& aCacheSession, vector<TUint>* aIdArray, ServiceRadio& aRadio)
    :iNetwork(aNetwork)
    ,iCacheSession(aCacheSession)
    ,iIdArray(aIdArray)
    ,iRadio(aRadio)
{
}

TUint RadioSnapshot::Total()
{
    //return ((TUint)iIdArray.Where(v => v != 0).Count());
    auto idNonZeroItems = IdCache::NonZeroItems(*iIdArray);
    return(idNonZeroItems.size());
}


vector<TUint>* RadioSnapshot::Alpha()
{
    return(NULL);
}


void RadioSnapshot::Read(/*CancellationToken aCancellationToken,*/TUint aIndex, TUint aCount, FunctorGeneric<vector<IMediaPreset*>*> aCallback)
{
    ASSERT((aIndex + aCount) <= Total());

    auto idList = new vector<TUint>();
    TUint limit = aIndex + aCount;
    for (TUint i = aIndex; i < limit; i++)
    {
        TUint id = (*iIdArray)[i];
        if (id!=0)
        {
            idList->push_back(id);
        }
    }

    auto readEntriesdata = new ReadEntriesData();
    readEntriesdata->iIndex = aIndex;
    readEntriesdata->iRequestedIds = idList;
    readEntriesdata->iPresetsCallback = aCallback;
    readEntriesdata->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &RadioSnapshot::ReadCallback1);

    iCacheSession.Entries(readEntriesdata);
}


void RadioSnapshot::ReadCallback1(ReadEntriesData* aObj)
{
    auto f = MakeFunctorGeneric(*this, &RadioSnapshot::ReadCallback2);
    iNetwork.Schedule(f, aObj);
}

void RadioSnapshot::ReadCallback2(void* aObj)
{
    ReadEntriesData* data = (ReadEntriesData*)aObj;

    TUint index = data->iIndex;
    auto callback = data->iPresetsCallback;
    auto entries = data->iRetrievedEntries;
    delete data;


    if (entries == NULL)
    {
        callback(NULL);
    }
    else
    {
        auto presets = new vector<IMediaPreset*>();

        vector<TUint> idArray;
        for (TUint x=0; x<iIdArray->size(); x++)
        {
            TUint id = (*iIdArray)[x];
            if(id!=0)
            {
                idArray.push_back(id);
            }
        }

        for (TUint i=0; i<entries->size(); i++)
        {
            IIdCacheEntry* e = (*entries)[i];

            TUint id = idArray[index];

            auto it = find(iIdArray->begin(), iIdArray->end(), id);
            ASSERT(it!=iIdArray->end());
            TUint idIndex = it-iIdArray->begin();

            presets->push_back(new MediaPresetRadio(iNetwork, (idIndex + 1), id, e->Metadata(), e->Uri(), iRadio));
            index++;
        }

        callback(presets);
    }
}

//////////////////////////////////////////////////////////////////////////////////

ProxyRadio::ProxyRadio(ServiceRadio& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice)
{
}

IWatchable<TUint>& ProxyRadio::Id()
{
    return iService.Id();
}

IWatchable<Brn>& ProxyRadio::TransportState()
{
    return iService.TransportState();
}

IWatchable<IInfoMetadata*>& ProxyRadio::Metadata()
{
    return iService.Metadata();
}

TUint ProxyRadio::ChannelsMax()
{
    return iService.ChannelsMax();
}

const Brx& ProxyRadio::ProtocolInfo()
{
    return iService.ProtocolInfo();
}

void ProxyRadio::Play()
{
    return iService.Play();
}

void ProxyRadio::Pause()
{
    return iService.Pause();
}

void ProxyRadio::Stop()
{
    return iService.Stop();
}

void ProxyRadio::SeekSecondAbsolute(TUint aValue)
{
    return iService.SeekSecondAbsolute(aValue);
}

void ProxyRadio::SeekSecondRelative(TInt aValue)
{
    return iService.SeekSecondRelative(aValue);
}

void ProxyRadio::SetId(TUint aId, const Brx& aUri)
{
    return iService.SetId(aId, aUri);
}

void ProxyRadio::SetChannel(const Brx& aUri, IMediaMetadata& aMetadata)
{
    return iService.SetChannel(aUri, aMetadata);
}

IWatchable<IWatchableSnapshot<IMediaPreset*>*>& ProxyRadio::Snapshot()
{
    return iService.Snapshot();
}

void ProxyRadio::Dispose()
{
    iService.Unsubscribe();
}


IDevice& ProxyRadio::Device()
{
    return (iDevice);
}
