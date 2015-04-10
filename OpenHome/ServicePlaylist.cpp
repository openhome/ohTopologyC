#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/ServicePlaylist.h>
#include <OpenHome/Network.h>
#include <OpenHome/AsyncAdaptor.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <Generated/CpAvOpenhomeOrgPlaylist1.h>
#include <vector>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace std;


MediaPresetPlaylist::MediaPresetPlaylist(IWatchableThread& aThread, TUint aIndex, TUint aId, IMediaMetadata& aMetadata, ServicePlaylist& aPlaylist)
    :iIndex(aIndex)
    ,iId(aId)
    ,iMetadata(aMetadata)
    ,iPlaylist(aPlaylist)
    ,iBuffering(new Watchable<TBool>(aThread, Brn("Buffering"), false))
    ,iPlaying(new Watchable<TBool>(aThread, Brn("Playing"), false))
    ,iSelected(new Watchable<TBool>(aThread, Brn("Selected"), false))
{
    iPlaylist.Id().AddWatcher(*this);
    iPlaylist.TransportState().AddWatcher(*this);
}

MediaPresetPlaylist::~MediaPresetPlaylist()
{
    delete iBuffering;
    delete iPlaying;
    delete iSelected;
}

void MediaPresetPlaylist::Dispose()
{
    iPlaylist.Id().RemoveWatcher(*this);
    iPlaylist.TransportState().RemoveWatcher(*this);
    iBuffering->Dispose();
    iPlaying->Dispose();
    iSelected->Dispose();
}

TUint MediaPresetPlaylist::Index()
{
    return iIndex;
}

TUint MediaPresetPlaylist::Id()
{
    return iId;
}

IMediaMetadata& MediaPresetPlaylist::Metadata()
{
    return iMetadata;
}

IWatchable<TBool>& MediaPresetPlaylist::Buffering()
{
    return (*iBuffering);
}

IWatchable<TBool>& MediaPresetPlaylist::Playing()
{
    return (*iPlaying);
}

IWatchable<TBool>& MediaPresetPlaylist::Selected()
{
    return (*iSelected);
}

void MediaPresetPlaylist::Play()
{
    iBuffering->Update(iCurrentTransportState == Brn("Buffering"));
    iPlaylist.SeekId(iId);
}

void MediaPresetPlaylist::EvaluatePlaying()
{
    iBuffering->Update((iCurrentId == iId) && (iCurrentTransportState == Brn("Buffering")));
    iPlaying->Update((iCurrentId == iId) && (iCurrentTransportState == Brn("Playing")));
    iSelected->Update(iCurrentId == iId);
}

void MediaPresetPlaylist::ItemOpen(const Brx& /*aId*/, TUint aValue)
{
    iCurrentId = aValue;
    EvaluatePlaying();
}

void MediaPresetPlaylist::ItemUpdate(const Brx& /*aId*/, TUint aValue, TUint /*aPrevious*/)
{
    iCurrentId = aValue;
    EvaluatePlaying();
}

void MediaPresetPlaylist::ItemClose(const Brx& /*aId*/, TUint /*aValue*/)
{
    iPlaying->Update(false);
}

void MediaPresetPlaylist::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iCurrentTransportState.Replace(aValue);
    EvaluatePlaying();
}

void MediaPresetPlaylist::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iCurrentTransportState.Replace(aValue);
    EvaluatePlaying();
}

void MediaPresetPlaylist::ItemClose(const Brx& /*aId*/, Brn /*aPrevious*/)
{
    iPlaying->Update(false);
}

//////////////////////////////////////////////////////////////////////////////

ServicePlaylist::ServicePlaylist(IInjectorDevice& aDevice, ILog& aLog)
    :Service(aDevice, aLog)
    ,iId(new Watchable<TUint>(iNetwork, Brn("Id"), 0))
    ,iInfoNext(new Watchable<IInfoMetadata*>(iNetwork, Brn("InfoNext"), iNetwork.InfoMetadataEmpty()))
    ,iInfoCurrentIndex(new Watchable<TInt>(iNetwork, Brn("CurrentIndex"), -1))
    ,iTransportState(new Watchable<Brn>(iNetwork, Brn("TransportState"), Brx::Empty()))
    ,iRepeat(new Watchable<TBool>(iNetwork, Brn("Repeat"), false))
    ,iShuffle(new Watchable<TBool>(iNetwork, Brn("Shuffle"), true))
{
}

