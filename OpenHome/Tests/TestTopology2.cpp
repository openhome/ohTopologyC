#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology2.h>
#include <OpenHome/Mockable.h>
#include <OpenHome/Injector.h>
#include <OpenHome/Private/Ascii.h>
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
namespace TestTopology2
{


class TestExceptionReporter;

/////////////////////////////////////////////////////////////////////

class SuiteTopology2: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology2(ReaderUntil& aReader);

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
    ReaderUntil& iReader;
};

/////////////////////////////////////////////////////////////////////

class GroupWatcher : public IWatcherUnordered<ITopology2Group*>, public IDisposable, public INonCopyable
{
public:
    GroupWatcher(MockableScriptRunner& aRunner)
        :iRunner(aRunner)
        ,iFactory(new ResultWatcherFactory(aRunner))
    {
    }

    virtual void Dispose()
    {
        iFactory->Dispose();
        delete iFactory;
    }

    virtual void UnorderedOpen()
    {
    }

    virtual void UnorderedInitialised()
    {
    }

    virtual void UnorderedClose()
    {
    }

    virtual void UnorderedRemove(ITopology2Group* aItem)
    {
        Bwh* result = new Bwh(100);
        result->Replace(aItem->Device().Udn());
        result->Append(" Group Removed");

        iFactory->Destroy(aItem->Device().Udn());
        iRunner.Result(result);
    }


    virtual void UnorderedAdd(ITopology2Group* aItem)
    {
        Bwh* result = new Bwh(100);
        result->Replace(aItem->Device().Udn());
        result->Append(Brn(" Group Added"));
        iRunner.Result(result);

        FunctorGeneric<MockCbData<Brn>*> fRoom = MakeFunctorGeneric(*this, &GroupWatcher::RoomCallback);
        FunctorGeneric<MockCbData<Brn>*> fName = MakeFunctorGeneric(*this, &GroupWatcher::NameCallback);
        FunctorGeneric<MockCbData<TUint>*> fSourceIndex = MakeFunctorGeneric(*this, &GroupWatcher::SourceIndexCallback);
        FunctorGeneric<MockCbData<TBool>*> fStandby = MakeFunctorGeneric(*this, &GroupWatcher::StandbyCallback);

        iFactory->Create<Brn>(aItem->Device().Udn(), aItem->RoomName(), fRoom);
        iFactory->Create<Brn>(aItem->Device().Udn(), aItem->Name(), fName);
        iFactory->Create<TUint>(aItem->Device().Udn(), aItem->SourceIndex(), fSourceIndex);
        iFactory->Create<TBool>(aItem->Device().Udn(), aItem->Standby(), fStandby);

        std::vector<Watchable<ITopology2Source*>*> sources = aItem->Sources();
        for(TUint i=0; i<sources.size(); i++)
        {
            iFactory->Create<ITopology2Source*>(aItem->Device().Udn(), *(sources[i]), MakeFunctorGeneric(*this, &GroupWatcher::SourcesCallback));
        }

    }


private:
    void SourcesCallback(MockCbData<ITopology2Source*>* aArgs)
    {
        // w("Source " + v.Index + " " + v.Name + " " + v.Type + " " + v.Visible));
        ITopology2Source* src = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;

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
        delete aArgs;
    }

    void RoomCallback(MockCbData<Brn>* aArgs)
    {
        Brn str = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
        Bws<100> buf;
        buf.Replace(Brn("Room "));
        buf.Append(str);
        f(buf);
        delete aArgs;
    }


    void NameCallback(MockCbData<Brn>* aArgs)
    {
        Brn str = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
        Bws<100> buf;
        buf.Replace(Brn("Name "));
        buf.Append(str);
        f(buf);
        delete aArgs;
    }

    void SourceIndexCallback(MockCbData<TUint>* aArgs)
    {
        TUint i = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
        Bws<100> buf;
        buf.Replace(Brn("SourceIndex "));
        Ascii::AppendDec(buf, i);
        f(buf);
        delete aArgs;
    }


    void StandbyCallback(MockCbData<TBool>* aArgs)
    {
        TBool i = aArgs->iData;
        FunctorGeneric<const Brx&> f = aArgs->iCallback;
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
        delete aArgs;
    }


private:
    MockableScriptRunner& iRunner;
    ResultWatcherFactory* iFactory;
};

} // namespace TestTopology2
} // namespace Topology
} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


using namespace OpenHome::Topology::TestTopology2;


SuiteTopology2::SuiteTopology2(ReaderUntil& aReader)
    :SuiteUnitTest("SuiteTopology2")
    ,iReader(aReader)
{
    AddTest(MakeFunctor(*this, &SuiteTopology2::Test1));
}


void SuiteTopology2::Setup()
{
}


void SuiteTopology2::TearDown()
{
}


void SuiteTopology2::Test1()
{
    Mockable* mocker = new Mockable();
    ILog* log = new LogDummy();
    Network* network = new Network(50, *log);

    InjectorMock* mockInjector = new InjectorMock(*network, Brx::Empty(), *log);
    mocker->Add(Brn("network"), *mockInjector);

    Topology1* topology1 = new Topology1(*network, *log);
    iTopology2 = new Topology2(topology1, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();
    GroupWatcher* watcher = new GroupWatcher(*runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology2::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    TEST(runner->Run(f, iReader, *mocker));

    FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology2::ExecuteCallback);
    network->Execute(fe, watcher);


    iTopology2->Dispose();
    mockInjector->Dispose();
    network->Dispose();

    delete watcher;
    delete runner;
    delete log;
    delete mocker;

    delete iTopology2;
    delete mockInjector;
    delete network;
}


void SuiteTopology2::ExecuteCallback(void* aObj)
{
    LOG(kApplication7, "SuiteTopology2::ExecuteCallback() \n");
    GroupWatcher* watcher = (GroupWatcher*)aObj;
    iTopology2->Groups().RemoveWatcher(*watcher);
    watcher->Dispose();
}


void SuiteTopology2::ScheduleCallback(void* aObj)
{
    LOG(kApplication7, "SuiteTopology2::ScheduleCallback() \n");
    GroupWatcher* watcher = (GroupWatcher*)aObj;
    iTopology2->Groups().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology2(Environment& aEnv, const std::vector<Brn>& aArgs)
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

    Runner runner("Topology2 tests\n");
    runner.Add(new SuiteTopology2(*readerUntil));
    runner.Run();

    delete readerUntil;
    delete reader;
}


