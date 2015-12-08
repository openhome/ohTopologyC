#include <OpenHome/TestCpProxies/CpAvOpenhomeOrgRadio1Test.h>

using namespace OpenHome::Topology;
using namespace OpenHome::Net;
using namespace OpenHome;

CpProxyAvOpenhomeOrgRadio1Test::CpProxyAvOpenhomeOrgRadio1Test(std::shared_ptr<ResultRecorder<RadioEvent>> aRecorder)
  : iRecorder(aRecorder)
  , iAsync(new AsyncTest())
{

}

CpProxyAvOpenhomeOrgRadio1Test::~CpProxyAvOpenhomeOrgRadio1Test()
{

}

void CpProxyAvOpenhomeOrgRadio1Test::SyncPlay()
{
  iRecorder->Record(RadioEvent::eSyncPlay);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginPlay(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginPlay);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndPlay(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndPlay);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncPause()
{
  iRecorder->Record(RadioEvent::eSyncPause);

}

void CpProxyAvOpenhomeOrgRadio1Test::BeginPause(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginPause);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndPause(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndPause);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncStop()
{
  iRecorder->Record(RadioEvent::eSyncStop);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginStop(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginStop);

  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndStop(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndStop);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSeekSecondAbsolute(TUint /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncSeekSecondAbsolute);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSeekSecondAbsolute(TUint /*aValue*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginSeekSecondAbsolute);

  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSeekSecondAbsolute(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndSeekSecondAbsolute);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSeekSecondRelative(TInt /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncSeekSecondRelative);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSeekSecondRelative(TInt /*aValue*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginSeekSecondRelative);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSeekSecondRelative(IAsync& /*aFunctor*/)
{
  iRecorder->Record(RadioEvent::eEndSeekSecondRelative);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncChannel(Brh& /*aUri*/, Brh& /*aMetadata*/)
{
  iRecorder->Record(RadioEvent::eSyncChannel);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginChannel(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginChannel);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndChannel(IAsync& /*aFunctor*/, Brh& /*aUri*/, Brh& /*aMetadata*/)
{
  iRecorder->Record(RadioEvent::eEndChannel);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSetChannel(const Brx& /*aUri*/, const Brx& /*aMetadata*/)
{
  iRecorder->Record(RadioEvent::eSyncSetChannel);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSetChannel(const Brx& /*aUri*/, const Brx&/* aMetadata*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginSetChannel);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSetChannel(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndSetChannel);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncTransportState(Brh& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncTransportState);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginTransportState(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginTransportState);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndTransportState(IAsync& /*aAsync*/, Brh& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eEndTransportState);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncId(TUint& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncId);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginId(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginId);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndId(IAsync& /*aAsync*/, TUint& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eEndId);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncSetId(TUint /*aValue*/, const Brx& /*aUri*/)
{
  iRecorder->Record(RadioEvent::eSyncSetId);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginSetId(TUint /*aValue*/, const Brx& /*aUri*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginSetId);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndSetId(IAsync& /*aAsync*/)
{
  iRecorder->Record(RadioEvent::eEndSetId);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncRead(TUint /*aId*/, Brh& /*aMetadata*/)
{
  iRecorder->Record(RadioEvent::eSyncRead);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginRead(TUint /*aId*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginRead);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndRead(IAsync& /*aAsync*/, Brh& /*aMetadata*/)
{
  iRecorder->Record(RadioEvent::eEndRead);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncReadList(const Brx& /*aIdList*/, Brh& /*aChannelList*/)
{
  iRecorder->Record(RadioEvent::eSyncReadList);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginReadList(const Brx& /*aIdList*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginReadList);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndReadList(IAsync& /*aAsync*/, Brh& /*aChannelList*/)
{
  iRecorder->Record(RadioEvent::eEndReadList);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncIdArray(TUint& /*aToken*/, Brh& /*aArray*/)
{
  iRecorder->Record(RadioEvent::eSyncIdArray);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginIdArray(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginIdArray);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndIdArray(IAsync& /*aAsync*/, TUint& /*aToken*/, Brh& /*aArray*/)
{
  iRecorder->Record(RadioEvent::eEndIdArray);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncIdArrayChanged(TUint /*aToken*/, TBool& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncIdArrayChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginIdArrayChanged(TUint /*aToken*/, FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginIdArrayChanged);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndIdArrayChanged(IAsync& /*aAsync*/, TBool& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eEndIdArrayChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncChannelsMax(TUint& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncChannelsMax);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginChannelsMax(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginChannelsMax);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndChannelsMax(IAsync& /*aAsync*/, TUint& /*aValue*/)
{
  iRecorder->Record(RadioEvent::eEndChannelsMax);
}

void CpProxyAvOpenhomeOrgRadio1Test::SyncProtocolInfo(Brh&/* aValue*/)
{
  iRecorder->Record(RadioEvent::eSyncProtocolInfo);
}

void CpProxyAvOpenhomeOrgRadio1Test::BeginProtocolInfo(FunctorAsync& aFunctor)
{
  iRecorder->Record(RadioEvent::eBeginProtocolInfo);
  aFunctor(*iAsync);
}

void CpProxyAvOpenhomeOrgRadio1Test::EndProtocolInfo(IAsync& /*aAsync*/, Brh& aValue)
{
  iRecorder->Record(RadioEvent::eEndProtocolInfo);
  aValue.Set(Brn("TestEndProtocolInfo"));
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyUriChanged(Functor& /*aUriChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyUriChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyUri(Brhz& aUri) const
{
  iRecorder->Record(RadioEvent::ePropertyUri);
  aUri.Set(Brn("TestUri"));
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyMetadataChanged(Functor& /*aMetadataChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyMetadataChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyMetadata(Brhz& aMetadata) const
{
  iRecorder->Record(RadioEvent::ePropertyMetadata);
  aMetadata.Set(Brn("TestMetadata"));
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyTransportStateChanged(Functor& /*aTransportStateChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyTransportStateChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyTransportState(Brhz& aTransportState) const
{
  iRecorder->Record(RadioEvent::ePropertyTransportState);
  aTransportState.Set(Brn("TestTransportState"));
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyIdChanged(Functor& /*aIdChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyIdChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyId(TUint& aId) const
{
  iRecorder->Record(RadioEvent::ePropertyId);
  aId = 1;
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyIdArrayChanged(Functor& /*aIdArrayChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyIdArrayChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyIdArray(Brh& aIdArray) const
{
  iRecorder->Record(RadioEvent::ePropertyIdArray);
  aIdArray.Set(Brn("TestIdArray"));
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyChannelsMaxChanged(Functor& /*aChannelsMaxChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyChannelsMaxChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyChannelsMax(TUint& /*aChannelsMax*/) const
{
  iRecorder->Record(RadioEvent::ePropertyChannelsMax);
}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyProtocolInfoChanged(Functor& /*aProtocolInfoChanged*/)
{
  iRecorder->Record(RadioEvent::eSetPropertyProtocolInfoChanged);
}

void CpProxyAvOpenhomeOrgRadio1Test::PropertyProtocolInfo(Brhz& aProtocolInfo) const
{
  iRecorder->Record(RadioEvent::ePropertyProtocolInfo);
  aProtocolInfo.Set(Brn("TestProtocolInfo"));
}




////////////////////////////////////ICpProxy Functions/////////
void CpProxyAvOpenhomeOrgRadio1Test::Subscribe()
{

}

void CpProxyAvOpenhomeOrgRadio1Test::Unsubscribe()
{

}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyChanged(Functor& /*aFunctor*/)
{

}

void CpProxyAvOpenhomeOrgRadio1Test::SetPropertyInitialEvent(Functor& /*aFunctor*/)
{

}

void CpProxyAvOpenhomeOrgRadio1Test::AddProperty(Property* /*aProperty*/)
{

}

void CpProxyAvOpenhomeOrgRadio1Test::DestroyService()
{

}

void CpProxyAvOpenhomeOrgRadio1Test::ReportEvent(Functor /*aFunctor*/)
{

}

TUint CpProxyAvOpenhomeOrgRadio1Test::Version() const
{
  return 1;
}


/////////////////////////////////////////////AsyncTest/////////


AsyncTest::AsyncTest()
{

}

AsyncTest::~AsyncTest()
{

}

void AsyncTest::Output(IAsyncOutput& /*aConsole*/)
{

}