void ServicePlaylist::Dispose()
{
    Service::Dispose();
    iId->Dispose();
    iInfoNext->Dispose();
    iTransportState->Dispose();
    iRepeat->Dispose();
    iShuffle->Dispose();
}


IProxy* ServicePlaylist::OnCreate(IDevice& aDevice)
{
    return(new ProxyPlaylist(*this, aDevice));
}


IWatchable<TUint>& ServicePlaylist::Id()
{
    return(*iId);
}


IWatchable<IInfoMetadata*>& ServicePlaylist::InfoNext()
{
    return(*iInfoNext);
}


IWatchable<TInt>& ServicePlaylist::InfoCurrentIndex()
{
    return(*iInfoCurrentIndex);
}


IWatchable<Brn>& ServicePlaylist::TransportState()
{
    return(*iTransportState);
}


IWatchable<TBool>& ServicePlaylist::Repeat()
{
    return(*iRepeat);
}


IWatchable<TBool>& ServicePlaylist::Shuffle()
{
    return(*iShuffle);
}


IWatchable<IWatchableSnapshot<IMediaPreset*>*>& ServicePlaylist::Snapshot()
{
    return iMediaSupervisor->Snapshot();
}


TUint ServicePlaylist::TracksMax()
{
    return iTracksMax;
}


const Brx& ServicePlaylist::ProtocolInfo()
{
    return iProtocolInfo;
}


//////////////////////////////////////////////////////////////////////////////



ServicePlaylistNetwork::ServicePlaylistNetwork(IInjectorDevice& aDevice, CpProxyAvOpenhomeOrgPlaylist1* aService, ILog& aLog)
    :ServicePlaylist(aDevice, aLog)
    ,iService(aService)
    ,iSubscribed(false)
{
    Functor f1 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleIdChanged);
    iService->SetPropertyIdChanged(f1);

    Functor f2 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleIdArrayChanged);
    iService->SetPropertyIdArrayChanged(f2);

    Functor f3 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleTransportStateChanged);
    iService->SetPropertyTransportStateChanged(f3);

    Functor f4 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleRepeatChanged);
    iService->SetPropertyRepeatChanged(f4);

    Functor f5 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleShuffleChanged);
    iService->SetPropertyShuffleChanged(f5);

    Functor f6 = MakeFunctor(*this, &ServicePlaylistNetwork::HandleInitialEvent);
    iService->SetPropertyInitialEvent(f6);
}



ServicePlaylistNetwork::~ServicePlaylistNetwork()
{
}


void ServicePlaylistNetwork::Dispose()
{
    ServicePlaylist::Dispose();
    ASSERT(iCacheSession == NULL);
}


TBool ServicePlaylistNetwork::OnSubscribe()
{
    TUint id = IdCache::Hash(kCacheIdPrefixPlaylist, Device().Udn());

    iCacheSession = iNetwork.IdCache().CreateSession(id, MakeFunctorGeneric<ReadListData*>(*this, &ServicePlaylistNetwork::ReadList));
    iMediaSupervisor = new MediaSupervisor<IMediaPreset*>(iNetwork, new PlaylistSnapshot(iNetwork, *iCacheSession, new vector<TUint>(), *this));

    iService->Subscribe();
    iSubscribed = true;

    return(false); // false = not mock
}


void ServicePlaylistNetwork::OnCancelSubscribe()
{
/*
    if (iSubscribedSource != NULL)
    {
        iSubscribedSource->iCancelled = true;
    }
*/
}


void ServicePlaylistNetwork::HandleInitialEvent()
{
    iService->PropertyTracksMax(iTracksMax);

    Brhz protocolInfo;
    iService->PropertyProtocolInfo(protocolInfo);
    iProtocolInfo.Replace(protocolInfo);

//    if (!iSubscribedSource->iCancelled)
//    {
        SubscribeCompleted();
//    }
}


void ServicePlaylistNetwork::OnUnsubscribe()
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
}


void ServicePlaylistNetwork::Play()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPlay(f);
}


void ServicePlaylistNetwork::Pause()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPause(f);
}


