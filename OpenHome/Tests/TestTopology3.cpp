#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology3.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


namespace OpenHome {

namespace Av {


class SuiteTopology3: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology3(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology3* iTopology3;
    IReader& iReader;
};



////////////////////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcherUnordered<ITopology3Room*>, public IDisposable
{
public:
    RoomWatcher(MockableScriptRunner* aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(*iRunner))
    {
    }

    // IUnorderedWatcher<ITopology3Room*>
    void UnorderedOpen() {}
    void UnorderedInitialised() {}
    void UnorderedClose() {}


    void UnorderedAdd(ITopology3Room* aItem)
    {
        Bws<100> buf;
        buf.Replace("Room Added ");
        buf.Append(aItem->Name());
        Bwh* result = new Bwh(buf);

        iRunner->Result(result);
        iFactory->Create<ITopologymGroup*>(aItem->Name(), aItem->Groups(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
    }

    void UnorderedRemove(ITopology3Room* aItem)
    {
        Bws<100> buf;
        buf.Replace("Room Removed ");
        buf.Append(aItem->Name());
        Bwh* result = new Bwh(buf);

        iFactory->Destroy(aItem->Name());
        iRunner->Result(result);
    }

    // IDisposable
    void Dispose()
    {
        iFactory->Dispose();
    }

private:


    void CreateCallback(ArgsTwo<ITopologymGroup*, FunctorGeneric<const Brx&>>* aArgs)
    {
        ITopologymGroup* group = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();
        f(group->Id());
    }


private:
    MockableScriptRunner* iRunner;
    ResultWatcherFactory* iFactory;
};

} // Av

} // OpenHome


//////////////////////////////////////////////////////////////////////////



SuiteTopology3::SuiteTopology3(IReader& aReader)
    :SuiteUnitTest("SuiteTopology3")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology3::Test1));
}


void SuiteTopology3::Setup()
{
}


void SuiteTopology3::TearDown()
{
    delete iTopology3;
}


void SuiteTopology3::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topologym* topologym = new Topologym(topology2, *log);
    iTopology3 = new Topology3(topologym, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    RoomWatcher* watcher = new RoomWatcher(runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology3::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));
    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology3::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology3->Dispose();
    topologym->Dispose();
    topology2->Dispose();
    topology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();
}


void SuiteTopology3::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology3::ExecuteCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology3->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology3::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology3::ScheduleCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology3->Rooms().AddWatcher(*watcher);
}


void TestTopology3(Environment& aEnv, std::vector<Brn>& aArgs)
{
    if(aArgs.size()<2)
    {
        aArgs.push_back(Brn("--path"));
        aArgs.push_back(Brn("~eamonnb/Topology3TestScript.txt"));
    }

    TestScriptHttpReader reader(aEnv, aArgs);

    Runner runner("Topology3 tests\n");
    runner.Add(new SuiteTopology3(reader));
    runner.Run();
}






