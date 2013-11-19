#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Converter.h>
#include <OpenHome/Private/Thread.h>

using namespace OpenHome;
using namespace OpenHome::TestFramework;

namespace OpenHome {

class SuiteWatchableThread: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteWatchableThread();

private:
    // from SuiteUnitTest
    void Setup();
    void TearDown();
private:
    void Test1();
    void TestFunctor();
    void TestFunctorWt1();
    void TestFunctorWt2();

private:
    IExceptionReporter* iExceptionReporter;
    IWatchableThread* iWatchableThread;
    Functor iFunctor;
    Semaphore iSema1;
    Semaphore iSema2;
};

/////////////////////////////////////////////////////////////////////

class TestExceptionReporter :public IExceptionReporter
{
public:
    TestExceptionReporter();
    virtual void Report(Exception& aException);
    virtual void Report(std::exception& aException);
};

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////

TestExceptionReporter::TestExceptionReporter()
{
}

void TestExceptionReporter::Report(Exception& /*aException*/)
{

}


void TestExceptionReporter::Report(std::exception& /*aException*/)
{

}

////////////////////////////////////////////////////////////////////

// SuitePowerManager

SuiteWatchableThread::SuiteWatchableThread()
    :SuiteUnitTest("SuiteWatchableThread")
    ,iSema1("1", 0)
    ,iSema2("2", 0)
{
    iFunctor = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor);
    AddTest(MakeFunctor(*this, &SuiteWatchableThread::Test1));
}


void SuiteWatchableThread::Setup()
{
    iExceptionReporter = new TestExceptionReporter();
    iWatchableThread = new WatchableThread(*iExceptionReporter);
}

void SuiteWatchableThread::TearDown()
{
    delete iExceptionReporter;
    delete iWatchableThread;
}


void SuiteWatchableThread::Test1()
{
    TEST_THROWS(iWatchableThread->Assert(), AssertionFailed);

    Functor f = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor);

    iWatchableThread->Execute(f);
    iSema1.Wait();

    iWatchableThread->Schedule(f);
    iSema1.Wait();

    Functor fwt1 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorWt1);
    iWatchableThread->Execute(fwt1);
    iSema1.Wait();

    iWatchableThread->Schedule(fwt1);
    iSema2.Wait();
}


void SuiteWatchableThread::TestFunctorWt1()
{
    TEST(iWatchableThread->IsWatchableThread());
    Functor fwt2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorWt2);
    iSema1.Signal();
    iWatchableThread->Execute(fwt2);
}


void SuiteWatchableThread::TestFunctorWt2()
{
    TEST(iWatchableThread->IsWatchableThread());
    iSema2.Signal();
}


void SuiteWatchableThread::TestFunctor()
{
    TEST(iWatchableThread->IsWatchableThread());
    iSema1.Signal();
}


void TestWatchableThread()
{
    Runner runner("WatchableThread tests\n");
    runner.Add(new SuiteWatchableThread());
    runner.Run();
}