void ServicePlaylistNetwork::Stop()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginStop(f);
}


void ServicePlaylistNetwork::Previous()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPrevious(f);

}


void ServicePlaylistNetwork::Next()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginNext(f);
}


void ServicePlaylistNetwork::SeekId(TUint aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekId(aValue, f);
}


void ServicePlaylistNetwork::SeekSecondAbsolute(TUint aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekSecondAbsolute(aValue, f);
}


void ServicePlaylistNetwork::SeekSecondRelative(TInt aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekSecondRelative(aValue, f);
}


void ServicePlaylistNetwork::Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata)
{
    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, metadataDidlLite);

    FunctorAsync f;
    iService->BeginInsert(aAfterId, aUri, metadataDidlLite, f);
}


void ServicePlaylistNetwork::Insert(IMediaPreset& aMediaPreset, const Brx& aUri, IMediaMetadata& aMetadata)
{
    MediaPresetPlaylist& preset = (MediaPresetPlaylist&)aMediaPreset;
    TUint id = preset.Id();
    Insert(id, aUri, aMetadata);
}


void ServicePlaylistNetwork::InsertNext(const Brx& aUri, IMediaMetadata& aMetadata)
{
    TUint id;
    iService->PropertyId(id);

    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, metadataDidlLite);

    FunctorAsync f;
    iService->BeginInsert(id, aUri, metadataDidlLite, f);
}


void ServicePlaylistNetwork::InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata)
{
    auto playlistItemData = new PlaylistItemData();
    playlistItemData->iUri = Brn(aUri);
    playlistItemData->iMetadata = &aMetadata;

    AsyncAdaptor& asyncAdaptor = iNetwork.GetAsyncAdaptorManager().GetAdaptor();
    auto f = MakeFunctorGeneric<AsyncCbArg*>(*this, &ServicePlaylistNetwork::BeginIdArrayCallback);
    asyncAdaptor.SetCallback(f, playlistItemData);
    FunctorAsync fa = asyncAdaptor.AsyncCb();
    iService->BeginIdArray(fa);

}


void ServicePlaylistNetwork::BeginIdArrayCallback(AsyncCbArg* aArg)
{
    TUint token;
    Brh idArrayBuf;
    Net::IAsync* ptr = aArg->iAsync;
    iService->EndIdArray(*ptr, token, idArrayBuf);

    vector<TUint> idArray;
    IdCache::UnpackIdArray(idArrayBuf, idArray);

    TUint id = 0;
    if (idArray.size() > 0)
    {
        id = idArray[idArray.size()-1]; // last element
    }

    PlaylistItemData* playlistItemData = (PlaylistItemData*)(aArg->iArg);
    Brn uri = playlistItemData->iUri;
    IMediaMetadata* metadata = playlistItemData->iMetadata;
    delete playlistItemData;
    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(*metadata, metadataDidlLite);

    FunctorAsync f;
    iService->BeginInsert(id, uri, metadataDidlLite, f);
}


void ServicePlaylistNetwork::MakeRoomForInsert(TUint aCount)
{
    vector<TUint> idArray;
    IdArray(idArray);

    vector<TUint> ids(idArray.begin(), idArray.begin()+aCount);
    Delete(ids);
}


void ServicePlaylistNetwork::Delete(vector<TUint>& aIds)
{
    for(TUint i=0; i<aIds.size(); i++)
    {
        TUint id = aIds[i];
        Delete(id);
    }
}

void ServicePlaylistNetwork::Delete(IMediaPreset& aValue)
{
    TUint id = ((MediaPresetPlaylist&)aValue).Id();
    Delete(id);
}


void ServicePlaylistNetwork::Delete(TUint aId)
{
    FunctorAsync f;  // no callback
    iService->BeginDeleteId(aId, f);
}


void ServicePlaylistNetwork::DeleteAll()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginDeleteAll(f);
}


void ServicePlaylistNetwork::SetRepeat(TBool aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSetRepeat(aValue, f);
}


void ServicePlaylistNetwork::SetShuffle(TBool aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSetShuffle(aValue, f);
}


