#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/CpProxy.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Media.h>
#include <OpenHome/IdCache.h>
#include <OpenHome/AsyncAdaptor.h>

#include <vector>
#include <memory>

namespace OpenHome
{

namespace Net
{
    class ICpProxyAvOpenhomeOrgRadio1;
    class ICpProxy;
}

namespace Topology
{

class ServiceRadio;

/////////////////////////////////////////////////////////

class MediaPresetRadio : public IMediaPreset, public IWatcher<TUint>, public IWatcher<Brn>
{
public:
    MediaPresetRadio(IWatchableThread& aThread, TUint aIndex, TUint aId, IMediaMetadata& aMetadata, const Brx& aUri, ServiceRadio& aRadio);
    ~MediaPresetRadio();

    void Dispose();
    TUint Index();
    IMediaMetadata& Metadata();
    IWatchable<TBool>& Buffering();
    IWatchable<TBool>& Playing();
    IWatchable<TBool>& Selected();
    void Play();
    void ItemOpen(const Brx& aId, TUint aValue);
    void ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious);
    void ItemClose(const Brx& aId, TUint aValue);
    void ItemOpen(const Brx& aId, Brn aValue);
    void ItemUpdate(const Brx& aId, Brn, Brn aPrevious);
    void ItemClose(const Brx& aId, Brn aValue);

private:
    void EvaluatePlaying();

private:
    TUint iIndex;
    TUint iId;
    IMediaMetadata& iMetadata;
    Bws<1000> iUri;  // FIXME: random capacity
    ServiceRadio& iRadio;
    Watchable<TBool>* iBuffering;
    Watchable<TBool>* iPlaying;
    Watchable<TBool>* iSelected;
    TUint iCurrentId;
    Bws<100> iCurrentTransportState;
};

//////////////////////////////////////////////////////////////////

class IProxyRadio : public IProxy
{
    virtual IWatchable<TUint>& Id() = 0;
    virtual IWatchable<Brn>& TransportState() = 0;
    virtual IWatchable<IInfoMetadata*>& Metadata() = 0;
    virtual IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot() = 0;

    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void SeekSecondAbsolute(TUint aValue) = 0;
    virtual void SeekSecondRelative(TInt aValue) = 0;
    virtual void SetId(TUint aId, const Brx& aUri) = 0;
    virtual void SetChannel(const Brx& aUri, IMediaMetadata& aMetadata) = 0;

    virtual TUint ChannelsMax() = 0;
    virtual const Brx& ProtocolInfo() = 0;
};

//////////////////////////////////////////////////////////////////

class ServiceRadio : public Service
{
protected:
    ServiceRadio(IInjectorDevice& aDevice, ILog& aLog);
    ~ServiceRadio();

public:
    virtual void Dispose();
    virtual IProxy* OnCreate(IDevice& aDevice);
    IWatchable<TUint>& Id();
    IWatchable<Brn>& TransportState();
    IWatchable<IInfoMetadata*>& Metadata();
    IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot();
    TUint ChannelsMax();
    const Brx& ProtocolInfo();

    virtual void Play() = 0;
    virtual void Pause() = 0;
    virtual void Stop() = 0;
    virtual void SeekSecondAbsolute(TUint aValue) = 0;
    virtual void SeekSecondRelative(TInt aValue) = 0;
    virtual void SetId(TUint aId, const Brx& aUri) = 0;
    virtual void SetChannel(const Brx& aUri, IMediaMetadata& aMetadata) = 0;

protected:
    Bws<1000> iProtocolInfo;
    Watchable<TUint>* iId;
    Watchable<Brn>* iTransportState;
    Watchable<IInfoMetadata*>* iMetadata;
    std::unique_ptr<MediaSupervisor<IMediaPreset*>> iMediaSupervisor;
    Bws<100>* iCurrentTransportState;
    TUint iChannelsMax;
};

//////////////////////////////////////////////////////////////////

class ServiceRadioNetwork : public ServiceRadio
{
public:
    ServiceRadioNetwork(IInjectorDevice& aDevice, std::unique_ptr<Net::ICpProxyAvOpenhomeOrgRadio1> aService, ILog& aLog); //ICpProxyAvOpenhomeOrgRadio1* and ICpProxy* must be wrapped in shared_ptr
    ~ServiceRadioNetwork();

