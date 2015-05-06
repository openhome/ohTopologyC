#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology3.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Tests/TestScriptHttpReader.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/MetaData.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;


namespace OpenHome
{
namespace Topology
{
namespace TestTopology3
{


/////////////////////////////////////////////////////////////////////

class SuiteTopology3: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology3(ReaderUntil& aReader);

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
    ReaderUntil& iReader;
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


    virtual void UnorderedOpen() {}
    virtual void UnorderedInitialised() {}
    virtual void UnorderedClose() {}

    virtual void UnorderedAdd(ITopology3Group* aItem)
    {
        Bwh* result = new Bwh(200);
        result->Replace(aItem->Device().Udn());
        result->Append(" Group Added");
        iRunner.Result(result);
        iFactory->Create<ISender*>(aItem->Device().Udn(), aItem->Sender(), MakeFunctorGeneric(*this, &RoomWatcher::CreateCallback));
    }

    virtual void UnorderedRemove(ITopology3Group* aItem)
    {
        Bwh* result = new Bwh(200);
        result->Replace(aItem->Device().Udn());
        result->Append(" Group Removed");

        iFactory->Destroy(aItem->Device().Udn());
        iRunner.Result(result);
    }

    // IDisposable
    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;
    }

private:
    void CreateCallback(MockCbData<ISender*>* aArgs)
    {
        ISender* sender = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

        Bws<100> buf;

        TBool enabled = sender->Enabled();

        if (enabled)
        {
            buf.Replace(Brn("Sender True "));

            IDevice& device = sender->Device();

            Brn udn = device.Udn();
            buf.Append(udn);
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


} // TestTopology3
} // Av
} // OpenHome


/////////////////////////////////////////////////////////////////////

using namespace OpenHome::Topology::TestTopology3;


SuiteTopology3::SuiteTopology3(ReaderUntil& aReader)
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
    delete runner;
    delete log;
    delete mocker;

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
    if (!reader->Connect(uri))
    {
        Log::Print("Failed to connect \n");
        ASSERTS();
    }

    ReaderUntilS<1024>* readerUntil = new ReaderUntilS<1024>(*reader);

    Runner runner("Topology3 tests\n");
    runner.Add(new SuiteTopology3(*readerUntil));
    runner.Run();

    delete readerUntil;
    delete reader;
}