void ServicePlaylistNetwork::ReadList(ReadListData* aReadListData)
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

    auto f = MakeFunctorGeneric<AsyncCbArg*>(*this, &ServicePlaylistNetwork::ReadListCallback);
    asyncAdaptor.SetCallback(f, aReadListData);
    FunctorAsync fa = asyncAdaptor.AsyncCb();

    iService->BeginReadList(idList, fa);

}


void ServicePlaylistNetwork::ReadListCallback(AsyncCbArg* aArg)
{
    ReadListData* readListData = (ReadListData*)aArg->iArg;

    try
    {
        Brh trackList;
        iService->EndReadList(*aArg->iAsync, trackList);

        auto entries = new vector<IIdCacheEntry*>();

        // Parse XML here and populate entries
        Brn xmlNodeList = XmlParserBasic::Find(Brn("TrackList"), trackList);
        Brn remaining = xmlNodeList;
        while(!remaining.Equals(Brx::Empty()))
        {
            Brn metadataText = XmlParserBasic::Find(Brn("Metadata"), xmlNodeList, remaining);
            Brn uriText = XmlParserBasic::Find(Brn("Uri"), xmlNodeList, remaining);
            xmlNodeList = remaining;
            IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(metadataText);
            entries->push_back(new IdCacheEntry(metadata, uriText));
        }

        readListData->iEntries = entries;
    }
    catch(...)
    {
        readListData->iEntries = NULL;
    }

    readListData->iCallback(readListData);

}


void ServicePlaylistNetwork::HandleIdChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdChangedCallback1);
    Schedule(f, NULL);
}


void ServicePlaylistNetwork::HandleIdChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleIdChangedCallback2(void*)
{
    vector<TUint> idArray;
    IdArray(idArray);

    TUint id;
    iService->PropertyId(id);
    if (iSubscribed)
    {
        iId->Update(id);
        EvaluateInfoCurrentIndex(id, idArray);
        EvaluateInfoNext(id, idArray);
    }
}

void ServicePlaylistNetwork::HandleIdArrayChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdArrayChangedCallback1);
    Schedule(f, NULL);
}

void ServicePlaylistNetwork::HandleIdArrayChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdArrayChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleIdArrayChangedCallback2(void*)
{
    vector<TUint>* idArray = new vector<TUint>();
    IdArray(*idArray);

    if (iSubscribed)
    {
        iCacheSession->SetValid(*idArray);
        iMediaSupervisor->Update(new PlaylistSnapshot(iNetwork, *iCacheSession, idArray, *this));
        EvaluateInfoCurrentIndex(iId->Value(), *idArray);
        EvaluateInfoNext(iId->Value(), *idArray);
    }
}


void ServicePlaylistNetwork::EvaluateInfoCurrentIndex(TUint aId, vector<TUint>& aIdArray)
{
    auto it = find(aIdArray.begin(), aIdArray.end(), aId);
    TUint index = it - aIdArray.begin();
    iInfoCurrentIndex->Update(index);
}

void ServicePlaylistNetwork::EvaluateInfoNext(TUint aId, vector<TUint>& aIdArray)
{
    auto it = find(aIdArray.begin(), aIdArray.end(), aId);
    TUint index = it - aIdArray.begin();

// if (!iShuffle.Value && (index > -1) && (index < aIdArray.Count - 1) && (aIdArray.Count > 1))

    if ((!iShuffle->Value()) && (it!=aIdArray.end()) && (index < (aIdArray.size()-1)) )
    {
        auto readEntriesData = new ReadEntriesData();
        readEntriesData->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback1);
        readEntriesData->iRequestedIds = new vector<TUint>();
        readEntriesData->iRequestedIds->push_back(aIdArray[index+1]);
        iCacheSession->Entries(readEntriesData);
    }
    // if (!iShuffle.Value && iRepeat.Value && (index > -1) && index == aIdArray.Count - 1)
    else if ((!iShuffle->Value()) && (iRepeat->Value()) && (it!=aIdArray.end()) && (index == (aIdArray.size()-1)) )
    {
        auto readEntriesData = new ReadEntriesData();
        readEntriesData->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback1);
        readEntriesData->iRequestedIds = new vector<TUint>();
        readEntriesData->iRequestedIds->push_back(aIdArray[0]);
        iCacheSession->Entries(readEntriesData);
    }
    else
    {
        iInfoNext->Update(iNetwork.InfoMetadataEmpty());
    }

}

