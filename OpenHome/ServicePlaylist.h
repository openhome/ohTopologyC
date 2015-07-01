#ifndef HEADER_OHTOPC_SERVICE_PLAYLIST
#define HEADER_OHTOPC_SERVICE_PLAYLIST

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/AsyncAdaptor.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Media.h>
#include <OpenHome/IdCache.h>

#include <vector>
#include <memory>

namespace OpenHome
{
namespace Net
{
class CpProxyAvOpenhomeOrgPlaylist1;
}

namespace Topology
{

class ServicePlaylist;

class MediaPresetPlaylist : public IMediaPreset, public IWatcher<TUint>, public IWatcher<Brn>
{
public:
    MediaPresetPlaylist(IWatchableThread& aThread, TUint aIndex, TUint aId, IMediaMetadata& aMetadata, ServicePlaylist& aPlaylist);
    ~MediaPresetPlaylist();

    void Dispose();
    TUint Index();
    TUint Id();
    IMediaMetadata& Metadata();
    IWatchable<TBool>& Buffering();
    IWatchable<TBool>& Playing();
    IWatchable<TBool>& Selected();
    void Play();
    void ItemOpen(const Brx& aId, TUint aValue);
    void ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious);
    void ItemClose(const Brx& aId, TUint aValue);
    void ItemOpen(const Brx& aId, Brn aValue);
    void ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious);
    void ItemClose(const Brx& aId, Brn aValue);

private:
    void EvaluatePlaying();

private:
    TUint iIndex;
    TUint iId;
    IMediaMetadata& iMetadata;
    ServicePlaylist& iPlaylist;
    Watchable<TBool>* iBuffering;
    Watchable<TBool>* iPlaying;
    Watchable<TBool>* iSelected;
    TUint iCurrentId;
    Bws<100> iCurrentTransportState;
};

/////////////////////////////////////////////////////////////////////////////

struct PlaylistItemData
{
    Brn iUri;
    IMediaMetadata* iMetadata;
};

/////////////////////////////////////////////////////////////////////////////

class IProxyPlaylist : public IProxy
{
public:
    virtual IWatchable<TUint>& Id() = 0;
    virtual IWatchable<IInfoMetadata*>& InfoNext() = 0;
    virtual IWatchable<Brn>& TransportState() = 0;
    virtual IWatchable<TBool>& Repeat() = 0;
    virtual IWatchable<TBool>& Shuffle() = 0;

    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void Previous() = 0;
    virtual void Next() = 0;
    virtual void SeekId(TUint aValue) = 0;
    virtual void SeekSecondAbsolute(TUint aValue) = 0;
    virtual void SeekSecondRelative(TInt aValue) = 0;

    virtual void Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void InsertNext(const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void Delete(IMediaPreset& aValue) = 0;
    virtual void DeleteAll() = 0;
    virtual void SetRepeat(TBool aValue) = 0;
    virtual void SetShuffle(TBool aValue) = 0;

    virtual IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot() = 0;

    virtual TUint TracksMax() = 0;
    virtual const Brx& ProtocolInfo() = 0;
};

/////////////////////////////////////////////////////////////////////////////

class ServicePlaylist : public Service
{
protected:
    ServicePlaylist(IInjectorDevice& aDevice, ILog& aLog);

public:
    void Dispose();
    IProxy* OnCreate(IDevice& aDevice);
    IWatchable<TUint>& Id();
    IWatchable<IInfoMetadata*>& InfoNext();
    IWatchable<TInt>& InfoCurrentIndex();
    IWatchable<Brn>& TransportState();
    IWatchable<TBool>& Repeat();
    IWatchable<TBool>& Shuffle();
    IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot();
    TUint TracksMax();
    const Brx& ProtocolInfo();


    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void Previous() = 0;
    virtual void Next() = 0;
    virtual void SeekId(TUint aValue) = 0;
    virtual void SeekSecondAbsolute(TUint aValue) = 0;
    virtual void SeekSecondRelative(TInt aValue) = 0;
    virtual void Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void Insert(IMediaPreset& aMediaPreset, const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void InsertNext(const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata) = 0;
    virtual void MakeRoomForInsert(TUint aCount) = 0;
    virtual void Delete(IMediaPreset& aValue) = 0;
    virtual void DeleteAll() = 0;
    virtual void SetRepeat(TBool aValue) = 0;
    virtual void SetShuffle(TBool aValue) = 0;



protected:
    Bws<1000> iProtocolInfo;
    Watchable<TUint>* iId;
    Watchable<IInfoMetadata*>* iInfoNext;
    Watchable<TInt>* iInfoCurrentIndex;
    Watchable<Brn>* iTransportState;
    Watchable<TBool>* iRepeat;
    Watchable<TBool>* iShuffle;
    MediaSupervisor<IMediaPreset*>* iMediaSupervisor;
    TUint iTracksMax;
};

/////////////////////////////////////////////////////////////////////////////

class ServicePlaylistNetwork : public ServicePlaylist
{
public:
    ServicePlaylistNetwork(IInjectorDevice& aDevice, Net::CpProxyAvOpenhomeOrgPlaylist1* aService, ILog& aLog);
    ~ServicePlaylistNetwork();

