#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology2.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Private/Http.h>
#include <exception>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {

namespace Av {


class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopology2: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology2(Environment& aEnv);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology2* iTopology2;
    Environment& iEnv;
};

/////////////////////////////////////////////////////////////////////

class GroupWatcher : public IWatcherUnordered<ITopology2Group*>, public IDisposable
{
public:
    GroupWatcher(MockableScriptRunner* aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(*aRunner))
    {
    }

    void Dispose()
    {
        iFactory->Dispose();
    }

    void UnorderedOpen()
    {
    }

    void UnorderedInitialised()
    {
    }

    void UnorderedClose()
    {
    }

    void UnorderedRemove(ITopology2Group* aItem)
    {
        Bws<100> buf;
        buf.Replace(aItem->Device().Udn());
        buf.Append(" Group Removed");
        Bwh* result = new Bwh(buf);

        iFactory->Destroy(aItem->Device().Udn());
        iRunner->Result(result);
/*
        iFactory->Destroy(aItem.Device().Udn());
        iRunner->Result(aItem.Device().Udn() + " Group Removed");
*/
    }


    void UnorderedAdd(ITopology2Group* aItem)
    {
        Bws<100> buf;

        buf.Replace(aItem->Device().Udn());
        buf.Append(Brn(" Group Added"));

        Bwh* result = new Bwh(buf);
        iRunner->Result(result);


        FunctorGeneric<ArgsTwo<Brn, FunctorGeneric<const Brx&>>*> fRoom = MakeFunctorGeneric(*this, &GroupWatcher::RoomCallback);
        FunctorGeneric<ArgsTwo<Brn, FunctorGeneric<const Brx&>>*> fName = MakeFunctorGeneric(*this, &GroupWatcher::NameCallback);
        FunctorGeneric<ArgsTwo<TUint, FunctorGeneric<const Brx&>>*> fSourceIndex = MakeFunctorGeneric(*this, &GroupWatcher::SourceIndexCallback);
        FunctorGeneric<ArgsTwo<TBool, FunctorGeneric<const Brx&>>*> fStandby = MakeFunctorGeneric(*this, &GroupWatcher::StandbyCallback);

        iFactory->Create<Brn>(aItem->Device().Udn(), aItem->Room(), fRoom);
        iFactory->Create<Brn>(aItem->Device().Udn(), aItem->Name(), fName);
        iFactory->Create<TUint>(aItem->Device().Udn(), aItem->SourceIndex(), fSourceIndex);
        iFactory->Create<TBool>(aItem->Device().Udn(), aItem->Standby(), fStandby);


        std::vector<IWatchable<ITopology2Source*>*> sources(aItem->Sources());
        for(TUint i=0; i<sources.size(); i++)
        {
            FunctorGeneric<ArgsTwo<ITopology2Source*, FunctorGeneric<const Brx&>>*> fSources = MakeFunctorGeneric(*this, &GroupWatcher::SourcesCallback);

            iFactory->Create<ITopology2Source*>(aItem->Device().Udn(), *sources[i], fSources);
        }

/*
        iFactory->Create<Brn>(aItem.Device().Udn(), aItem.Room, (v, w) => w("Room " + v) );
        iFactory->Create<string>(aItem.Device().Udn(), aItem.Name, (v, w) => w("Name " + v));
        iFactory->Create<TUint>(aItem.Device().Udn(), aItem.SourceIndex, (v, w) => w("SourceIndex " + v));
        iFactory->Create<bool>(aItem.Device().Udn(), aItem.Standby, (v, w) => w("Standby " + v));

        foreach (IWatchable<ITopology2Source> s in aItem.Sources)
        {
            iFactory->Create<ITopology2Source>(aItem.Device().Udn(), s, (v, w) => w("Source " + v.Index + " " + v.Name + " " + v.Type + " " + v.Visible));
        }
*/

    }


private:
    void SourcesCallback(ArgsTwo<ITopology2Source*, FunctorGeneric<const Brx&>>* aObj)
    {
        // w("Source " + v.Index + " " + v.Name + " " + v.Type + " " + v.Visible));
        ArgsTwo<ITopology2Source*, FunctorGeneric<const Brx&>>* args = (ArgsTwo<ITopology2Source*, FunctorGeneric<const Brx&>>*)aObj;
        ITopology2Source* src = args->Arg1();
        FunctorGeneric<const Brx&> f = args->Arg2();
        Bws<100> buf;
        buf.Replace(Brn("Source "));
        buf.Append(src->Index());
        buf.Append(Brn(" "));
        buf.Append(src->Name());
        buf.Append(Brn(" "));
        buf.Append(src->Type());
        buf.Append(Brn(" "));
        if (src->Visible())
        {
            buf.Append(Brn("True"));
        }
        else
        {
            buf.Append(Brn("False"));
        }
        //f(new Bwh(buf));
        f(buf);
        delete args;
    }

