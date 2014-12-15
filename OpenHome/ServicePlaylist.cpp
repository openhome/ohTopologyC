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
using namespace OpenHome::Av;
using namespace OpenHome::Net;
using namespace std;



using namespace OpenHome;
using namespace OpenHome::Av;


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

ServicePlaylist::ServicePlaylist(INetwork& aNetwork, IInjectorDevice& aDevice, ILog& aLog)
    :Service(aNetwork, aDevice, aLog)
    ,iId(new Watchable<TUint>(aNetwork, Brn("Id"), 0))
    ,iInfoNext(new Watchable<IInfoMetadata*>(aNetwork, Brn("InfoNext"), InfoMetadata::Empty()))
    ,iTransportState(new Watchable<Brn>(aNetwork, Brn("TransportState"), Brx::Empty()))
    ,iRepeat(new Watchable<TBool>(aNetwork, Brn("Repeat"), false))
    ,iShuffle(new Watchable<TBool>(aNetwork, Brn("Shuffle"), true))
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



ServicePlaylistNetwork::ServicePlaylistNetwork(INetwork& aNetwork, IInjectorDevice& aDevice, CpDevice& aCpDevice, ILog& aLog)
    :ServicePlaylist(aNetwork, aDevice, aLog)
    ,iCpDevice(aCpDevice)
{
    iCpDevice.AddRef();

    iService = new CpProxyAvOpenhomeOrgPlaylist1(aCpDevice);

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

    //iService->Dispose();
    //iService = NULL;

    iCpDevice.RemoveRef();
}


TBool ServicePlaylistNetwork::OnSubscribe()
{
    TUint id = IdCache::Hash(kCacheIdPrefixPlaylist, Device().Udn());

    iCacheSession = iNetwork.IdCache().CreateSession(id, MakeFunctorGeneric<ReadListData*>(*this, &ServicePlaylistNetwork::ReadList));
    iMediaSupervisor = new MediaSupervisor<IMediaPreset*>(iNetwork, new PlaylistSnapshot(iNetwork, *iCacheSession, new vector<TUint>(), *this));

    iService->Subscribe();
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

}


