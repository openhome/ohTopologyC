#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology2.h>
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


        std::vector<Watchable<ITopology2Source*>*> sources(aItem->Sources());
        for(TUint i=0; i<sources.size(); i++)
        {
            FunctorGeneric<ArgsTwo<ITopology2Source*, FunctorGeneric<const Brx&>>*> fSources = MakeFunctorGeneric(*this, &GroupWatcher::SourcesCallback);

            iFactory->Create<ITopology2Source*>(aItem->Device().Udn(), *sources[i], fSources);
        }
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
        Ascii::AppendDec(buf, src->Index());
        buf.Append(Brn(" "));
        
		Brn name(src->Name());

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


//class HttpReader;


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


    //iTopology2->Dispose();
    //topology1->Dispose();
    //mockInjector->Dispose();
    //network->Dispose();
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


