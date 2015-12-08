#pragma once

#include <Generated/CpAvOpenhomeOrgRadio1.h>
#include <OpenHome/Net/Private/AsyncPrivate.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Types.h>
#include <OpenHome/TestCpProxies/ResultRecorder.h>

#include <memory>

namespace OpenHome {
namespace Topology{


class CpProxyAvOpenhomeOrgRadio1Test : public Net::ICpProxyAvOpenhomeOrgRadio1
{
public:
  enum class RadioEvent : TUint
  {
    eSyncPlay = 0,
    eBeginPlay = 1,
    eEndPlay,
    eSyncPause,
    eBeginPause,
    eEndPause,
    eSyncStop,
    eBeginStop,
    eEndStop,
    eSyncSeekSecondAbsolute,
    eBeginSeekSecondAbsolute,
    eEndSeekSecondAbsolute,
    eSyncSeekSecondRelative,
    eBeginSeekSecondRelative,
    eEndSeekSecondRelative,
    eSyncChannel,
    eBeginChannel,
    eEndChannel,
    eSyncSetChannel,
    eBeginSetChannel,
    eEndSetChannel,
    eSyncTransportState,
    eBeginTransportState,
    eEndTransportState,
    eSyncId,
    eBeginId,
    eEndId,
    eSyncSetId,
    eBeginSetId,
    eEndSetId,
    eSyncRead,
    eBeginRead,
    eEndRead,
    eSyncReadList,
    eBeginReadList,
    eEndReadList,
    eSyncIdArray,
    eBeginIdArray,
    eEndIdArray,
    eSyncIdArrayChanged,
    eBeginIdArrayChanged,
    eEndIdArrayChanged,
    eSyncChannelsMax,
    eBeginChannelsMax,
    eEndChannelsMax,
    eSyncProtocolInfo,
    eBeginProtocolInfo,
    eEndProtocolInfo,
    eSetPropertyUriChanged,
    ePropertyUri,
    eSetPropertyMetadataChanged,
    ePropertyMetadata,
    eSetPropertyTransportStateChanged,
    ePropertyTransportState,
    eSetPropertyIdChanged,
    ePropertyId,
    eSetPropertyIdArrayChanged,
    ePropertyIdArray,
    eSetPropertyChannelsMaxChanged,
    ePropertyChannelsMax,
    eSetPropertyProtocolInfoChanged,
    ePropertyProtocolInfo,
    eSubscribe,
    eUnsubscribe,
    eSetPropertyChanged,
    eSetPropertyInitialEvent,
    eAddProperty,
    eDestroyService,
    eReportEvent,
    eVersion
  };
public:
    CpProxyAvOpenhomeOrgRadio1Test(std::shared_ptr<ResultRecorder<RadioEvent>> aRecorder);
    ~CpProxyAvOpenhomeOrgRadio1Test();
public: //ICpProxyAvOpenhomeOrgRadio1
    void SyncPlay() override;
    void BeginPlay(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndPlay(OpenHome::Net::IAsync& aAsync) override;
    void SyncPause() override;
    void BeginPause(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndPause(OpenHome::Net::IAsync& aAsync) override;
    void SyncStop() override;
    void BeginStop(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndStop(OpenHome::Net::IAsync& aAsync) override;
    void SyncSeekSecondAbsolute(TUint aValue) override;
    void BeginSeekSecondAbsolute(TUint aValue, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndSeekSecondAbsolute(OpenHome::Net::IAsync& aAsync) override;
    void SyncSeekSecondRelative(TInt aValue) override;
    void BeginSeekSecondRelative(TInt aValue, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndSeekSecondRelative(OpenHome::Net::IAsync& aAsync) override;
    void SyncChannel(Brh& aUri, Brh& aMetadata) override;
    void BeginChannel(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndChannel(OpenHome::Net::IAsync& aAsync, Brh& aUri, Brh& aMetadata) override;
    void SyncSetChannel(const Brx& aUri, const Brx& aMetadata) override;
    void BeginSetChannel(const Brx& aUri, const Brx& aMetadata, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndSetChannel(OpenHome::Net::IAsync& aAsync) override;
    void SyncTransportState(Brh& aValue) override;
    void BeginTransportState(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndTransportState(OpenHome::Net::IAsync& aAsync, Brh& aValue) override;
    void SyncId(TUint& aValue) override;
    void BeginId(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndId(OpenHome::Net::IAsync& aAsync, TUint& aValue) override;
    void SyncSetId(TUint aValue, const Brx& aUri) override;
    void BeginSetId(TUint aValue, const Brx& aUri, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndSetId(OpenHome::Net::IAsync& aAsync) override;
    void SyncRead(TUint aId, Brh& aMetadata) override;
    void BeginRead(TUint aId, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndRead(OpenHome::Net::IAsync& aAsync, Brh& aMetadata) override;
    void SyncReadList(const Brx& aIdList, Brh& aChannelList) override;
    void BeginReadList(const Brx& aIdList, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndReadList(OpenHome::Net::IAsync& aAsync, Brh& aChannelList) override;
    void SyncIdArray(TUint& aToken, Brh& aArray) override;
    void BeginIdArray(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndIdArray(OpenHome::Net::IAsync& aAsync, TUint& aToken, Brh& aArray) override;
    void SyncIdArrayChanged(TUint aToken, TBool& aValue) override;
    void BeginIdArrayChanged(TUint aToken, OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndIdArrayChanged(OpenHome::Net::IAsync& aAsync, TBool& aValue) override;
    void SyncChannelsMax(TUint& aValue) override;
    void BeginChannelsMax(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndChannelsMax(OpenHome::Net::IAsync& aAsync, TUint& aValue) override;
    void SyncProtocolInfo(Brh& aValue) override;
    void BeginProtocolInfo(OpenHome::Net::FunctorAsync& aFunctor) override;
    void EndProtocolInfo(OpenHome::Net::IAsync& aAsync, Brh& aValue) override;
    void SetPropertyUriChanged(Functor& aUriChanged) override;
    void PropertyUri(Brhz& aUri) const override;
    void SetPropertyMetadataChanged(Functor& aMetadataChanged) override;
    void PropertyMetadata(Brhz& aMetadata) const override;
    void SetPropertyTransportStateChanged(Functor& aTransportStateChanged) override;
    void PropertyTransportState(Brhz& aTransportState) const override;
    void SetPropertyIdChanged(Functor& aIdChanged) override;
    void PropertyId(TUint& aId) const override;
    void SetPropertyIdArrayChanged(Functor& aIdArrayChanged) override;
    void PropertyIdArray(Brh& aIdArray) const override;
    void SetPropertyChannelsMaxChanged(Functor& aChannelsMaxChanged) override;
    void PropertyChannelsMax(TUint& aChannelsMax) const override;
    void SetPropertyProtocolInfoChanged(Functor& aProtocolInfoChanged) override;
    void PropertyProtocolInfo(Brhz& aProtocolInfo) const override;
    void Subscribe() override;
    void Unsubscribe() override;
    void SetPropertyChanged(Functor& aFunctor) override;
    void SetPropertyInitialEvent(Functor& aFunctor) override;
    void AddProperty(Net::Property* aProperty) override;
    void DestroyService() override;
    void ReportEvent(Functor aFunctor) override;
    TUint Version() const override;
  private:
    std::shared_ptr<ResultRecorder<RadioEvent>> iRecorder;
    std::unique_ptr<OpenHome::Net::IAsync> iAsync;

};

class AsyncTest : public OpenHome::Net::IAsync
{
public:
  AsyncTest();
  ~AsyncTest();
  void Output(OpenHome::Net::IAsyncOutput& aConsole) override;
};

}
}