    void RoomCallback(ArgsTwo<Brn, FunctorGeneric<const Brx&>>* aObj)
    {
        ArgsTwo<Brn, FunctorGeneric<const Brx&>>* args = (ArgsTwo<Brn, FunctorGeneric<const Brx&>>*)aObj;
        Brn str = args->Arg1();
        FunctorGeneric<const Brx&> f = args->Arg2();
        Bws<100> buf;
        buf.Replace(Brn("Room "));
        buf.Append(str);
        //f(new Bwh(buf));
        f(buf);
        delete args;
    }


    void NameCallback(ArgsTwo<Brn, FunctorGeneric<const Brx&>>* aObj)
    {

        ArgsTwo<Brn, FunctorGeneric<const Brx&>>* args = (ArgsTwo<Brn, FunctorGeneric<const Brx&>>*)aObj;
        Brn str = args->Arg1();
        FunctorGeneric<const Brx&> f = args->Arg2();
        Bws<100> buf;
        buf.Replace(Brn("Name "));
        buf.Append(str);
        //f(new Bwh(buf));
        f(buf);
        delete args;

    }

    void SourceIndexCallback(ArgsTwo<TUint, FunctorGeneric<const Brx&>>* aObj)
    {

        ArgsTwo<TUint, FunctorGeneric<const Brx&>>* args = (ArgsTwo<TUint, FunctorGeneric<const Brx&>>*)aObj;
        TUint i = args->Arg1();
        FunctorGeneric<const Brx&> f = args->Arg2();
        Bws<100> buf;
        buf.Replace(Brn("SourceIndex "));
        Ascii::AppendDec(buf, i);
        //f(new Bwh(buf));
        f(buf);
        delete args;

    }


    void StandbyCallback(ArgsTwo<TBool, FunctorGeneric<const Brx&>>* aObj)
    {

        ArgsTwo<TBool, FunctorGeneric<const Brx&>>* args = (ArgsTwo<TBool, FunctorGeneric<const Brx&>>*)aObj;
        TBool i = args->Arg1();
        FunctorGeneric<const Brx&> f = args->Arg2();
        Bws<100> buf;
        buf.Replace(Brn("Standby "));
        if (i)
        {
            buf.Append(Brn("True"));
        }
        else
        {
            buf.Append(Brn("False"));
        }
        //f(new Bwh(buf));
        f(buf);
        delete args;

    }


private:
    MockableScriptRunner* iRunner;
    ResultWatcherFactory* iFactory;
};


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


class HttpReader;


////////////////////////////////////////////////////////////////////


SuiteTopology2::SuiteTopology2(Environment& aEnv)
    :SuiteUnitTest("SuiteTopology2")
    ,iEnv(aEnv)
{
    AddTest(MakeFunctor(*this, &SuiteTopology2::Test1));
}


void SuiteTopology2::Setup()
{
}


void SuiteTopology2::TearDown()
{
    delete iTopology2;
}