void ServicePlaylistNetwork::Play()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPlay(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginPlay((ptr) =>
    {
        try
        {
            iService.EndPlay(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Pause()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPause(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginPause((ptr) =>
    {
        try
        {
            iService->EndPause(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Stop()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginStop(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginStop((ptr) =>
    {
        try
        {
            iService.EndStop(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Previous()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginPrevious(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginPrevious((ptr) =>
    {
        try
        {
            iService.EndPrevious(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Next()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginNext(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService.BeginNext((ptr) =>
    {
        try
        {
            iService.EndNext(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::SeekId(TUint aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekId(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginSeekId(aValue, (ptr) =>
    {
        try
        {
            iService.EndSeekId(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::SeekSecondAbsolute(TUint aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekSecondAbsolute(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginSeekSecondAbsolute(aValue, (ptr) =>
    {
        try
        {
            iService.EndSeekSecondAbsolute(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::SeekSecondRelative(TInt aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSeekSecondRelative(aValue, f);

/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginSeekSecondRelative(aValue, (ptr) =>
    {
        try
        {
            iService.EndSeekSecondRelative(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata)
{
    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, metadataDidlLite);

    FunctorAsync f;
    iService->BeginInsert(aAfterId, aUri, metadataDidlLite, f);

/*
    TaskCompletionSource<TUint> taskSource = new TaskCompletionSource<TUint>();
    iService->BeginInsert(aAfterId, aUri, iNetwork.TagManager.ToDidlLite(aMetadata), (ptr) =>
    {
        try
        {
            TUint newId;
            iService.EndInsert(ptr, out newId);
            taskSource.SetResult(newId);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::InsertNext(const Brx& aUri, IMediaMetadata& aMetadata)
{
    TUint id;
    iService->PropertyId(id);

    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, metadataDidlLite);


    FunctorAsync f;
    iService->BeginInsert(id, aUri, metadataDidlLite, f);

/*
    TUint id = iService.PropertyId();

    TaskCompletionSource<TUint> taskSource = new TaskCompletionSource<TUint>();
    iService->BeginInsert(id, aUri, iNetwork.TagManager.ToDidlLite(aMetadata), (ptr) =>
    {
        try
        {
            TUint newId;
            iService.EndInsert(ptr, out newId);
            taskSource.SetResult(newId);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata)
{
    TUint id = 0;

    Brh idArrayBuf;
    iService->PropertyIdArray(idArrayBuf);

    vector<TUint> idArray;
    IdCache::UnpackIdArray(idArrayBuf, idArray);

    if (idArray.size() > 0)
    {
        id = idArray[idArray.size()-1]; // last element
    }

    Bwh metadataDidlLite;
    iNetwork.GetTagManager().ToDidlLite(aMetadata, metadataDidlLite);

    FunctorAsync f;
    iService->BeginInsert(id, aUri, metadataDidlLite, f);
/*


    IList<TUint> idArray = ByteArray.Unpack(iService.PropertyIdArray());
    if (idArray.Count > 0)
    {
        id = idArray.Last();
    }

    TaskCompletionSource<TUint> taskSource = new TaskCompletionSource<TUint>();
    iService->BeginInsert(id, aUri, iNetwork.TagManager.ToDidlLite(aMetadata), (ptr) =>
    {
        try
        {
            TUint newId;
            iService.EndInsert(ptr, out newId);
            taskSource.SetResult(newId);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::Delete(IMediaPreset& aValue)
{
//    ASSERT(aValue is MediaPresetPlaylist);
    TUint id = ((MediaPresetPlaylist&)aValue).Id();

    FunctorAsync f;  // null functor (won't get called)
    iService->BeginDeleteId(id, f);

/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginDeleteId(id, (ptr) =>
    {
        try
        {
            iService.EndDeleteId(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::DeleteAll()
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginDeleteAll(f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginDeleteAll((ptr) =>
    {
        try
        {
            iService.EndDeleteAll(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::SetRepeat(TBool aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSetRepeat(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginSetRepeat(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetRepeat(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::SetShuffle(TBool aValue)
{
    FunctorAsync f;  // null functor (won't get called)
    iService->BeginSetShuffle(aValue, f);
/*
    TaskCompletionSource<TBool> taskSource = new TaskCompletionSource<TBool>();
    iService->BeginSetShuffle(aValue, (ptr) =>
    {
        try
        {
            iService.EndSetShuffle(ptr);
            taskSource.SetResult(true);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });
    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
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
        idList.Append(Brn("{"));
        Ascii::AppendDec(idList, (*requiredIds)[i]);
        idList.Append(Brn("}"));
    }

    AsyncAdaptor& asyncAdaptor = iNetwork.GetAsyncAdaptorManager().GetAdaptor();

    auto f = MakeFunctorGeneric<AsyncCbArg*>(*this, &ServicePlaylistNetwork::ReadListCallback);
    asyncAdaptor.SetCallback(f, aReadListData);
    FunctorAsync fa = asyncAdaptor.AsyncCb();

    iService->BeginReadList(idList, fa);

/*
    TaskCompletionSource<IEnumerable<IIdCacheEntry>> taskSource = new TaskCompletionSource<IEnumerable<IIdCacheEntry>>();

    string idList = string.Empty;
    foreach (TUint id in aIdList)
    {
        idList += string.Format("{0} ", id);
    }
    idList.Trim(' ');

    iService.BeginReadList(idList, (ptr) =>
    {
        try
        {
            string trackList;
            iService.EndReadList(ptr, out trackList);

            List<IIdCacheEntry> entries = new List<IIdCacheEntry>();

            XmlDocument document = new XmlDocument();
            document.LoadXml(trackList);

            XmlNodeList list = document.SelectNodes("/TrackList/Entry");
            foreach (XmlNode n in list)
            {
                IMediaMetadata metadata = iNetwork.TagManager.FromDidlLite(n["Metadata"].InnerText);
                string uri = n["Uri"].InnerText;
                entries.Add(new IdCacheEntry(metadata, uri));
            }

            taskSource.SetResult(entries);
        }
        catch (Exception e)
        {
            taskSource.SetException(e);
        }
    });

    taskSource.Task.ContinueWith(t => { iLog.Write("Unobserved exception: {0}\n", t.Exception); }, TaskContinuationOptions.OnlyOnFaulted);
    return taskSource.Task;
*/
}


void ServicePlaylistNetwork::ReadListCallback(AsyncCbArg* aArg)
{
    Brh trackList;
    iService->EndReadList(*aArg->iAsync, trackList);

    ReadListData* readListData = (ReadListData*)aArg->iArg;

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
    readListData->iCallback(readListData);

/*
    XmlDocument document = new XmlDocument();
    document.LoadXml(trackList);

    XmlNodeList list = document.SelectNodes("/TrackList/Entry");
    foreach (XmlNode n in list)
    {
        IMediaMetadata* metadata = iNetwork.GetTagManager().FromDidlLite(n["Metadata"].InnerText);
        string uri = n["Uri"].InnerText;
        entries.Add(new IdCacheEntry(metadata, uri));
    }


    taskSource.SetResult(entries);
*/




/*
    iService.BeginReadList(idList, (ptr) =>
    {
        string trackList;
        iService.EndReadList(ptr, out trackList);

        List<IIdCacheEntry> entries = new List<IIdCacheEntry>();

        XmlDocument document = new XmlDocument();
        document.LoadXml(trackList);

        XmlNodeList list = document.SelectNodes("/TrackList/Entry");
        foreach (XmlNode n in list)
        {
            IMediaMetadata metadata = iNetwork.TagManager.FromDidlLite(n["Metadata"].InnerText);
            string uri = n["Uri"].InnerText;
            entries.Add(new IdCacheEntry(metadata, uri));
        }

        taskSource.SetResult(entries);
    });
*/
}


void ServicePlaylistNetwork::HandleIdChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    IList<TUint> idArray = ByteArray.Unpack(iService.PropertyIdArray());
    TUint id = iService.PropertyId();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iId.Update(id);
            EvaluateInfoNext(id, idArray);
        });
    });
*/
}


void ServicePlaylistNetwork::HandleIdChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleIdChangedCallback2(void*)
{
    Brh idArrayBuf;
    iService->PropertyIdArray(idArrayBuf);

    vector<TUint> idArray;
    IdCache::UnpackIdArray(idArrayBuf, idArray);

    TUint id;
    iService->PropertyId(id);
    iId->Update(id);
    EvaluateInfoNext(id, idArray);
}

void ServicePlaylistNetwork::HandleIdArrayChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdArrayChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    IList<TUint> idArray = ByteArray.Unpack(iService.PropertyIdArray());
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iCacheSession.SetValid(idArray);
            iMediaSupervisor.Update(new PlaylistSnapshot(iNetwork, iCacheSession, idArray, this));
            EvaluateInfoNext(iId.Value, idArray);
        });
    });
*/
}

void ServicePlaylistNetwork::HandleIdArrayChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleIdArrayChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleIdArrayChangedCallback2(void*)
{
    Brh idArrayBuf;
    iService->PropertyIdArray(idArrayBuf);

    vector<TUint>* idArray = new vector<TUint>();
    IdCache::UnpackIdArray(idArrayBuf, *idArray);

    iCacheSession->SetValid(*idArray);
    iMediaSupervisor->Update(new PlaylistSnapshot(iNetwork, *iCacheSession, idArray, *this));
    EvaluateInfoNext(iId->Value(), *idArray);
}



void ServicePlaylistNetwork::EvaluateInfoNext(TUint aId, vector<TUint>& aIdArray)
{
    auto it = find(aIdArray.begin(), aIdArray.end(), aId);

    TUint index = it - aIdArray.begin();
    if ( (it!=aIdArray.end()) && (index < (aIdArray.size()-1)) )
    {
        auto readEntriesData = new ReadEntriesData();
        readEntriesData->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback1);
        readEntriesData->iRequestedIds = new vector<TUint>();
        readEntriesData->iRequestedIds->push_back(aIdArray[index+1]);
        iCacheSession->Entries(readEntriesData);
    }
    else
    {
        iInfoNext->Update(InfoMetadata::Empty());
    }

/*
            int index = aIdArray.IndexOf(aId);
            if ((index > -1) && (index < aIdArray.Count - 1) && (aIdArray.Count > 1))
            {
                iCacheSession.Entries(new uint[] { aIdArray.ElementAt(index + 1) }).ContinueWith((t) =>
                {
                    iNetwork.Schedule(() =>
                    {
                        iDisposeHandler.WhenNotDisposed(() =>
                        {
                            IIdCacheEntry entry = t.Result.ElementAt(0);
                            iInfoNext.Update(new InfoMetadata(entry.Metadata, entry.Uri));
                        });
                    });
                });
            }
            else
            {
                iInfoNext.Update(InfoMetadata.Empty);
            }
*/
}

void ServicePlaylistNetwork::EvaluateInfoNextCallback1(ReadEntriesData* aReadEntriesData)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback2);
    iNetwork.Schedule(f, aReadEntriesData);
}

void ServicePlaylistNetwork::EvaluateInfoNextCallback2(void* aReadEntriesData)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::EvaluateInfoNextCallback3);
    iDisposeHandler->WhenNotDisposed(f, aReadEntriesData);
}

void ServicePlaylistNetwork::EvaluateInfoNextCallback3(void* aReadEntriesData)
{
    auto readEntriesData = (ReadEntriesData*)aReadEntriesData;

    //IIdCacheEntry entry = t.Result.ElementAt(0);
    IIdCacheEntry* entry = (*readEntriesData->iRetrievedEntries)[0];
    iInfoNext->Update(new InfoMetadata(&entry->Metadata(), entry->Uri()));

}


void ServicePlaylistNetwork::HandleTransportStateChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleTransportStateChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    string transportState = iService.PropertyTransportState();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iTransportState.Update(transportState);
        });
    });
*/
}


void ServicePlaylistNetwork::HandleTransportStateChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleTransportStateChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleTransportStateChangedCallback2(void*)
{
    Brhz transportState;
    iService->PropertyTransportState(transportState);
    iTransportState->Update(Brn(transportState));
}


void ServicePlaylistNetwork::HandleRepeatChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleRepeatChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    TBool repeat = iService.PropertyRepeat();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iRepeat.Update(repeat);
        });
    });
*/
}


void ServicePlaylistNetwork::HandleRepeatChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleRepeatChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}


void ServicePlaylistNetwork::HandleRepeatChangedCallback2(void*)
{
    TBool repeat;
    iService->PropertyRepeat(repeat);
    iRepeat->Update(repeat);
}

void ServicePlaylistNetwork::HandleShuffleChanged()
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleShuffleChangedCallback1);
    iNetwork.Schedule(f, NULL);
/*
    TBool shuffle = iService.PropertyShuffle();
    iNetwork.Schedule(() =>
    {
        iDisposeHandler.WhenNotDisposed(() =>
        {
            iShuffle.Update(shuffle);
        });
    });
*/
}

void ServicePlaylistNetwork::HandleShuffleChangedCallback1(void*)
{
    auto f = MakeFunctorGeneric(*this, &ServicePlaylistNetwork::HandleShuffleChangedCallback2);
    iDisposeHandler->WhenNotDisposed(f, NULL);
}

void ServicePlaylistNetwork::HandleShuffleChangedCallback2(void*)
{
    TBool shuffle;
    iService->PropertyShuffle(shuffle);
    iShuffle->Update(shuffle);
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
    for (TUint i = aIndex; i < aIndex + aCount; ++i)
    {
        idList->push_back((*iIdArray)[i]);
    }

    auto readEntriesdata = new ReadEntriesData();
    readEntriesdata->iIndex = aIndex;
    readEntriesdata->iRequestedIds = idList;
    readEntriesdata->iPresetsCallback = aCallback;
    readEntriesdata->iEntriesCallback = MakeFunctorGeneric<ReadEntriesData*>(*this, &PlaylistSnapshot::ReadCallback1);

    iCacheSession.Entries(readEntriesdata);

/*
    iNetwork.Schedule(() =>
    {
        if (!aCancellationToken.IsCancellationRequested)
        {
            TUint index = aIndex;
            foreach (IIdCacheEntry e in entries)
            {
                TUint id = iIdArray.ElementAt((TInt)index);
                tracks.Add(new MediaPresetPlaylist(iNetwork, (TUint)(iIdArray.IndexOf(id) + 1), id, e.Metadata, iPlaylist));
                ++index;
            }

            aCallback(tracks);
        }
    });
*/
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
    auto entries = new vector<IIdCacheEntry*>();
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


/*
    uint index = aIndex;
    foreach (IIdCacheEntry e in entries)
    {
        uint id = iIdArray.ElementAt((int)index);
        tracks.Add(new MediaPresetPlaylist(iNetwork, (uint)(iIdArray.IndexOf(id) + 1), id, e.Metadata, iPlaylist));
        ++index;
    }

    aCallback(tracks);
*/


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