    void Dispose();
    void Play();
    void Pause();
    void Stop();
    void SeekSecondAbsolute(TUint aValue);
    void SeekSecondRelative(TInt aValue);
    void SetId(TUint aId, const Brx& aUri);
    void SetChannel(const Brx& aUri, IMediaMetadata& aMetadata);

protected:
    virtual TBool OnSubscribe();
    virtual void OnCancelSubscribe();
    virtual void OnUnsubscribe();

private:
    void ReadList(ReadListData* aReadListData);
    void ReadListCallback(AsyncCbArg* aArg);

    void HandleInitialEvent();
    void HandleIdChanged();
    void HandleIdArrayChanged();
    void HandleMetadataChanged();
    void HandleTransportStateChanged();

    void HandleIdChangedCallback1(void*);
    void HandleIdChangedCallback2(void*);
    void HandleIdArrayChangedCallback1(void*);
    void HandleIdArrayChangedCallback2(void*);
    void HandleMetadataChangedCallback1(void*);
    void HandleMetadataChangedCallback2(void*);
    void HandleTransportStateChangedCallback1(void*);
    void HandleTransportStateChangedCallback2(void*);

private:
    std::unique_ptr<Net::ICpProxyAvOpenhomeOrgRadio1> iService;
    std::unique_ptr<IIdCacheSession> iCacheSession;
    TBool iSubscribed;
};
//////////////////////////////////////////////////////////////////
class ServiceRadioMock : public ServiceRadio
{
public:
    ServiceRadioMock(IInjectorDevice& aDevice, TUint aId, std::vector<IMediaMetadata*>& aPresets, IInfoMetadata* aInfoMetadata, const Brx& aProtocolInfo, const Brx& aTransportState, TUint aChannelsMax, ILog& aLog);
    ~ServiceRadioMock();
public:
    void Dispose() override;
    TBool OnSubscribe() override;
    void OnUnsubscribe() override;
    void Play() override;
    void CallbackPlay(void*);

    void Pause() override;
    void CallbackPause(void*);

    void Stop() override;
    void CallbackStop(void*);
    void SeekSecondAbsolute(TUint aValue) override;
    void SeekSecondRelative(TInt aValue) override;
    void SetId(TUint aId, const Brx& aValue) override;
    void CallbackSetId(void* aValue);
    void SetChannel(const Brx& aUri, IMediaMetadata& aMetadata) override;
    void CallbackSetChannel(void* aValue);
    void Execute(ICommandTokens& aValue) override;
private:
    void ReadList(ReadListData* aIdList);
private:
    std::unique_ptr<IIdCacheSession> iCacheSession;
    std::vector<IMediaMetadata*>& iPresets;
    std::vector<TUint>* iIdArray;
};
//////////////////////////////////////////////////////////////////

class RadioSnapshot : public IMediaClientSnapshot<IMediaPreset*>, public INonCopyable
{
public:
    RadioSnapshot(INetwork& aNetwork, IIdCacheSession& aCacheSession, std::vector<TUint>* aIdArray, ServiceRadio& aRadio);
    ~RadioSnapshot();
    TUint Total();
    std::vector<TUint>* Alpha();
    void Read(/*CancellationToken aCancellationToken,*/ TUint aIndex, TUint aCount, FunctorGeneric<std::vector<IMediaPreset*>*> aCallback);

private:
    void ReadCallback1(ReadEntriesData* aObj);
    void ReadCallback2(void* aObj);


private:
    INetwork& iNetwork;
    IIdCacheSession& iCacheSession;
    std::unique_ptr<std::vector<TUint>> iIdArray;
    ServiceRadio& iRadio;
};

//////////////////////////////////////////////////////////////////

class ProxyRadio : public IProxyRadio, public INonCopyable
{
public:
    ProxyRadio(ServiceRadio& aService, IDevice& aDevice);

    IWatchable<TUint>& Id();
    IWatchable<Brn>& TransportState();
    IWatchable<IInfoMetadata*>& Metadata();

    TUint ChannelsMax();
    const Brx& ProtocolInfo();

    void Play();
    void Pause();
    void Stop();
    void SeekSecondAbsolute(TUint aValue);
    void SeekSecondRelative(TInt aValue);
    void SetId(TUint aId, const Brx& aUri);
    void SetChannel(const Brx& aUri, IMediaMetadata& aMetadata);

    IWatchable<IWatchableSnapshot<IMediaPreset*>*>& Snapshot();

    // IProxy
    virtual IDevice& Device();

    // IDisposable
    virtual void Dispose();

protected:
    ServiceRadio& iService;

private:
    IDevice& iDevice;
};


} // Topology
} // OpenHome