void SuiteTopology2::Test1()
{
    LOG(kTrace, "\n");

    Brn uriPath("http://eng.linn.co.uk/~eamonnb/Topology2TestScript.txt");
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



    Topology1* topology1 = new Topology1(network, *log);
    iTopology2 = new Topology2(topology1, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();

    GroupWatcher* watcher = new GroupWatcher(runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology2::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, reader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology2::ExecuteCallback);
    network->Execute(fe, watcher);


    iTopology2->Dispose();
    topology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();
}


void SuiteTopology2::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology2::ExecuteCallback() \n");
    GroupWatcher* watcher = (GroupWatcher*)aObj;
    iTopology2->Groups().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology2::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology2::ScheduleCallback() \n");
    GroupWatcher* watcher = (GroupWatcher*)aObj;
    iTopology2->Groups().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology2(Environment& aEnv)
{
    //Debug::SetLevel(Debug::kTrace);
    Runner runner("Topology2 tests\n");
    runner.Add(new SuiteTopology2(aEnv));
    runner.Run();
}



/*
namespace TestTopology2
{
    class Program
    {
        class GroupWatcher : IUnorderedWatcher<ITopology2Group>, IDisposable
        {
            public GroupWatcher(MockableScriptRunner aRunner)
            {
                iRunner = aRunner;
                iFactory = new ResultWatcherFactory(aRunner);
            }

            public void Dispose()
            {
                iFactory.Dispose();
            }

            public void UnorderedOpen()
            {
            }

            public void UnorderedInitialised()
            {
            }

            public void UnorderedClose()
            {
            }

            public void UnorderedAdd(ITopology2Group aItem)
            {
                iRunner.Result(aItem.Device.Udn + " Group Added");
                iFactory.Create<string>(aItem.Device.Udn, aItem.Room, (v, w) => w("Room " + v));
                iFactory.Create<string>(aItem.Device.Udn, aItem.Name, (v, w) => w("Name " + v));
                iFactory.Create<TUint>(aItem.Device.Udn, aItem.SourceIndex, (v, w) => w("SourceIndex " + v));
                iFactory.Create<bool>(aItem.Device.Udn, aItem.Standby, (v, w) => w("Standby " + v));

                foreach (IWatchable<ITopology2Source> s in aItem.Sources)
                {
                    iFactory.Create<ITopology2Source>(aItem.Device.Udn, s, (v, w) => w("Source " + v.Index + " " + v.Name + " " + v.Type + " " + v.Visible));
                }
            }

            public void UnorderedRemove(ITopology2Group aItem)
            {
                iFactory.Destroy(aItem.Device.Udn);
                iRunner.Result(aItem.Device.Udn + " Group Removed");
            }

            private readonly MockableScriptRunner iRunner;
            private readonly ResultWatcherFactory iFactory;
        }

        static int Main(string[] args)
        {
            if (args.Length != 1)
            {
                Console.WriteLine("Usage: TestTopology2.exe <testscript>");
                return 1;
            }

            Mockable mocker = new Mockable();

            Log log = new Log(new LogConsole());

            Network network = new Network(50, log);
            DeviceInjectorMock mockInjector = new DeviceInjectorMock(network, Path.GetDirectoryName(System.Reflection.Assembly.GetEntryAssembly().Location), log);
            mocker.Add("network", mockInjector);

            Topology2 topology1 = new Topology2(network, log);
            Topology2 topology2 = new Topology2(topology1, log);

            MockableScriptRunner runner = new MockableScriptRunner();

            GroupWatcher watcher = new GroupWatcher(runner);

            network.Schedule(() =>
            {
                topology2.Groups.AddWatcher(watcher);
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
                topology2.Groups.RemoveWatcher(watcher);
                watcher.Dispose();
            });

            topology2.Dispose();

            topology1.Dispose();

            mockInjector.Dispose();

            network.Dispose();

            return 0;
        }
    }
}
*/
