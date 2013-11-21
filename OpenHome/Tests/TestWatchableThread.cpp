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
    void TestFunctor1();
    void TestFunctor2();
    void TestFunctor2x();

private:
    Semaphore iSema1;
    Semaphore iSema2;
    IExceptionReporter* iExceptionReporter;
    IWatchableThread* iWatchableThread;
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
    // test the assert helper works
    TEST_THROWS(iWatchableThread->Assert(), AssertionFailed);

    // test Execute calls functor
    Functor f1 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor1);
    iWatchableThread->Execute(f1);
    iSema1.Wait();

    // test Schedule calls functor
    Functor f2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2);
    iWatchableThread->Schedule(f2);
    iSema2.Wait();

    // test Execute when other functors are scheduled
    TEST(!iSema2.Clear());
    TEST(!iSema1.Clear());

    for (TUint i=0; i<WatchableThread::kMaxFifoEntries; i++)
    {
        iWatchableThread->Schedule(f1);
    }

    iWatchableThread->Execute(f2);
    iSema2.Wait();
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries; i++)
    {
        iSema1.Wait();
    }
    TEST(!iSema1.Clear());



    Functor f2x = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2x);

    TEST(!iSema2.Clear());
    iWatchableThread->Execute(f2x);
    iSema2.Wait();

    TEST(!iSema2.Clear());
    iWatchableThread->Schedule(f2x);
    iSema2.Wait();
}


void SuiteWatchableThread::TestFunctor2x()
{
    TEST(iWatchableThread->IsWatchableThread());
    Functor f2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2);
    iWatchableThread->Execute(f2);
}


void SuiteWatchableThread::TestFunctor2()
{
    TEST(iWatchableThread->IsWatchableThread());
    iSema2.Signal();
}


void SuiteWatchableThread::TestFunctor1()
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
