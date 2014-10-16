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



/////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////
class RoomWatcher : public IWatcherUnordered<ITopology3Group*>, public IDisposable, public INonCopyable
{
public:
    RoomWatcher(MockableScriptRunner& aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(iRunner))
    {
    }


    void UnorderedOpen() {}
    void UnorderedInitialised() {}
    void UnorderedClose() {}

    void UnorderedAdd(ITopology3Group* aItem)
    {
        Bws<100> buf;
        buf.Replace(aItem->Device().Udn());
        buf.Append(" Group Added");
        Bwh* result = new Bwh(buf);
        iRunner.Result(result);
        iFactory->Create<ITopology3Sender*>(aItem->Device().Udn(), aItem->Sender(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
    }

    void UnorderedRemove(ITopology3Group* aItem)
    {
        Bws<100> buf;
        buf.Replace(aItem->Device().Udn());
        buf.Append(" Group Removed");
        Bwh* result = new Bwh(buf);

        iFactory->Destroy(aItem->Device().Udn());
        iRunner.Result(result);
    }

    // IDisposable
    void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;
    }

private:
    void CreateCallback(ArgsTwo<ITopology3Sender*, FunctorGeneric<const Brx&>>* aArgs)
    {
        ITopology3Sender* sender = aArgs->Arg1();
        FunctorGeneric<const Brx&> f = aArgs->Arg2();

        Bws<100> buf;

        if (sender->Enabled())
        {
            buf.Replace(Brn("Sender True "));

            IDevice* device = &sender->Device();
            buf.Append(device->Udn());
            f(buf);
        }
        else
        {
            f(Brn("Sender False"));
        }
        delete aArgs;
    }

private:
    MockableScriptRunner& iRunner;
    ResultWatcherFactory* iFactory;
};


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


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
    iTopology3 = new Topology3(topology2, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    RoomWatcher* watcher = new RoomWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology3::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology3::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology3->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete mocker;
    delete runner;
    delete log;

    delete iTopology3;
    delete mockInjector;
    delete network;

}


void SuiteTopology3::ExecuteCallback(void* aObj)
{
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology3->Groups().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology3::ScheduleCallback(void* aObj)
{
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology3->Groups().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology3(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    std::vector<Brn> args(aArgs);

    if(args.size()<2)
    {
        args.push_back(Brn("--path"));
        args.push_back(Brn("~eamonnb/Topology3TestScript.txt"));
    }

    TestScriptHttpReader* reader = new TestScriptHttpReader(aEnv, args);

    Runner runner("Topology3 tests\n");
    runner.Add(new SuiteTopology3(*reader));
    runner.Run();

    delete reader;
}


