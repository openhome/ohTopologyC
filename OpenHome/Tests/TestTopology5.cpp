#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology5.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Http.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/MetaData.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;


namespace OpenHome {

namespace Topology {

namespace TestTopology5 {

class SuiteTopology5: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology5(ReaderUntil& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology5* iTopology5;
    ReaderUntil& iReader;
};

////////////////////////////////////////////////////////////////////////////////////

class RoomWatcher : public IWatcherUnordered<ITopology5Room*>, public IDisposable, public INonCopyable
{
public:
    RoomWatcher(MockableScriptRunner& aRunner);
    ~RoomWatcher();
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedClose() {}
    virtual void UnorderedAdd(ITopology5Room* aItem);
    virtual void UnorderedRemove(ITopology5Room* aItem);
    virtual void Dispose();

private:
    void CreateCallback(MockCbData<ITopology4Group*>* aArgs);


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

    void RoomWatcher::UnorderedAdd(ITopology5Room* aItem)
    {
        //Log::Print("RoomWatcher::UnorderedAdd() \n");
        if (aItem!=NULL)
        {
            Bwh* result = new Bwh(1000);
            result->Replace("Room Added ");
            result->Append(aItem->Name());

            iRunner.Result(result);
            iFactory->Create<ITopology4Group*>(aItem->Name(), aItem->Groups(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
        }
    }

    void RoomWatcher::UnorderedRemove(ITopology5Room* aItem)
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

    void RoomWatcher::CreateCallback(MockCbData<ITopology4Group*>* aArgs)
    {
        //Log::Print("RoomWatcher::CreateCallback() \n");
        ITopology4Group* group = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
        delete aArgs;
        f(group->Id());
    }


} // TestTopology5
} // Av
} // OpenHome


//////////////////////////////////////////////////////////////////////////

using namespace OpenHome::Topology::TestTopology5;


SuiteTopology5::SuiteTopology5(ReaderUntil& aReader)
    :SuiteUnitTest("SuiteTopology5")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology5::Test1));
}


void SuiteTopology5::Setup()
{
}


void SuiteTopology5::TearDown()
{
}


void SuiteTopology5::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(*network, *log);
    Topology2* topology2 = new Topology2(topology1, *log);
    Topology3* topology3 = new Topology3(topology2, *log);
    Topology4* topology4 = new Topology4(topology3, *log);
    iTopology5 = new Topology5(topology4, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    RoomWatcher* watcher = new RoomWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology5::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));
    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology5::ExecuteCallback);
    network->Execute(fe, watcher);


    iTopology5->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete mocker;
    delete runner;
    delete log;

    delete iTopology5;
    delete mockInjector;
    delete network;

}


void SuiteTopology5::ExecuteCallback(void* aObj)
{
    //Log::Print("SuiteTopology5::ExecuteCallback() \n");
    RoomWatcher* watcher = (RoomWatcher*)aObj;
    iTopology5->Rooms().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology5::ScheduleCallback(void* aObj)
{
    auto watcher = (IWatcherUnordered<ITopology5Room*>*)aObj;
    //Log::Print("SuiteTopology5::ScheduleCallback()  watcher=%lx\n", watcher);
    iTopology5->Rooms().AddWatcher(*watcher);
}


void TestTopology5(Environment& aEnv, const std::vector<Brn>& aArgs)
{
    if( find(aArgs.begin(), aArgs.end(), Brn("--path")) == aArgs.end() )
    {
        Log::Print("No path supplied!\n");
        ASSERTS();
    }

    OptionParser parser;
    OptionString optionServer("-s", "--server", Brn("eng"), "address of server to connect to");
    parser.AddOption(&optionServer);
    OptionUint optionPort("-p", "--port", 80, "server port to connect on");
    parser.AddOption(&optionPort);
    OptionString optionPath("", "--path", Brn(""), "path to use on server");
    parser.AddOption(&optionPath);
    if (!parser.Parse(aArgs) || parser.HelpDisplayed()) {
        return;
    }

    TUint port = optionPort.Value();
    ASSERT(port <= 65535);
    Bwh uriBuf(100);

    Endpoint endptServer = Endpoint(port, optionServer.Value());
    uriBuf.Replace(Brn("http://"));
    endptServer.AppendEndpoint(uriBuf);
    uriBuf.Append(Brn("/"));
    uriBuf.Append(optionPath.Value());
    Uri uri(uriBuf);

    auto reader = new HttpReader(aEnv);
    TUint code = reader->Connect(uri);

    if ((code < HttpStatus::kSuccessCodes) || (code >= HttpStatus::kRedirectionCodes))
    {
        Log::Print("Failed to connect \n");
        ASSERTS();
    }

    ReaderUntilS<1024>* readerUntil = new ReaderUntilS<1024>(*reader);

    Runner runner("Topology5 tests\n");
    runner.Add(new SuiteTopology5(*readerUntil));
    runner.Run();

    delete readerUntil;
    delete reader;
}
