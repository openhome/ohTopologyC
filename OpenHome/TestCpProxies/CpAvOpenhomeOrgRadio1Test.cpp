#include <OpenHome/TestCpProxies/CpProxyAvOpenhomeOrgRadio1Test.h>

void CpProxyAvOpenhomeOrgRadio1Test::SyncPlay()
{
  iPlaying = true;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginPlay(FunctorAsync& aFunctor)
{
  iPlaying = true;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndPlay(IAsync& /*aAsync*/)
{
  //Retrieve arguments?
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncPause()
{
  iPaused = true;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginPause(FunctorAsync& aFunctor)
{
  iPaused = true;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndPlay(IAsync& /*aAsync*/)
{
  //Retrieve arguments
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncStop()
{
  iStopped = true;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginStop(FunctorAsync& aFunctor)
{
  iStopped = true;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndStop(IAsync& /*aAsync*/)
{
  //Retrieve arguments
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSeekSecondAbsolute(TUint aValue)
{
  iSeconds = aValue;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSeekSecondAbsolute(TUint aValue, FunctorAsync& aFunctor)
{
  iSeconds = aValue;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSeekSecondAbsolute(TUint aValue, IAsync& /*aAsync*/)
{
  //Retrieve arguments
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSeekSecondRelative(TInt aValue)
{
  iSeconds += aValue;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSeekSecondRelative(TInt aValue, FunctorAsync& aFunctor)
{
  iSeconds += aValue;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSeekSecondRelative(IAsync& aFunctor)
{
  //Retrieve arguments
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncChannel(Brh& aUri, Brh& aMetadata)
{
  aUri = iUri;
  aMetadata = iMetadata;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginChannel(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndChannel(IAsync& /*aFunctor*/, Brh& aUri, Brh& aMetadata)
{
  aUri = iUri;
  aMetadata = iMetadata;
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSetChannel(const Brx& aUri, const Brx& aMetadata)
{
  iUri = aUri;
  iMetadata = aMetadata;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSetChannel(const Brx& aUri, const Brx& aMetadata, FunctorAsync& aFunctor)
{
  iUri = aUri;
  iMetadata = aMetadata;
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSetChannel(IAsync& /*aAsync*/)
{

}

void CpProxyAvOpenhomeOrgRadio1Test::SyncTransportState(Brh& aValue)
{
  aValue = iState;
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginTransportState(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomeOrgRadio1Test::EndTransportState(IAsync& /*aAsync*/, Brh& aValue)
{
  aValue = iState;
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncId(TUint& aValue)
{
  aValue = iId;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginId(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndId(IAsync& /*aAsync*/, TUint& aValue)
{
  aValue = iId;
  //pass back args to async?
}

void CpProxyAvOpenhomOrgRadio1Test::SyncSetId(TUint aValue, const Brx& aUri)
{
  iId = aValue;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginSetId(TUint aValue, const Brx& aUri, FunctorAsync& aFunctor)
{
  iId = aValue;
  iUri = aUri;
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndSetId(IAsync& /*aAsync*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::SyncRead(TUint /*aId*/, Brh& aMetadata)
{
  aMetadata = iMetadata;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginRead(TUint /*aId*/, FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndRead(IAsync& /*aAsync*/, Brh& aMetadata)
{
  aMetadata = iMetadata;
}

void CpProxyAvOpenhomOrgRadio1Test::SyncReadList(const Brx& /*aIdList*/, Brh& aChannelList)
{
  aChannelList = iChannelList;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginReadList(const Brx& /*aIdList*/, FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndReadList(IAsync& /*aAsync*/, Brh& aChannelList)
{
  aChannelList = iChannelList;
}

void CpProxyAvOpenhomOrgRadio1Test::SyncIdArray(TUint& aToken, Brh& aArray)
{
  aToken = iToken;
  aArray = iArray;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginIdArray(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndIdArray(IAsync& /*aAsync*/, TUint& aToken, Brh& aArray)
{
  aToken = iToken;
  aArray = iArray;
}

void CpProxyAvOpenhomOrgRadio1Test::SyncIdArrayChanged(TUint /*aToken*/, TBool& aValue)
{
   aValue = iArrayChanged;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginIdArrayChanged(TUint /*aToken*/, FunctorAsync& aFunctor)
{
  aFuntor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndIdArrayChanged(IAsync& /*aAsync*/, TBool& aValue)
{
aValue = iArrayChanged;
}

void CpProxyAvOpenhomOrgRadio1Test::SyncChannelsMax(TUint& aValue)
{
  aValue = iMaxChannels;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginChannelsMax(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndChannelsMax(IAsync& /*aAsync*/, TUint& aValue)
{
  aValue = iMaxChannels;
}

void CpProxyAvOpenhomOrgRadio1Test::SyncProtocolInfo(Brh& aValue)
{
  aValue = iProtocolInfo;
}

void CpProxyAvOpenhomOrgRadio1Test::BeginProtocolInfo(FunctorAsync& aFunctor)
{
  aFunctor();
}

void CpProxyAvOpenhomOrgRadio1Test::EndProtocolInfo(IAsync& /*aAsync*/, Brh& aValue)
{
  aValue = iProtocolInfo;
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyUriChanged(Functor& aUriChanged)
{
  aUriChanged = iUriChanged;
}

void CpProxyAvOpenhomOrgRadio1Test::PropertyUri(Brhz& aUri) const
{
  aUri.Set(iUri);
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyMetadataChanged(Functor& /*aMetadataChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyMetadata(Brhz& aMetadata) const
{
  aMetadata.Set(iMetadata);
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyTransportStateChanged(Functor& /*aTransportStateChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyTransportState(Brhz& aTransportState) const
{
  aTransportState.Set(iState);
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyIdChanged(Functor& /*aIdChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyId(TUint& aId) const
{
  aId = iId;
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyIdArrayChanged(Functor& /*aIdArrayChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyIdArray(Brh& aIdArray) const
{
  aIdArray.Set(iArray);
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyChannelsMaxChanged(Functor& /*aChannelsMaxChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyChannelsMax(TUint& aChannelsMax) const
{
  aChannelsMax = iChannelsMax;
}

void CpProxyAvOpenhomOrgRadio1Test::SetPropertyProtocolInfoChanged(Functor& /*aProtocolInfoChanged*/)
{

}

void CpProxyAvOpenhomOrgRadio1Test::PropertyProtocolInfo(Brhz& aProtocolInfo) const
{
  aProtocolInfo.Set(iProtocolInfo);
}

AsyncTest::AsyncTest()
{

}

AsyncTest::~AsyncTest()
{

}

void AsyncTest::Output(IAsyncOutput& /*aConsole*/)
{

}
