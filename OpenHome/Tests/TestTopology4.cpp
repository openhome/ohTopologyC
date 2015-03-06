#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology4.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/MetaData.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;


namespace OpenHome {

namespace Topology {

namespace TestTopology4 {

class SuiteTopology4: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology4(IReader& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology4* iTopology4;
    IReader& iReader;
};

////////////////////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcherUnordered<ITopology4Room*>, public IDisposable, public INonCopyable
{
public:
    RoomWatcher(MockableScriptRunner& aRunner);
    ~RoomWatcher();
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose() {}
    virtual void UnorderedAdd(ITopology4Room* aItem);
    virtual void UnorderedRemove(ITopology4Room* aItem);
    virtual void Dispose();

private:
    void CreateCallback(MockCbData<ITopology3Group*>* aArgs);


private:
    MockableScriptRunner& iRunner;
    ResultWatcherFactory* iFactory;

};


    RoomWatcher::RoomWatcher(MockableScriptRunner& aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(iRunner))
    {
        //Log::Print("RoomWatcher::RoomWatcher() adr=%lx\n", this);
    }

    RoomWatcher::~RoomWatcher()
    {
        //Log::Print("RoomWatcher::~RoomWatcher()  DESTRUCTION \n");
    }

    void RoomWatcher::UnorderedOpen()
    {
        //Log::Print("RoomWatcher::UnorderedOpen() \n");
    }

    void RoomWatcher::UnorderedInitialised()
    {
        //Log::Print("RoomWatcher::UnorderedInitialised() \n");
    }

    void RoomWatcher::UnorderedAdd(ITopology4Room* aItem)
    {
        //Log::Print("RoomWatcher::UnorderedAdd() \n");
        if (aItem!=NULL)
        {
            Bwh* result = new Bwh(1000);
            result->Replace("Room Added ");
            result->Append(aItem->Name());

            iRunner.Result(result);
            iFactory->Create<ITopology3Group*>(aItem->Name(), aItem->Groups(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
        }
    }

    void RoomWatcher::UnorderedRemove(ITopology4Room* aItem)
    {
        //Log::Print("RoomWatcher::UnorderedRemove() \n");
        Bwh* result = new Bwh(1000);
        result->Replace("Room Removed ");
        result->Append(aItem->Name());

        iFactory->Destroy(aItem->Name());
        iRunner.Result(result);
    }

    void RoomWatcher::Dispose()
    {
        //Log::Print("RoomWatcher::Dispose() \n");
        iFactory->Dispose();
        delete iFactory;
    }

    void RoomWatcher::CreateCallback(MockCbData<ITopology3Group*>* aArgs)
    {
        //Log::Print("RoomWatcher::CreateCallback() \n");
        ITopology3Group* group = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
        delete aArgs;
        f(group->Id());
    }


} // TestTopology4
} // Av
} // OpenHome


//////////////////////////////////////////////////////////////////////////

using namespace OpenHome::Topology::TestTopology4;


SuiteTopology4::SuiteTopology4(IReader& aReader)
    :SuiteUnitTest("SuiteTopology4")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology4::Test1));
}


void SuiteTopology4::Setup()
{
}


void SuiteTopology4::TearDown()
{
}


void SuiteTopology4::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topology3* topology3 = new Topology3(topology2, *log);
    iTopology4 = new Topology4(topology3, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    RoomWatcher* watcher = new RoomWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology4::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));
    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology4::ExecuteCallback);
    network->Execute(fe, watcher);


    iTopology4->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete mocker;
    delete runner;
    delete log;

    delete iTopology4;
    delete mockInjector;
    delete network;

}


void SuiteTopology4::ExecuteCallback(void* aObj)
{
    //Log::Print("SuiteTopology4::ExecuteCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology4->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology4::ScheduleCallback(void* aObj)
{
    auto watcher = (IWatcherUnordered<ITopology4Room*>*)aObj;
    //Log::Print("SuiteTopology4::ScheduleCallback()  watcher=%lx\n", watcher);
    iTopology4->Rooms().AddWatcher(*watcher);
}


void TestTopology4(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    std::vector<Brn> args(aArgs);

    if( find(args.begin(), args.end(), Brn("--path")) == args.end() )
    {
        args.push_back(Brn("--path"));
        args.push_back(Brn("~eamonnb/Topology4TestScript.txt"));
    }

    TestScriptHttpReader* reader = new TestScriptHttpReader(aEnv, args);

    Runner runner("Topology4 tests\n");
    runner.Add(new SuiteTopology4(*reader));
    runner.Run();

    delete reader;
}






