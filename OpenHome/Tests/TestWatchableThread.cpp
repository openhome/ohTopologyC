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
    TUint CallCount();
    TUint CallCountWt();

private:
    IWatchableThread* iWatchableThread;
    Functor iFunctor;
    TUint iCallCount;
    TUint iCallCountWt;

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
    ,iCallCount(0)
    ,iCallCountWt(0)
{
    iFunctor = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor);

    AddTest(MakeFunctor(*this, &SuiteWatchableThread::Test1));
}


void SuiteWatchableThread::Setup()
{
    TestExceptionReporter* e = new TestExceptionReporter();
    iWatchableThread = new WatchableThread(e);
}

void SuiteWatchableThread::TearDown()
{
    delete iWatchableThread;
}



void SuiteWatchableThread::Test1()
{
    TEST_THROWS(iWatchableThread->Assert(), AssertionFailed);

    Functor f = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor);


    iWatchableThread->Execute(f);
    Thread::Sleep(1000);
    TEST(CallCount()==1);

    iWatchableThread->Schedule(f);
    Thread::Sleep(1000);
    TEST(CallCount()==2);


    Functor fwt1 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorWt1);
    iWatchableThread->Execute(fwt1);

    iWatchableThread->Schedule(fwt1);

    Thread::Sleep(1000);

}


void SuiteWatchableThread::TestFunctorWt1()
{
    TEST(iWatchableThread->IsWatchableThread());

    Functor fwt2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorWt2);

    TUint count = CallCountWt();
    iWatchableThread->Execute(fwt2);
    TEST(CallCountWt()==(count+1));
}


void SuiteWatchableThread::TestFunctorWt2()
{
    TEST(iWatchableThread->IsWatchableThread());
    iCallCountWt++;
}


void SuiteWatchableThread::TestFunctor()
{
    TEST(iWatchableThread->IsWatchableThread());
    iCallCount++;
}


TUint SuiteWatchableThread::CallCount()
{
    return(iCallCount);
}

TUint SuiteWatchableThread::CallCountWt()
{
    return(iCallCountWt);
}


void TestWatchableThread()
{
    Runner runner("WatchableThread tests\n");
    runner.Add(new SuiteWatchableThread());
    runner.Run();
}
