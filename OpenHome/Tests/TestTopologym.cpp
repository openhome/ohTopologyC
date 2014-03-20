#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topologym.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Private/Http.h>
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
    SuiteTopologym(Environment& aEnv);

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
    Environment& iEnv;
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

    // IUnorderedWatcher<ITopologymGroup*>

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
            //w("Sender True " + v.Device.Udn);
            buf.Replace(Brn("Sender True "));
            buf.Append(sender->Device().Udn());
            f(buf);
        }
        else
        {
            //w("Sender False");
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


SuiteTopologym::SuiteTopologym(Environment& aEnv)
    :SuiteUnitTest("SuiteTopologym")
    ,iEnv(aEnv)
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
    LOG(kTrace, "\n");

    Brn uriPath("http://eng.linn.co.uk/~eamonnb/TopologymTestScript.txt");
    Uri uri(uriPath);

    HttpReader reader(iEnv);

    if (!reader.Connect(uri))
    {
        ASSERTS();
    }

    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    //Topology1* topology1 = new Topology1(network, *log);
    //iTopologym = new Topologym(topology1, *log);


    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    iTopologym = new Topologym(topology2, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();

    RoomWatcher* watcher = new RoomWatcher(runner);


    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopologym::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, reader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopologym::ExecuteCallback);
    network->Execute(fe, watcher);


    //iTopologym->Dispose();
    //topology1->Dispose();
    //mockInjector->Dispose();
    //network->Dispose();


/*
    Mockable mocker = new Mockable();
    Log log = new Log(new LogConsole());
    Network network = new Network(50, log);
    DeviceInjectorMock mockInjector = new DeviceInjectorMock(network, Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location), log);
    mocker.Add("network", mockInjector);



    Topology1 topology1 = new Topology1(network, log);
    Topology2 topology2 = new Topology2(topology1, log);
    Topologym topologym = new Topologym(topology2, log);

    MockableScriptRunner runner = new MockableScriptRunner();

    RoomWatcher watcher = new RoomWatcher(runner);

    network.Schedule(() =>
    {
        topologym.Groups.AddWatcher(watcher);
    });

    try
    {
        runner.Run(network.Wait, new StreamReader(args[0]), mocker);
    }
    catch (MockableScriptRunner.AssertError)
    {
        return 1;
    }

    network.Execute(() =>
    {
        topologym.Groups.RemoveWatcher(watcher);
        watcher.Dispose();
    });

    topologym.Dispose();

    topology2.Dispose();

    topology1.Dispose();

    mockInjector.Dispose();

    network.Dispose();


*/

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

void TestTopologym(Environment& aEnv)
{
    //Debug::SetLevel(Debug::kTrace);
    Runner runner("Topologym tests\n");
    runner.Add(new SuiteTopologym(aEnv));
    runner.Run();
}


