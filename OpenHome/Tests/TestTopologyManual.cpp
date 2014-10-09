#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology1.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <exception>

#include <OpenHome/Private/Thread.h>
#include <OpenHome/Net/Private/Ssdp.h>
#include <OpenHome/Net/Core/CpDeviceUpnp.h>
#include <OpenHome/Net/Core/FunctorCpDevice.h>
#include <OpenHome/Net/Private/XmlParser.h>
#include <OpenHome/Net/Private/Globals.h>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {

namespace Av {


class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopologyManual: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopologyManual(Net::Library& aLib);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();


private:
    Topology1* iTopology1;
    Net::Library& iLib;
};

/////////////////////////////////////////////////////////////////////


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


SuiteTopologyManual::SuiteTopologyManual(Net::Library& aLib)
    :SuiteUnitTest("SuiteTopologyManual")
    ,iLib(aLib)
{
    AddTest(MakeFunctor(*this, &SuiteTopologyManual::Test1));
}


void SuiteTopologyManual::Setup()
{
}


void SuiteTopologyManual::TearDown()
{

}


void SuiteTopologyManual::Test1()
{
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    std::vector<NetworkAdapter*>* list = iLib.CreateSubnetList();
    TIpAddress subnetAddress = (*list)[0]->Subnet();

    iLib.DestroySubnetList(list);

    Net::CpStack& cpStack = *iLib.StartCp(subnetAddress);

    InjectorProduct* injector = new InjectorProduct(*network, cpStack, *log);

    iTopology1 = new Topology1(network, *log);

    Thread::Sleep(10000);

    iTopology1->Dispose();
    network->Dispose();
    injector->Dispose();

    delete iTopology1;
    delete network;
    delete injector;
}

////////////////////////////////////////////

void TestTopologyManual(Net::Library& aLib, const std::vector<Brn>& /*aArgs*/)
{
    Runner runner("TopologyManaul tests\n");
    runner.Add(new SuiteTopologyManual(aLib));
    runner.Run();
}