    void Dispose();
    void Play();
    void Pause();
    void Stop();
    void Previous();
    void Next();
    void SeekId(TUint aValue);
    void SeekSecondAbsolute(TUint aValue);
    void SeekSecondRelative(TInt aValue);
    void Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata);
    void Insert(IMediaPreset& aMediaPreset, const Brx& aUri, IMediaMetadata& aMetadata);
    void InsertNext(const Brx& aUri, IMediaMetadata& aMetadata);
    void InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata);
    void Delete(IMediaPreset& aValue);
    void DeleteAll();
    void SetRepeat(TBool aValue);
    void SetShuffle(TBool aValue);
    void MakeRoomForInsert(TUint aCount);

protected:
    TBool OnSubscribe();
    void OnCancelSubscribe();
    void OnUnsubscribe();

    void ReadList(ReadListData* aReadListData);

    void HandleInitialEvent();
    void HandleIdChanged();
    void HandleIdArrayChanged();
    void EvaluateInfoNext(TUint aId, std::vector<TUint>& aIdArray);
    void HandleTransportStateChanged();
    void HandleRepeatChanged();
    void HandleShuffleChanged();


    void HandleIdChangedCallback1(void*);
    void HandleIdChangedCallback2(void*);
    void HandleIdArrayChangedCallback1(void*);
    void HandleIdArrayChangedCallback2(void*);
    void EvaluateInfoNextCallback1(ReadEntriesData* aReadEntriesData);
    void EvaluateInfoNextCallback2(void* aReadEntriesData);
    void EvaluateInfoNextCallback3(void* aReadEntriesData);
    void HandleTransportStateChangedCallback1(void*);
    void HandleTransportStateChangedCallback2(void*);
    void HandleRepeatChangedCallback1(void*);
    void HandleRepeatChangedCallback2(void*);
    void HandleShuffleChangedCallback1(void*);
    void HandleShuffleChangedCallback2(void*);

private:
    void ReadListCallback(AsyncCbArg* aArg);
    void BeginIdArrayCallback(AsyncCbArg* aArg);
    void Delete(std::vector<TUint>& aIds);
    void Delete(TUint);
    void IdArray(std::vector<TUint>& aIdArray);

    void EvaluateInfoCurrentIndex(TUint aId, std::vector<TUint>& aIdArray);

private:
    Net::CpProxyAvOpenhomeOrgPlaylist1* iService;
    IIdCacheSession* iCacheSession;
    TBool iSubscribed;
};

/////////////////////////////////////////////////////////////////////////////


class PlaylistSnapshot : public IMediaClientSnapshot<IMediaPreset*>, public INonCopyable
{
public:
    PlaylistSnapshot(INetwork& aNetwork, IIdCacheSession& aCacheSession, std::vector<TUint>* aIdArray, ServicePlaylist& aPlaylist);
    TUint Total();
    std::vector<TUint>* Alpha();
    void Read(/*CancellationToken aCancellationToken,*/ TUint aIndex, TUint aCount, FunctorGeneric<std::vector<IMediaPreset*>*> aCallback);

private:
    void ReadCallback1(ReadEntriesData* aReadEntriesData);
    void ReadCallback2(void* aObj);

private:
    INetwork& iNetwork;
    IIdCacheSession& iCacheSession;
    std::vector<TUint>* iIdArray;
    ServicePlaylist& iPlaylist;
};

/////////////////////////////////////////////////////////////////////////////

class ProxyPlaylist : public IProxyPlaylist, public INonCopyable
{
public:
    ProxyPlaylist(ServicePlaylist& aService, IDevice& aDevice);

    IWatchable<TUint>& Id();
    IWatchable<TInt>& InfoCurrentIndex();
    IWatchable<IInfoMetadata*>& InfoNext();
    IWatchable<Brn>& TransportState();
    IWatchable<TBool>& Repeat();
    IWatchable<TBool>& Shuffle();
    TUint TracksMax();
    const Brx& ProtocolInfo();
    void Play();
    void Pause();
    void Stop();
    void Previous();
    void Next();
    void SeekId(TUint aValue);
    void SeekSecondAbsolute(TUint aValue);
    void SeekSecondRelative(TInt aValue);
    void Insert(IMediaPreset& aMediaPreset, const Brx& aUri, IMediaMetadata& aMetadata);
    void Insert(TUint aAfterId, const Brx& aUri, IMediaMetadata& aMetadata);
    void InsertNext(const Brx& aUri, IMediaMetadata& aMetadata);
    void InsertEnd(const Brx& aUri, IMediaMetadata& aMetadata);
    void MakeRoomForInsert(TUint aCount);
    void Delete(IMediaPreset& aValue);
    void DeleteAll();
    void SetRepeat(TBool aValue);
    void SetShuffle(TBool aValue);
    IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot();

    // IProxyPlaylist
    virtual IDevice& Device();
    virtual void Dispose();

private:
    ServicePlaylist& iService;
    IDevice& iDevice;
};

} // Topology
} // OpenHome

#endif // HEADER_OHTOPC_SERVICE_PLAYLIST
