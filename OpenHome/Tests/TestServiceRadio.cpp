/*! Test for Topology's ServiceRadio*/
#include <Generated/CpAvOpenhomeOrgRadio1.h>
#include <OpenHome/TestCpProxies/CpAvOpenhomeOrgRadio1Test.h>
#include <OpenHome/Network.h>
#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Device.h>
#include <OpenHome/ServiceRadio.h>
#include <OpenHome/TestCpProxies/ResultRecorder.h>
#include <OpenHome/TagManager.h>
#include <memory>

namespace OpenHome{
namespace Topology{
namespace TestServiceRadio{

class TestDevice : public IDevice
{
public:
    TestDevice(ResultRecorder<CpProxyAvOpenhomeOrgRadio1Test::RadioEvent>& aRecorder) : iRecorder(aRecorder), iUdn("test") {}
    ~TestDevice() {}
public:
    Brn Udn() { return Brn(iUdn); }
    void Create(FunctorGeneric<IProxy*> /*aCallback*/, EServiceType /*aService*/)
        {
            iRecorder.Record(CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eDeviceCreate);
        }
    void Join(Functor /*aAction*/)
    {
        iRecorder.Record(CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eDeviceJoin);
    }
    void Unjoin(Functor /*aAction*/)
    {
        iRecorder.Record(CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eDeviceUnjoin);
    }
private:
    ResultRecorder<CpProxyAvOpenhomeOrgRadio1Test::RadioEvent>& iRecorder;
    Bws<4> iUdn;
};


class SuiteServiceRadioNetwork : public TestFramework::SuiteUnitTest, public INonCopyable
{
public:
  SuiteServiceRadioNetwork();
  ~SuiteServiceRadioNetwork();
private: //SuiteUnitTest
  void Setup() override;
  void TearDown() override;
private:
  void FirstTest();
  void TestPlay();
  void TestPause();
  void TestStop();
  void TestSSAbsolute();
  void TestSSRelative();
  void TestSetId();
  void TestSetChannel();
  void TestCreateProxy();
  void WatchableThreadTests();
  void DoWatchableTests(void*);
  void TestDispose();
private:
    void FunctorCreate(IProxy* aProxy){
        TEST(aProxy != nullptr);
        delete aProxy;
    }
private:
  std::shared_ptr<ResultRecorder<CpProxyAvOpenhomeOrgRadio1Test::RadioEvent>> iRecorder;
  std::unique_ptr<ILog> iLog;
  std::unique_ptr<INetwork> iNetwork;
  std::unique_ptr<IInjectorDevice> iInjectorDevice;
  std::unique_ptr<CpProxyAvOpenhomeOrgRadio1Test> iCp;
  std::unique_ptr<ServiceRadioNetwork> iService;
  std::vector<CpProxyAvOpenhomeOrgRadio1Test::RadioEvent> iExpected;
};

}//TestServiceRadio
}//Topology
}//OpenHome
using namespace OpenHome;
using namespace Topology;
using namespace TestServiceRadio;
using namespace TestFramework;
using namespace Net;
void SuiteServiceRadioNetwork::Setup()
{

}

void SuiteServiceRadioNetwork::TearDown()
{
    TEST(iRecorder->AsExpected(iExpected));
    iRecorder->ClearResults();
    iExpected.clear();
}

void SuiteServiceRadioNetwork::WatchableThreadTests()
{
    //Network is actually directly available in test harness, but in production the service would access it via the InjectorDevice
    iInjectorDevice->Network().Execute(MakeFunctorGeneric<void*>(*this, &SuiteServiceRadioNetwork::DoWatchableTests), nullptr);
}

void SuiteServiceRadioNetwork::DoWatchableTests(void*)
{
    TestCreateProxy();
    TestDispose();
}
SuiteServiceRadioNetwork::SuiteServiceRadioNetwork()
  : SuiteUnitTest("SuiteServiceRadioNetwork")
  , iRecorder(new ResultRecorder<CpProxyAvOpenhomeOrgRadio1Test::RadioEvent>())
  , iLog(new LogDummy())
  , iNetwork(new Network(50, *iLog))
  , iInjectorDevice(new InjectorDeviceTest(*iNetwork))
  , iCp(new CpProxyAvOpenhomeOrgRadio1Test(iRecorder))
  , iService(new ServiceRadioNetwork(*iInjectorDevice, std::move(iCp), *iLog))
{
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::FirstTest));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestPlay));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestPause));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestStop));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestSSAbsolute));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestSSRelative));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestSetId));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::TestSetChannel));
    AddTest(MakeFunctor(*this, &SuiteServiceRadioNetwork::WatchableThreadTests));

}

SuiteServiceRadioNetwork::~SuiteServiceRadioNetwork()
{

}
void SuiteServiceRadioNetwork::FirstTest()
{
  iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eSetPropertyIdChanged, CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eSetPropertyIdArrayChanged, CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eSetPropertyMetadataChanged, CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eSetPropertyTransportStateChanged };
}
void SuiteServiceRadioNetwork::TestPlay()
{
  iExpected = {CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginPlay};
  iService->Play();
}

void SuiteServiceRadioNetwork::TestPause()
{
  iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginPause };
  iService->Pause();
}

void SuiteServiceRadioNetwork::TestStop()
{
  iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginStop };
  iService->Stop();
}

void SuiteServiceRadioNetwork::TestSSAbsolute()
{
  iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginSeekSecondAbsolute };
  iService->SeekSecondAbsolute(1);
}

void SuiteServiceRadioNetwork::TestSSRelative()
{
    iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginSeekSecondRelative };
    iService->SeekSecondRelative(1);
}

void SuiteServiceRadioNetwork::TestSetId()
{
    iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginSetId };
    iService->SetId(0, Brn("test"));
}

void SuiteServiceRadioNetwork::TestSetChannel()
{
    iExpected = { CpProxyAvOpenhomeOrgRadio1Test::RadioEvent::eBeginSetChannel };
    std::unique_ptr<MediaMetadata> metadata(new MediaMetadata());
    Bws<4> test("test");
    iService->SetChannel(test, *metadata);
}

void SuiteServiceRadioNetwork::TestCreateProxy()
{
    std::unique_ptr<IDevice> dv(new TestDevice(*iRecorder));
    iService->Create(MakeFunctorGeneric<IProxy*>(*this, &SuiteServiceRadioNetwork::FunctorCreate), dv.release());
}

void SuiteServiceRadioNetwork::TestDispose()
{
    iService->Dispose();
}
///////////////////////////////////////////////////////////////

void TestServiceRadio()
{
  Runner runner("ServiceRadio tests\n");
  runner.Add(new SuiteServiceRadioNetwork());
  runner.Run();
}
