#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/Topology1.h>
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

class SuiteTopology1: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteTopology1(Environment& aEnv);

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
    Environment& iEnv;
};

/////////////////////////////////////////////////////////////////////

class ProductWatcher : public IWatcherUnordered<IProxyProduct>
{
public:
    ProductWatcher(MockableScriptRunner* aRunner) : iRunner(aRunner) {}

    virtual void UnorderedOpen() {}
    virtual void UnorderedClose() {/*LOG(kTrace, "ProductWatcher::UnorderedClose \n");*/}
    virtual void UnorderedInitialised() {/*LOG(kTrace, "ProductWatcher::UnorderedInitialised \n");*/}

    virtual void UnorderedAdd(IProxyProduct& aWatcher)
	{
		//LOG(kTrace, "ProductWatcher::UnorderedAdd \n");
		Bws<100> buf;
		buf.Replace(Brn("product added "));
		buf.Append(aWatcher.Device().Udn());

		Bwh* result = new Bwh(buf);
		iRunner->Result(result);
	}

    virtual void UnorderedRemove(IProxyProduct& aWatcher)
    {
        //LOG(kTrace, "ProductWatcher::UnorderedRemove \n");
		Bws<100> buf;
		buf.Replace(Brn("product removed "));
		buf.Append(aWatcher.Device().Udn());

		Bwh* result = new Bwh(buf);
		iRunner->Result(result);
    }

private:
    MockableScriptRunner* iRunner;
	//Bws<100> iResult;
};


} // namespace Av

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////


class HttpReader;


////////////////////////////////////////////////////////////////////


SuiteTopology1::SuiteTopology1(Environment& aEnv)
    :SuiteUnitTest("SuiteTopology1")
    ,iEnv(aEnv)
{
    AddTest(MakeFunctor(*this, &SuiteTopology1::Test1));
}


void SuiteTopology1::Setup()
{
}


void SuiteTopology1::TearDown()
{
    delete iTopology1;
}


void SuiteTopology1::Test1()
{
    LOG(kTrace, "\n");

    Brn uriPath("http://eng.linn.co.uk/~eamonnb/Topology1TestScript.txt");
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

    iTopology1 = new Topology1(network, *log);

    MockableScriptRunner* runner = new MockableScriptRunner();

	ProductWatcher* watcher = new ProductWatcher(runner);

    FunctorGeneric<void*> fs = MakeFunctorGeneric(*this, &SuiteTopology1::ScheduleCallback);
    network->Schedule(fs, watcher);

    Functor f = MakeFunctor(*network, &Network::Wait);

    //try
    //{
        runner->Run(f, reader, *mocker);
    //}
    //catch(MockableScriptRunner.AssertError)
    //{
    //    return 1;
    //}

    //FunctorGeneric<void*> fe = MakeFunctorGeneric(*this, &SuiteTopology1::ExecuteCallback);
    //network->Execute(fe, watcher);

/*
    topology.Dispose();
    mockInjector.Dispose();
    network.Dispose();
*/

}


void SuiteTopology1::ExecuteCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology1::ExecuteCallback() \n");
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().RemoveWatcher(*watcher);
}


void SuiteTopology1::ScheduleCallback(void* aObj)
{
    LOG(kTrace, "SuiteTopology1::ScheduleCallback() \n");
    ProductWatcher* watcher = (ProductWatcher*)aObj;
    iTopology1->Products().AddWatcher(*watcher);
}


////////////////////////////////////////////

void TestTopology1(Environment& aEnv)
{
    Debug::SetLevel(Debug::kTrace);
    Runner runner("Topology1 tests\n");
    runner.Add(new SuiteTopology1(aEnv));
    runner.Run();
}
