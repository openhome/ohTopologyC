#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topologym.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <exception>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {

namespace Av {


class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopologym: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopologym(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topologym* iTopologym;
    IReader& iReader;
};

/////////////////////////////////////////////////////////////////////
class RoomWatcher : public IWatcherUnordered<ITopologymGroup*>, public IDisposable
{
public:
    RoomWatcher(MockableScriptRunner* aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(*iRunner))
    {
    }


    void UnorderedOpen() {}
    void UnorderedInitialised() {}
    void UnorderedClose() {}

    void UnorderedAdd(ITopologymGroup* aItem)
    {
        Bws<100> buf;
        buf.Replace(aItem->Device().Udn());
        buf.Append(" Group Added");
        Bwh* result = new Bwh(buf);
        iRunner->Result(result);
        iFactory->Create<ITopologymSender*>(aItem->Device().Udn(), aItem->Sender(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
    }

    void UnorderedRemove(ITopologymGroup* aItem)
    {
        Bws<100> buf;
        buf.Replace(aItem->Device().Udn());
        buf.Append(" Group Removed");
        Bwh* result = new Bwh(buf);

        iFactory->Destroy(aItem->Device().Udn());
        iRunner->Result(result);
    }

    // IDisposable
    void Dispose()
    {
        iFactory->Dispose();
    }

private:
    void CreateCallback(ArgsTwo<ITopologymSender*, FunctorGeneric<const Brx&>>* aObj)
    {
        ITopologymSender* sender = aObj->Arg1();
        FunctorGeneric<const Brx&> f = aObj->Arg2();

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
    }

private:
    MockableScriptRunner* iRunner;
    ResultWatcherFactory* iFactory;
};


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


//class HttpReader;


SuiteTopologym::SuiteTopologym(IReader& aReader)
    :SuiteUnitTest("SuiteTopologym")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopologym::Test1));
}


void SuiteTopologym::Setup()
{
}


void SuiteTopologym::TearDown()
{
    delete iTopologym;
}


void SuiteTopologym::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    iTopologym = new Topologym(topology2, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    RoomWatcher* watcher = new RoomWatcher(runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopologym::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));
    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopologym::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopologym->Dispose();
    topology2->Dispose();
    topology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();
}


void SuiteTopologym::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopologym::ExecuteCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopologym->Groups().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopologym::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopologym::ScheduleCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopologym->Groups().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopologym(Environment& aEnv, std::vector<Brn>& aArgs)
{
    if(aArgs.size()<2)
    {
        aArgs.push_back(Brn("--path"));
        aArgs.push_back(Brn("~eamonnb/TopologymTestScript.txt"));
    }
    TestScriptHttpReader reader(aEnv, aArgs);
    Runner runner("Topologym tests\n");
    runner.Add(new SuiteTopologym(reader));
    runner.Run();
}


