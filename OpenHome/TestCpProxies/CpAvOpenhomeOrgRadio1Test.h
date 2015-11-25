#pragma once

#include <Generated/CpAvOpenhomeOrgRadio1.h>
#include <OpenHome/Net/Private/AsyncPrivate.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/Functor.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Types.h>
namespace OpenHome {
namespace Topology{


class CpProxyAvOpenhomeOrgRadio1Test : public ICpProxyAvOpenhomeOrgRadio1
{
public:
    void SyncPlay() override;
    void BeginPlay(FunctorAsync& aFunctor) override;
    void EndPlay(IAsync& aAsync) override;
    void SyncPause() override;
    void BeginPause(FunctorAsync& aFunctor) override;
    void EndPause(IAsync& aAsync) override;
    void SyncStop() override;
    void BeginStop(FunctorAsync& aFunctor) override;
    void EndStop(IAsync& aAsync) override;
    void SyncSeekSecondAbsolute(TUint aValue) override;
    void BeginSeekSecondAbsolute(TUint aValue, FunctorAsync& aFunctor) override;
    void EndSeekSecondAbsolute(IAsync& aAsync) override;
    void SyncSeekSecondRelative(TInt aValue) override;
    void BeginSeekSecondRelative(TInt aValue, FunctorAsync& aFunctor) override;
    void EndSeekSecondRelative(IAsync& aAsync) override;
    void SyncChannel(Brh& aUri, Brh& aMetadata) override;
    void BeginChannel(FunctorAsync& aFunctor) override;
    void EndChannel(IAsync& aAsync, Brh& aUri, Brh& aMetadata) override;
    void SyncSetChannel(const Brx& aUri, const Brx& aMetadata) override;
    void BeginSetChannel(const Brx& aUri, const Brx& aMetadata, FunctorAsync& aFunctor) override;
    void EndSetChannel(IAsync& aAsync) override;
    void SyncTransportState(Brh& aValue) override;
    void BeginTransportState(FunctorAsync& aFunctor) override;
    void EndTransportState(IAsync& aAsync, Brh& aValue) override;
    void SyncId(TUint& aValue) override;
    void BeginId(FunctorAsync& aFunctor) override;
    void EndId(IAsync& aAsync, TUint& aValue) override;
    void SyncSetId(TUint aValue, const Brx& aUri) override;
    void BeginSetId(TUint aValue, const Brx& aUri, FunctorAsync& aFunctor) override;
    void EndSetId(IAsync& aAsync) override;
    void SyncRead(TUint aId, Brh& aMetadata) override;
    void BeginRead(TUint aId, FunctorAsync& aFunctor) override;
    void EndRead(IAsync& aAsync, Brh& aMetadata) override;
    void SyncReadList(const Brx& aIdList, Brh& aChannelList) override;
    void BeginReadList(const Brx& aIdList, FunctorAsync& aFunctor) override;
    void EndReadList(IAsync& aAsync, Brh& aChannelList) override;
    void SyncIdArray(TUint& aToken, Brh& aArray) override;
    void BeginIdArray(FunctorAsync& aFunctor) override;
    void EndIdArray(IAsync& aAsync, TUint& aToken, Brh& aArray) override;
    void SyncIdArrayChanged(TUint aToken, TBool& aValue) override;
    void BeginIdArrayChanged(TUint aToken, FunctorAsync& aFunctor) override;
    void EndIdArrayChanged(IAsync& aAsync, TBool& aValue) override;
    void SyncChannelsMax(TUint& aValue) override;
    void BeginChannelsMax(FunctorAsync& aFunctor) override;
    void EndChannelsMax(IAsync& aAsync, TUint& aValue) override;
    void SyncProtocolInfo(Brh& aValue) override;
    void BeginProtocolInfo(FunctorAsync& aFunctor) override;
    void EndProtocolInfo(IAsync& aAsync, Brh& aValue) override;
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
  private:
    TBool CheckPlaying();
    void ResetPlaying();
    TBool CheckPaused();
    void ResetPaused();
    TBool CheckStopped();
    void ResetStopped();
    TBool CheckSeconds();
    void ResetSeconds();
  private:
    TBool iPlaying;
    TBool iPaused;
    TBool iStopped;
    TUint iSeconds;
    Brh iState;
    TUint iId;
    Brh iUri;
    Brh iMetadata;
    TBool iArrayChanged;
    Brh iChannelList;
    TUint iToken;
    Brh iArray;
    TBool iArrayChanged;
    TUint iMaxChannels;
    Brh iProtocolInfo;

};

class AsyncTest : public IAsync
{
public:
  AsyncTest();
  ~AsyncTest();
  void Output(IAsyncOutput& aConsole) override;
};

}
}