void ServicePlaylistNetwork::EvaluateInfoNextCallback1(ReadEntriesData* aReadEntriesData)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback2);
    Schedule(f, aReadEntriesData);
}

void ServicePlaylistNetwork::EvaluateInfoNextCallback2(void* aReadEntriesData)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback3);
    iDisposeHandler->WhenNotDisposed(f, aReadEntriesData);
}

void ServicePlaylistNetwork::EvaluateInfoNextCallback3(void* aReadEntriesData)
{
    auto readEntriesData = (ReadEntriesData*)aReadEntriesData;

    try
    {
        IIdCacheEntry* entry = (*readEntriesData->iRetrievedEntries)[0];
        iInfoNext->Update(new InfoMetadata(&entry->Metadata(), entry->Uri()));
    }
    catch(...)
    {
        iInfoNext->Update(iNetwork.InfoMetadataEmpty());
    }
}


void ServicePlaylistNetwork::HandleTransportStateChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleTransportStateChangedCallback1);
    Schedule(f, NULL);
}


void ServicePlaylistNetwork::HandleTransportStateChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleTransportStateChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleTransportStateChangedCallback2(void*)
{
    if (iSubscribed)
    {
        Brhz transportState;
        iService->PropertyTransportState(transportState);
        iTransportState->Update(Brn(transportState));
    }
}


void ServicePlaylistNetwork::HandleRepeatChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleRepeatChangedCallback1);
    Schedule(f, NULL);
}


void ServicePlaylistNetwork::HandleRepeatChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleRepeatChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleRepeatChangedCallback2(void*)
{
    if (iSubscribed)
    {
        vector<TUint> idArray;
        IdArray(idArray);

        TBool repeat;
        iService->PropertyRepeat(repeat);
        iRepeat->Update(repeat);
        EvaluateInfoNext(iId->Value(), idArray);
    }
}


void ServicePlaylistNetwork::IdArray(vector<TUint>& aIdArray)
{
    Brh idArrayBuf;
    iService->PropertyIdArray(idArrayBuf);
    IdCache::UnpackIdArray(idArrayBuf, aIdArray);
}


void ServicePlaylistNetwork::HandleShuffleChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleShuffleChangedCallback1);
    Schedule(f, NULL);
}

void ServicePlaylistNetwork::HandleShuffleChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleShuffleChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServicePlaylistNetwork::HandleShuffleChangedCallback2(void*)
{
    if (iSubscribed)
    {
        vector<TUint> idArray;
        IdArray(idArray);
        TBool shuffle;
        iService->PropertyShuffle(shuffle);
        iShuffle->Update(shuffle);
        EvaluateInfoNext(iId->Value(), idArray);
    }
}


////////////////////////////////////////////////////////////////////////////////////


PlaylistSnapshot::PlaylistSnapshot(INetwork& aNetwork, IIdCacheSession& aCacheSession, vector<TUint>* aIdArray, ServicePlaylist& aPlaylist)
    :iNetwork(aNetwork)
    ,iCacheSession(aCacheSession)
    ,iIdArray(aIdArray)
    ,iPlaylist(aPlaylist)
{
}


TUint PlaylistSnapshot::Total()
{
    return (iIdArray->size());
}



vector<TUint>* PlaylistSnapshot::Alpha()
{
    return NULL;
}




void PlaylistSnapshot::Read(/*CancellationToken aCancellationToken,*/ TUint aIndex, TUint aCount, FunctorGeneric<vector<IMediaPreset*>*> aCallback)
{
    ASSERT((aIndex + aCount) <= Total());

    auto idList = new vector<TUint>();
    for (TUint i = aIndex; i < aIndex + aCount; i++)
    {
        idList->push_back((*iIdArray)[i]);
    }

    auto readEntriesdata = new ReadEntriesData();
    readEntriesdata->iIndex = aIndex;
    readEntriesdata->iRequestedIds = idList;
    readEntriesdata->iPresetsCallback = aCallback;
    readEntriesdata->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &PlaylistSnapshot::ReadCallback1);

    iCacheSession.Entries(readEntriesdata);
}


