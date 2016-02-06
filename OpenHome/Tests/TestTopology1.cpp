#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology1.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Http.h>
#include <exception>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome
{
namespace Topology
{

namespace TestTopology1
{

class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopology1: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology1(ReaderUntil& aReader);

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
    void Test1();

private:
    void ExecuteCallback(void* aObj);
    void ScheduleCallback(void* aObj);

private:
    Topology1* iTopology1;
    ReaderUntil& iReader;
};

/////////////////////////////////////////////////////////////////////

class ProductWatcher : public IWatcherUnordered<IProxyProduct*> , public INonCopyable
{
public:
    ProductWatcher(MockableScriptRunner& aRunner) : iRunner(aRunner) {}

    virtual void UnorderedOpen() {}
    virtual void UnorderedClose() {}
    virtual void UnorderedInitialised() {}

    virtual void UnorderedAdd(IProxyProduct* aWatcher)
    {
        Bwh* result = new Bwh(100);
        result->Replace(Brn("product added "));
        result->Append(aWatcher->Device().Udn());
        iRunner.Result(result);
    }

    virtual void UnorderedRemove(IProxyProduct* aWatcher)
    {
        Bwh* result = new Bwh(100);
        result->Replace(Brn("product removed "));
        result->Append(aWatcher->Device().Udn());
        iRunner.Result(result);
    }

private:
    MockableScriptRunner& iRunner;
};

} // TestTopology1
} // namespace Topology
} // namespace OpenHome


/////////////////////////////////////////////////////////////////////

using namespace OpenHome::Topology::TestTopology1;

SuiteTopology1::SuiteTopology1(ReaderUntil& aReader)
    :SuiteUnitTest("SuiteTopology1")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology1::Test1));
}


void SuiteTopology1::Setup()
{
}


void SuiteTopology1::TearDown()
{

}


void SuiteTopology1::Test1()
{
    Mockable* mocker = new Mockable();

    Network* network = new Network(50);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty());
    mocker->Add(Brn("network"), *mockInjector);

    iTopology1 = new Topology1(*network);

    MockableScriptRunner* runner = new MockableScriptRunner();

    ProductWatcher* watcher = new ProductWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology1::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology1::ExecuteCallback);
    network->Execute(fe, watcher);

    iTopology1->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete mocker;
    delete runner;

    delete iTopology1;
    delete mockInjector;
    delete network;
}


void SuiteTopology1::ExecuteCallback(void* aObj)
{
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().RemoveWatcher(*watcher);
}


void SuiteTopology1::ScheduleCallback(void* aObj)
{
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology1(Environment& aEnv, const std::vector<Brn>& aArgs)
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

    Runner runner("Topology1 tests\n");
    runner.Add(new SuiteTopology1(*readerUntil));
    runner.Run();

    delete readerUntil;
    delete reader;
}
