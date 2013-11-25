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
    void TestFunctor1();
    void TestFunctor2();
    void TestFunctor2x();
    void TestFunctorBlock();
    void TestFunctorException();
    void TestFunctorExceptionStd();

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

    Functor f1 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor1);
    Functor f2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2);
    Functor fb = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorBlock);
    Functor f2x = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2x);

    // test Execute calls functor
    iWatchableThread->Execute(f1);
    iSema1.Wait();

    // test Schedule calls functor
    iWatchableThread->Schedule(f2);
    iSema2.Wait();

    // test Execute blocks when other functors are scheduled
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



    // Test Execute calls back immediately if on the watchable thread
    TEST(!iSema2.Clear());


    for (TUint i=0; i<6; i++)
    {
        iWatchableThread->Schedule(f1);
    }

    iWatchableThread->Schedule(f2x); //this will call execute on WT

    for (TUint i=0; i<3; i++)
    {
        iWatchableThread->Schedule(fb); // these will block
    }


    iSema2.Wait();


    for (TUint i=0; i<3; i++)
    {
        iSemaBlock.Signal();  // unblock the remaining scheduled blocking functors
    }

}


void SuiteWatchableThread::TestFunctorBlock()
{
    iSemaBlock.Wait();
}

void SuiteWatchableThread::TestFunctor2x()
{
    //Log::Print("2x \n");
    TEST(iWatchableThread->IsWatchableThread());
    Functor f2 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor2);
    iWatchableThread->Execute(f2);
}


void SuiteWatchableThread::TestFunctor2()
{
    //Log::Print("2 \n");
    TEST(iWatchableThread->IsWatchableThread());
    iSema2.Signal();
}


void SuiteWatchableThread::TestFunctor1()
{
    //Log::Print("1 \n");
    TEST(iWatchableThread->IsWatchableThread());
    iSema1.Signal();
}


void SuiteWatchableThread::TestFunctorException()
{
    THROW(TestException);
}

void SuiteWatchableThread::TestFunctorExceptionStd()
{
    std::exception e;
    throw(e);
}



void SuiteWatchableThread::Test2()
{
    Functor f1 = MakeFunctor(*this, &SuiteWatchableThread::TestFunctor1);

    ////////////////////////
    // test ohNet exceptions
    Functor f = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorException);
    TUint count = iExceptionReporter->Count();

    // executing
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Execute(f);
        iWatchableThread->Execute(f1);
        iSema1.Wait();
        count++;
        TEST(iExceptionReporter->Count()==count);
    }

    // scheduling
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Schedule(f);
    }

    iWatchableThread->Execute(f1);
    iSema1.Wait();
    TEST(iExceptionReporter->Count()==(count+WatchableThread::kMaxFifoEntries));


    //////////////////////
    // test std exceptions
    Functor fstd = MakeFunctor(*this, &SuiteWatchableThread::TestFunctorExceptionStd);
    TUint countstd = iExceptionReporter->CountStd();

    // executing
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Execute(fstd);
        iWatchableThread->Execute(f1);
        iSema1.Wait();
        countstd++;

        TEST(iExceptionReporter->CountStd()==countstd);
    }

    // scheduling
    for (TUint i=0; i<WatchableThread::kMaxFifoEntries;i++)
    {
        iWatchableThread->Schedule(fstd);
    }

    iWatchableThread->Execute(f1);
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