void PlaylistSnapshot::ReadCallback1(ReadEntriesData* aReadEntriesData)
{
    auto f = MakeFunctorGeneric(*this, &PlaylistSnapshot::ReadCallback2);
    iNetwork.Schedule(f, aReadEntriesData);
}


void PlaylistSnapshot::ReadCallback2(void* aObj)
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
        auto tracks = new vector<IMediaPreset*>();

        for (TUint i=0; i<entries->size(); i++)
        {
            IIdCacheEntry* e = (*entries)[i];

            TUint id = (*iIdArray)[index];

            auto it = find(iIdArray->begin(), iIdArray->end(), id);
            ASSERT(it!=iIdArray->end());
            TUint idIndex = it-iIdArray->begin();

            tracks->push_back(new MediaPresetPlaylist(iNetwork, (idIndex + 1), id, e->Metadata(), iPlaylist));
            index++;
        }

        callback(tracks);
    }
}

////////////////////////////////////////////////////////////////////////////////

ProxyPlaylist::ProxyPlaylist(ServicePlaylist& aService, IDevice& aDevice)
    :iService(aService)
    ,iDevice(aDevice){
}


IWatchable<TUint>& ProxyPlaylist::Id()
{
    return iService.Id();
}

IWatchable<TInt>& ProxyPlaylist::InfoCurrentIndex()
{
    return iService.InfoCurrentIndex();
}

IWatchable<IInfoMetadata*>& ProxyPlaylist::InfoNext()
{
     return iService.InfoNext();
}


IWatchable<Brn>& ProxyPlaylist::TransportState()
{
    return iService.TransportState();
}


IWatchable<TBool>& ProxyPlaylist::Repeat()
{
    return iService.Repeat();
}


IWatchable<TBool>& ProxyPlaylist::Shuffle()
{
    return iService.Shuffle();
}


TUint ProxyPlaylist::TracksMax()
{
    return iService.TracksMax();
}


const Brx& ProxyPlaylist::ProtocolInfo()
{
    return iService.ProtocolInfo();
}


void ProxyPlaylist::Play()
{
    return iService.Play();
}


void ProxyPlaylist::Pause()
{
    return iService.Pause();
}


void ProxyPlaylist::Stop()
{
    return iService.Stop();
}


void ProxyPlaylist::Previous()
{
    return iService.Previous();
}


void ProxyPlaylist::Next()
{
    return iService.Next();
}


void ProxyPlaylist::SeekId(TUint aValue)
{
    return iService.SeekId(aValue);
}


void ProxyPlaylist::SeekSecondAbsolute(TUint aValue)
{
    return iService.SeekSecondAbsolute(aValue);
}


void ProxyPlaylist::SeekSecondRelative(TInt aValue)
{
    return iService.SeekSecondRelative(aValue);
}


void ProxyPlaylist::Insert(IMediaPreset& aMediaPreset, const Brx& aUri, IMediaMetadata& aMetadata)
{
    return iService.Insert(aMediaPreset, aUri, aMetadata);
}


void ProxyPlaylist::Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata)
{
    return iService.Insert(aAfterId, aUri, aMetadata);
}


void ProxyPlaylist::InsertNext(const Brx& aUri, IMediaMetadata& aMetadata)
{
    return iService.InsertNext(aUri, aMetadata);
}


void ProxyPlaylist::InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata)
{
    return iService.InsertEnd(aUri, aMetadata);
}

void ProxyPlaylist::MakeRoomForInsert(TUint aCount)
{
    return iService.MakeRoomForInsert(aCount);
}

void ProxyPlaylist::Delete(IMediaPreset& aValue)
{
    return iService.Delete(aValue);
}


void ProxyPlaylist::DeleteAll()
{
    return iService.DeleteAll();
}


void ProxyPlaylist::SetRepeat(TBool aValue)
{
    return iService.SetRepeat(aValue);
}


void ProxyPlaylist::SetShuffle(TBool aValue)
{
    return iService.SetShuffle(aValue);
}


IWatchable<IWatchableSnapshot<IMediaPreset*>*>& ProxyPlaylist::Snapshot()
{
    return iService.Snapshot();
}

void ProxyPlaylist::Dispose()
{
    iService.Unsubscribe();
}


IDevice& ProxyPlaylist::Device()
{
    return (iDevice);
}
