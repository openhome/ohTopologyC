#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Converter.h>
#include <OpenHome/Private/Thread.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::TestFramework;


EXCEPTION(TestException);


namespace OpenHome {


class TestExceptionReporter;

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
    void Test2();
    void TestFunctor1(void*);
    void TestFunctor2(void*);
    void TestFunctor2x(void*);
    void TestFunctorBlock(void*);
    void TestFunctorException(void*);
    void TestFunctorExceptionStd(void*);

private:
    Semaphore iSema1;
    Semaphore iSema2;
    Semaphore iSemaBlock;
    TestExceptionReporter* iExceptionReporter;
    IWatchableThread* iWatchableThread;
};

/////////////////////////////////////////////////////////////////////

class TestExceptionReporter :public IExceptionReporter
{
public:
    TestExceptionReporter();
    virtual void Report(Exception& aException);
    virtual void Report(std::exception& aException);

    TUint Count() const;
    TUint CountStd() const;

private:
    TUint iCount;
    TUint iCountStd;
};

} // namespace OpenHome


/////////////////////////////////////////////////////////////////////

TestExceptionReporter::TestExceptionReporter()
    :iCount(0)
    ,iCountStd(0)
{
}


void TestExceptionReporter::Report(Exception& /*aException*/)
{
    //Log::Print("TestExceptionReporter::Report(Exception) \n");
    iCount++;
}


void TestExceptionReporter::Report(std::exception& /*aException*/)
{
    //Log::Print("TestExceptionReporter::Report(std::Exception) \n");
    iCountStd++;
}


TUint TestExceptionReporter::Count() const
{
    return(iCount);
}


TUint TestExceptionReporter::CountStd() const
{
    return(iCountStd);
}

////////////////////////////////////////////////////////////////////

// SuitePowerManager

SuiteWatchableThread::SuiteWatchableThread()
    :SuiteUnitTest("SuiteWatchableThread")
    ,iSema1("1", 0)
    ,iSema2("2", 0)
    ,iSemaBlock("b", 0)
{
    AddTest(MakeFunctor(*this, &SuiteWatchableThread::Test1));
    AddTest(MakeFunctor(*this, &SuiteWatchableThread::Test2));
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

    FunctorGeneric<void*> f1 = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor1);
    FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor2);
    FunctorGeneric<void*> fb = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctorBlock);
    FunctorGeneric<void*> f2x = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor2x);

    TUint x;

    // test Execute calls functor
    iWatchableThread->Execute(f1, &x);
    iSema1.Wait();

    // test Schedule calls functor
    iWatchableThread->Schedule(f2, &x);
    iSema2.Wait();

    // test Execute blocks when other functors are scheduled
    TEST(!iSema2.Clear());
    TEST(!iSema1.Clear());

    for (TUint i=0; i<WatchableThread::kMaxFifoEntries; i++)
    {
        iWatchableThread->Schedule(f1, &x);
    }

    iWatchableThread->Execute(f2, &x);
    iSema2.Wait();
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries; i++)
    {
        iSema1.Wait();
    }
    TEST(!iSema1.Clear());



    // Test Execute calls back immediately if on the watchable thread
    TEST(!iSema2.Clear());


    for (TUint i=0; i<6; i++)
    {
        iWatchableThread->Schedule(f1, &x);
    }



    iWatchableThread->Schedule(f2x, &x); //this will call execute on WT

    for (TUint i=0; i<3; i++)
    {
        iWatchableThread->Schedule(fb, &x); // these will block
    }


    iSema2.Wait();


    for (TUint i=0; i<3; i++)
    {
        iSemaBlock.Signal();  // unblock the remaining scheduled blocking functors
    }

    Log::Print("Completed Test1\n");
}


void SuiteWatchableThread::TestFunctorBlock(void*)
{
    iSemaBlock.Wait();
}


void SuiteWatchableThread::TestFunctor2x(void*)
{
    TUint x;
    //Log::Print("2x \n");
    TEST(iWatchableThread->IsWatchableThread());
    FunctorGeneric<void*> f2 = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor2);
    iWatchableThread->Execute(f2, &x);
}


void SuiteWatchableThread::TestFunctor2(void*)
{
    //Log::Print("2 \n");
    TEST(iWatchableThread->IsWatchableThread());
    iSema2.Signal();
}


void SuiteWatchableThread::TestFunctor1(void*)
{
    //Log::Print("1 \n");
    TEST(iWatchableThread->IsWatchableThread());
    iSema1.Signal();
}


void SuiteWatchableThread::TestFunctorException(void*)
{
    THROW(TestException);
}


void SuiteWatchableThread::TestFunctorExceptionStd(void*)
{
    std::exception e;
    throw(e);
}



void SuiteWatchableThread::Test2()
{
    TUint x;

    FunctorGeneric<void*> f1 = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctor1);

    ////////////////////////
    // test ohNet exceptions
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctorException);
    TUint count = iExceptionReporter->Count();

    // executing
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Execute(f, &x);
        iWatchableThread->Execute(f1, &x);
        iSema1.Wait();
        count++;
        TEST(iExceptionReporter->Count()==count);
    }

    // scheduling
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Schedule(f, &x);
    }

    iWatchableThread->Execute(f1, &x);
    iSema1.Wait();
    TEST(iExceptionReporter->Count()==(count+WatchableThread::kMaxFifoEntries));


    //////////////////////
    // test std exceptions
    FunctorGeneric<void*> fstd = MakeFunctorGeneric(*this, &SuiteWatchableThread::TestFunctorExceptionStd);
    TUint countstd = iExceptionReporter->CountStd();

    // executing
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Execute(fstd, &x);
        iWatchableThread->Execute(f1, &x);
        iSema1.Wait();
        countstd++;

        TEST(iExceptionReporter->CountStd()==countstd);
    }

    // scheduling
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Schedule(fstd, &x);
    }

    iWatchableThread->Execute(f1, &x);
    iSema1.Wait();
    TEST(iExceptionReporter->CountStd()==(countstd+WatchableThread::kMaxFifoEntries));
}

////////////////////////////////////////////

void TestWatchableThread()
{
    Runner runner("WatchableThread tests\n");
    runner.Add(new SuiteWatchableThread());
    runner.Run();
}
