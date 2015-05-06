#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology6.h>
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
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {

namespace Topology {


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

    void ScheduleCallback(void* aWatcher);
    void ExecuteCallback(void* aWatcher);

private:
    Net::Library& iLib;
    Topology6* iTopology;
};

/////////////////////////////////////////////////////////////////////

class HouseWatcher : public IWatcherUnordered<ITopologyRoom*>
{

    virtual void UnorderedOpen() { /*   Log::Print("HouseWatcher::UnorderedOpen()  \n");*/ };
    virtual void UnorderedInitialised() {};
    virtual void UnorderedAdd(ITopologyRoom* aItem);
    virtual void UnorderedRemove(ITopologyRoom* aItem);
    virtual void UnorderedClose() {};
};


} // namespace Topology

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////

void HouseWatcher::UnorderedRemove(ITopologyRoom* aItem)
{
    Log::Print("HouseWatcher::UnorderedRemove()  removing room: ");
    Log::Print(aItem->Name());
    Log::Print("\n");;
}


void HouseWatcher::UnorderedAdd(ITopologyRoom* aItem)
{
    Log::Print("HouseWatcher::UnorderedAdd()  adding room: ");
    Log::Print(aItem->Name());
    Log::Print("\n");;
}

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

    FunctorGeneric<Net::CpDevice*> fAdd = MakeFunctorGeneric<Net::CpDevice*>(*network, &Network::AddCpDevice);
    FunctorGeneric<Net::CpDevice*> fRem = MakeFunctorGeneric<Net::CpDevice*>(*network, &Network::RemoveCpDevice);

    InjectorProduct* injector = new InjectorProduct(cpStack, fAdd, fRem, *log);

    auto watcher = new HouseWatcher();


    iTopology = Topology6::CreateTopology(network, *log);

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopologyManual::ScheduleCallback);
    network->Schedule(fe, watcher);



    for (TUint i=0; i<3600; i++)
    {
        Log::Print("i=%d \n", i);
        Thread::Sleep(1000);
    }

    Log::Print("exiting \n");
    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopologyManual::ExecuteCallback);
    network->Execute(fs, watcher);


    iTopology->Dispose();
    network->Dispose();
    injector->Dispose();

    delete iTopology;
    delete network;
    delete injector;

}


void SuiteTopologyManual::ScheduleCallback(void* aWatcher)
{
    HouseWatcher* watcher = (HouseWatcher*)aWatcher;
    iTopology->Rooms().AddWatcher(*watcher);
}

void SuiteTopologyManual::ExecuteCallback(void* aWatcher)
{
    HouseWatcher* watcher = (HouseWatcher*)aWatcher;
    iTopology->Rooms().RemoveWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopologyManual(Net::Library& aLib, const std::vector<Brn>& /*aArgs*/)
{
    Runner runner("TopologyManaul tests\n");
    runner.Add(new SuiteTopologyManual(aLib));
    runner.Run();
}
