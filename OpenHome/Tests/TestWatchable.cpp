#include <OpenHome/Private/SuiteUnitTest.h>
#include <OpenHome/WatchableThread.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/WatchableOrdered.h>
#include <OpenHome/WatchableUnordered.h>
#include <OpenHome/IWatcher.h>
#include <OpenHome/OsWrapper.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Converter.h>
#include <OpenHome/Private/Thread.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


//EXCEPTION(TestException);


namespace OpenHome {

namespace Av {


////////////////////////////////////////////////////

template<class T>
class WatcherEvent
{
private:
    Brn iType;
    Brn iId;
    T iValue;
    T iPrevious;

    TBool iTypeModified;
    TBool iIdModified;
    TBool iValueModified;
    TBool iPreviousModified;

public:
    WatcherEvent(const Brx& aType, const Brx& aId, T aValue)
    {
        iType = Brn(aType);
        iId = Brn(aId);
        iValue = aValue;

        iTypeModified = true;
        iIdModified = true;
        iValueModified = true;
        iPreviousModified = false;
    }

    WatcherEvent(const Brx& aType, const Brx& aId, T aValue, T aPrevious)
    {
        iType = Brn(aType);
        iId = Brn(aId);
        iValue = aValue;
        iPrevious = aPrevious;

        iTypeModified = true;
        iIdModified = true;
        iValueModified = true;
        iPreviousModified = true;
    }

    void Check(const Brx& aType, const Brx& aId, T aValue, T aPrevious)
    {
        ASSERT(iType.Equals(Brn(aType)));
        ASSERT(iId.Equals(Brn(aId)));
        ASSERT(iValue.Equals(aValue));
        ASSERT(iPrevious.Equals(aPrevious));

        ASSERT(iTypeModified);
        ASSERT(iIdModified);
        ASSERT(iValueModified);
        ASSERT(iPreviousModified);
    }

    void Check(const Brx& aType, const Brx& aId, T aValue)
    {
        ASSERT(iType.Equals(Brn(aType)));
        ASSERT(iId.Equals(aId));
        ASSERT(iValue.Equals(aValue));

        ASSERT(iTypeModified);
        ASSERT(iIdModified);
        ASSERT(iValueModified);

        ASSERT(!iPreviousModified);
    }
};

////////////////////////////////////////////////////

template<class T>
class WatcherOrderedEvent
{
private:
    Brn iType;
    T iValue;
    TUint iIndex;
    TUint iFrom;
    TUint iTo;

    TBool iTypeModified;
    TBool iValueModified;
    TBool iIndexModified;
    TBool iFromModified;
    TBool iToModified;

public:
    WatcherOrderedEvent(const Brx& aType)
    {
        iType = Brn(aType);

        iTypeModified = true;

        iValueModified = false;
        iIndexModified = false;
        iFromModified = false;
        iToModified = false;

    }

    WatcherOrderedEvent(const Brx& aType, T aValue, TUint aIndex)
    {
        iType = Brn(aType);
        iValue = aValue;
        iIndex = aIndex;

        iTypeModified = true;
        iValueModified = true;
        iIndexModified = true;

        iFromModified = false;
        iToModified = false;

    }

    WatcherOrderedEvent(const Brx& aType, T aValue, TUint aFrom, TUint aTo)
    {
        iType = Brn(aType);
        iValue = aValue;
        iFrom = aFrom;
        iTo = aTo;

        iTypeModified = true;
        iValueModified = true;
        iFromModified = true;
        iToModified = true;

        iIndexModified = false;
    }

    void Check(const Brx& aType)
    {
        ASSERT(iType == Brn(aType));

        ASSERT(iTypeModified);

        ASSERT(!iIndexModified);
        ASSERT(!iValueModified);
        ASSERT(!iFromModified);
        ASSERT(!iToModified);
    }

    void Check(const Brx& aType, T aValue, TUint aIndex)
    {
        ASSERT(iType == Brn(aType));
        ASSERT(iValue.Equals(aValue));
        ASSERT(iIndex == aIndex);

        ASSERT(iIndexModified);
        ASSERT(iTypeModified);
        ASSERT(iValueModified);

        ASSERT(!iFromModified);
        ASSERT(!iToModified);
    }

    void Check(const Brx& aType, T aValue, TUint aFrom, TUint aTo)
    {
        ASSERT(iType == Brn(aType));
        ASSERT(iValue.Equals(aValue));
        ASSERT(iFrom == aFrom);
        ASSERT(iTo == aTo);

        ASSERT(iTypeModified);
        ASSERT(iValueModified);
        ASSERT(iFromModified);
        ASSERT(iToModified);

        ASSERT(!iIndexModified);
    }
};

////////////////////////////////////////////////////

template<class T>
class WatcherUnorderedEvent
{
private:
    Brn iType;
    T iValue;

    TBool iTypeModified;
    TBool iValueModified;

public:
    WatcherUnorderedEvent(const Brx& aType)
    {
        iType = Brn(aType);

        iTypeModified = true;
        iValueModified = false;
    }

    WatcherUnorderedEvent(const Brx& aType, T aValue)
    {
        iType = Brn(aType);
        iValue = aValue;
        iTypeModified = true;
        iValueModified = true;
    }

    void Check(const Brx& aType)
    {
        ASSERT(iType.Equals(Brn(aType)));

        ASSERT(iTypeModified);
        ASSERT(!iValueModified);
    }

    void Check(const Brx& aType, T aValue)
    {
        ASSERT(iType.Equals(Brn(aType)));
        ASSERT(iValue.Equals(aValue));

        ASSERT(iTypeModified);
        ASSERT(iValueModified);
    }
};

////////////////////////////////////////////////////

template<class T>
class WWatcher : public IWatcher<T>, public IWatcher<IWatchable<T>*>
{
private:
    mutable Mutex iMutex;
    Fifo<WatcherEvent<T>*> iEvents;

public:
    WWatcher()
        :iMutex("WWMX")
        ,iEvents(100)
    {
        //iEvents = new Fifo<WatcherEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherEvent<T>* Pop()
    {
        return (iEvents.Read());
    }

    // IWatcher<T>
    virtual void ItemOpen(const Brx& aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("open"), aId, aValue));
    }

    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("update"), aId, aValue, aPrevious));
    }

    virtual void ItemClose(const Brx& aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("close"), aId, aValue));
    }

    // IWatcher<IWatchable<T>>

    virtual void ItemOpen(const Brx& /*aId*/, IWatchable<T>* aValue)
    {
        aValue->AddWatcher(*this);
    }

    virtual void ItemUpdate(const Brx& /*aId*/, IWatchable<T>* aValue, IWatchable<T>* aPrevious)
    {
        aPrevious->RemoveWatcher(*this);
        aValue->AddWatcher(*this);
    }

    virtual void ItemClose(const Brx& /*aId*/, IWatchable<T>* aValue)
    {
        aValue->RemoveWatcher(*this);
    }
};

////////////////////////////////////////////////////

template<class T>
class Watcher : public IWatcher<T>
{
private:
    mutable Mutex iMutex;
    Fifo<WatcherEvent<T>*> iEvents;

public:
    Watcher() :iMutex("WRMX"), iEvents(100)
    {
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherEvent<T>* Pop()
    {
        return (iEvents.Read());
    }

    // IWatcher<T>

    virtual void ItemOpen(const Brx& aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("open"), aId, aValue));
    }

    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("update"), aId, aValue, aPrevious));
    }

    virtual void ItemClose(const Brx& aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("close"), aId, aValue));
    }
};

////////////////////////////////////////////////////

template<class T>
class WatcherOrdered : public IWatcherOrdered<T>
{
private:
    mutable Mutex iMutex;
    Fifo<WatcherOrderedEvent<T>*> iEvents;

public:
    WatcherOrdered()
        :iMutex("WOMX")
        ,iEvents(100)
    {
        //iEvents = new Fifo<WatcherOrderedEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherOrderedEvent<T>* Pop()
    {
        return (iEvents.Read());
    }

    // IOrderedWatcher<T>
    virtual void OrderedOpen()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("open")));
    }

    virtual void OrderedInitialised()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("initialised")));
    }

    virtual void OrderedAdd(T aItem, TUint aIndex)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("add"), aItem, aIndex));
    }

    virtual void OrderedMove(T aItem, TUint aFrom, TUint aTo)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("move"), aItem, aFrom, aTo));
    }

    virtual void OrderedRemove(T aItem, TUint aIndex)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("remove"), aItem, aIndex));
    }

    virtual void OrderedClose()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("close")));
    }
};

////////////////////////////////////////////////////

template<class T>
class WatcherUnordered : public IWatcherUnordered<T>
{
private:
    mutable Mutex iMutex;
    Fifo<WatcherUnorderedEvent<T>*> iEvents;


public:
    WatcherUnordered()
        :iMutex("WOMX")
        ,iEvents(100)
    {
        //iEvents = new Fifo<WatcherUnorderedEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherUnorderedEvent<T>* Pop()
    {
        return (iEvents.Read());
    }

    // IWatcherUnordered<T>

    virtual void UnorderedOpen()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("open")));
    }

    virtual void UnorderedInitialised()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("initialised")));
    }

    virtual void UnorderedAdd(T aItem)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("add"), aItem));
    }

    virtual void UnorderedRemove(T aItem)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("remove"), aItem));
    }

    virtual void UnorderedClose()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("close")));
    }
};

///////////////////////////////////////////////////////////////////////////////////

class TestExceptionReporter : public IExceptionReporter
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

///////////////////////////////////////////////////////////////////////////////////

class SuiteWatchable: public SuiteUnitTest, public INonCopyable
{
public:
    SuiteWatchable();
    void ReportException(Exception& aException);

private:
    // from SuiteWatchable
    void Setup();
    void TearDown();
private:
    void Test1(); // SingleWatchableSingleWatcher
    void Test2(); // RemoveWatcherFlushesCallbacks
    void Test3(); // SingleWatchableTwoWatchers
    void Test4();
    void Test5();
    void Test6();
    void Test7();
    void Test8();
    void Test9();



    // Test1 callbacks
    void Test1UpdateB(void*);
    void Test1UpdateC(void*);
    void Test1UpdateDE(void*);
    void Test1UpdateBA(void*);
    void Test1RemoveWatcher(void*);
    void Test1AddWatcher(void*);

    // Test2 callbacks
    void Test2Callback(void*);
    void Test3Callback(void*);
    void Test4Callback(void*);
    void Test5Callback(void*);
    void Test6Callback(void*);
    void Test7Callback(void*);
    void Test8Callback(void*);
    void Test9Callback(void*);



    void DoNothing(void*);




private:
    Watchable<Brn>* iWatchable;
    Watcher<Brn>* iWatcher1;
    Watcher<Brn>* iWatcher2;
    WWatcher<Brn>* iWWatcher1;

    WatcherOrdered<Brn>* iWatcherOrdered1;
    WatcherUnordered<Brn>* iWatcherUnordered1;

    WatchableThread* iThread;
    TestExceptionReporter* iExceptionReporter;

    TBool iSuccess;
};




} // Av

} // OpenHome

/////////////////////////////////////////////////////////////

SuiteWatchable::SuiteWatchable()
    :SuiteUnitTest("SuiteWatchable")
{
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test1));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test2));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test3));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test4));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test5));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test6));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test7));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test8));
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test9));
}


void SuiteWatchable::Setup()
{
    iExceptionReporter = new TestExceptionReporter();
}


void SuiteWatchable::TearDown()
{
    TEST(iSuccess);
}


void SuiteWatchable::Test1() // SingleWatchableSingleWatcher
{
    iSuccess = false;
    iWatcher1 = new Watcher<Brn>();
    iThread = new WatchableThread(*iExceptionReporter);
    iWatchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateBA) , 0);
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1AddWatcher) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);

    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("test"), Brn("A"));
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateB) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A"));
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateC) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateDE) , 0);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D"));
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1RemoveWatcher) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("test"), Brn("E"));
    iWatcher1->CheckEmpty();

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateBA) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckEmpty();

    iWatcher1->CheckEmpty();

    delete iThread;
    iSuccess = true;
}


void SuiteWatchable::Test1UpdateB(void*)
{
    iWatchable->Update(Brn("B"));
}

void SuiteWatchable::Test1UpdateC(void*)
{
    iWatchable->Update(Brn("C"));
}

void SuiteWatchable::Test1UpdateDE(void*)
{
    iWatchable->Update(Brn("D"));
    iWatchable->Update(Brn("E"));
}

void SuiteWatchable::Test1UpdateBA(void*)
{
    iWatchable->Update(Brn("B"));
    iWatchable->Update(Brn("A"));
}

void SuiteWatchable::Test1RemoveWatcher(void*)
{
    iWatchable->RemoveWatcher(*iWatcher1);
}

void SuiteWatchable::Test1AddWatcher(void*)
{
    iWatchable->AddWatcher(*iWatcher1);
}



void SuiteWatchable::Test2() // RemoveWatcherFlushesCallbacks
{
    iSuccess = false;
    iWatcher1 = new Watcher<Brn>();
    iThread = new WatchableThread(*iExceptionReporter);
    iWatchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test2Callback), 0);
    delete iThread;
    iWatcher1->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test2Callback(void*)
{
    iWatcher1->CheckEmpty();
    iWatchable->AddWatcher(*iWatcher1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("test"), Brn("A"));
    iWatcher1->CheckEmpty();
    iWatchable->Update(Brn("B"));
    iWatchable->Update(Brn("C"));
    iWatchable->RemoveWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("test"), Brn("C"));
    iWatcher1->CheckEmpty();
    iWatchable->Update(Brn("D"));
    iWatchable->AddWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("test"), Brn("D"));
    iWatcher1->CheckEmpty();
    iWatchable->RemoveWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("test"), Brn("D"));
    iWatcher1->CheckEmpty();
}



void SuiteWatchable::DoNothing(void*)
{
    // do nothing
}






void SuiteWatchable::Test3() // SingleWatchableTwoWatchers
{
    iSuccess = false;
    iWatcher1 = new Watcher<Brn>();
    iWatcher2 = new Watcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test3Callback), 0);

    delete iThread;

    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test3Callback(void*)
{
    Watchable<Brn>* watchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();

    watchable->Update(Brn("B"));
    watchable->Update(Brn("A"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();

    watchable->AddWatcher(*iWatcher1);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("test"), Brn("A"));
    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();

    watchable->Update(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A"));
    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();

    watchable->AddWatcher(*iWatcher2);
    iWatcher2->CheckNotEmpty();
    iWatcher2->Pop()->Check(Brn("open"), Brn("test"), Brn("B"));
    iWatcher2->CheckEmpty();
    iWatcher1->CheckEmpty();

    watchable->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWatcher1->CheckEmpty();
    iWatcher2->CheckNotEmpty();
    iWatcher2->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWatcher2->CheckEmpty();

    watchable->RemoveWatcher(*iWatcher1);
    watchable->Update(Brn("D"));
    watchable->Update(Brn("E"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("test"), Brn("C"));
    iWatcher1->CheckEmpty();
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher2->CheckNotEmpty();
    iWatcher2->Pop()->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C"));
    iWatcher2->CheckNotEmpty();
    iWatcher2->Pop()->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D"));
    iWatcher2->CheckEmpty();

    watchable->RemoveWatcher(*iWatcher2);
    iWatcher2->CheckNotEmpty();
    iWatcher2->Pop()->Check(Brn("close"), Brn("test"), Brn("E"));
    iWatcher2->CheckEmpty();
    iWatcher1->CheckEmpty();

    watchable->Update(Brn("B"));
    watchable->Update(Brn("A"));
    iWatcher1->CheckEmpty();
    iWatcher2->CheckEmpty();
}


void SuiteWatchable::Test4() // TwoWatchablesSingleWatcher
{
    iSuccess = false;
    iWatcher1 = new Watcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test4Callback), 0);

    delete iThread;
    iWatcher1->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test4Callback(void*)
{
    Watchable<Brn>* watchable1 = new Watchable<Brn>(*iThread, Brn("X"), Brn("A"));
    Watchable<Brn>* watchable2 = new Watchable<Brn>(*iThread, Brn("Y"), Brn("A"));

    iWatcher1->CheckEmpty();

    watchable1->Update(Brn("B"));
    watchable1->Update(Brn("A"));
    watchable2->Update(Brn("B"));
    watchable2->Update(Brn("A"));
    iWatcher1->CheckEmpty();

    watchable1->AddWatcher(*iWatcher1);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("X"), Brn("A"));
    iWatcher1->CheckEmpty();

    watchable2->AddWatcher(*iWatcher1);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("open"), Brn("Y"), Brn("A"));
    iWatcher1->CheckEmpty();

    watchable1->Update(Brn("B"));
    watchable2->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("X"), Brn("B"), Brn("A"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("Y"), Brn("C"), Brn("A"));
    iWatcher1->CheckEmpty();

    watchable1->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("X"), Brn("C"), Brn("B"));
    iWatcher1->CheckEmpty();

    watchable1->RemoveWatcher(*iWatcher1);
    watchable1->Update(Brn("D"));
    watchable1->Update(Brn("E"));
    watchable2->Update(Brn("D"));
    watchable2->Update(Brn("E"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("X"), Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("Y"), Brn("D"), Brn("C"));
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("update"), Brn("Y"), Brn("E"), Brn("D"));
    iWatcher1->CheckEmpty();

    watchable2->RemoveWatcher(*iWatcher1);
    iWatcher1->CheckNotEmpty();
    iWatcher1->Pop()->Check(Brn("close"), Brn("Y"), Brn("E"));
    iWatcher1->CheckEmpty();

    watchable1->Update(Brn("B"));
    watchable1->Update(Brn("A"));
    watchable2->Update(Brn("B"));
    watchable2->Update(Brn("A"));
    iWatcher1->CheckEmpty();

}



void SuiteWatchable::Test5() // SingleWatchableWatchableSingleWatcher
{
    iSuccess = false;
    iWWatcher1 = new WWatcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test5Callback), 0);

    delete iThread;
    iWWatcher1->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test5Callback(void*)
{
    Watchable<Brn>* w = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));
    Watchable<IWatchable<Brn>*>* ww = new Watchable<IWatchable<Brn>*>(*iThread, Brn("WW"), w);

    iWWatcher1->CheckEmpty();

    ww->AddWatcher(*iWWatcher1);
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("open"), Brn("test"), Brn("A"));
    iWWatcher1->CheckEmpty();

    w->Update(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A"));
    iWWatcher1->CheckEmpty();

    w->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWWatcher1->CheckEmpty();

    w->Update(Brn("D"));
    w->Update(Brn("E"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C"));
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D"));
    iWWatcher1->CheckEmpty();

    ww->RemoveWatcher(*iWWatcher1);
    iWWatcher1->CheckNotEmpty();
    iWWatcher1->Pop()->Check(Brn("close"), Brn("test"), Brn("E"));
    iWWatcher1->CheckEmpty();

    w->Update(Brn("B"));
    w->Update(Brn("A"));
    iWWatcher1->CheckEmpty();

}


void SuiteWatchable::Test6() // SingleWatchableOrderedSingleOrderedWatcher
{
    iSuccess = false;
    iWatcherOrdered1 = new WatcherOrdered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test6Callback), 0);

    delete iThread;
    iWatcherOrdered1->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test6Callback(void*)
{
    WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(*iThread);

    iWatcherOrdered1->CheckEmpty();

    watchable->Add(Brn("A"), 0);
    watchable->Add(Brn("B"), 1);
    iWatcherOrdered1->CheckEmpty();

    watchable->AddWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("open"));
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("A"), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("B"), 1);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("initialised"));
    iWatcherOrdered1->CheckEmpty();

    watchable->Add(Brn("C"), 0);
    watchable->Add(Brn("D"), 1);
    watchable->Move(Brn("C"), 2);
    watchable->Remove(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("C"), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("D"), 1);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("move"), Brn("C"), 0, 2);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("remove"), Brn("B"), 3);
    iWatcherOrdered1->CheckEmpty();

    watchable->RemoveWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("close"));

    watchable->AddWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("open"));
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("D"), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("A"), 1);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("add"), Brn("C"), 2);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("initialised"));
    iWatcherOrdered1->CheckEmpty();

    watchable->RemoveWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherOrdered1->CheckNotEmpty();
    iWatcherOrdered1->Pop()->Check(Brn("close"));

    watchable->Dispose();
}


void SuiteWatchable::Test7() // SingleWatchableUnorderedSingleWatcherUnordered
{
    iSuccess = false;
    iWatcherUnordered1 = new WatcherUnordered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test7Callback), 0);


    delete iThread;
    iWatcherUnordered1->CheckEmpty();
    iSuccess = true;
}


void SuiteWatchable::Test7Callback(void*)
{
    WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(*iThread);

    iWatcherUnordered1->CheckEmpty();

    watchable->Add(Brn("A"));
    watchable->Add(Brn("B"));
    iWatcherUnordered1->CheckEmpty();

    watchable->AddWatcher(*iWatcherUnordered1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("open"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("A"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("B"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("initialised"));
    iWatcherUnordered1->CheckEmpty();

    watchable->Add(Brn("C"));
    watchable->Add(Brn("D"));
    watchable->Remove(Brn("A"));
    watchable->Remove(Brn("C"));

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("C"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("D"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("remove"), Brn("A"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("remove"), Brn("C"));

    watchable->RemoveWatcher(*iWatcherUnordered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("close"));

    watchable->AddWatcher(*iWatcherUnordered1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("open"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("B"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("add"), Brn("D"));
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("initialised"));
    iWatcherUnordered1->CheckEmpty();

    watchable->RemoveWatcher(*iWatcherUnordered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    iWatcherUnordered1->CheckNotEmpty();
    iWatcherUnordered1->Pop()->Check(Brn("close"));

    watchable->Dispose();
}


void SuiteWatchable::Test8() // WatchableOrderedThrowsIfDisposedWithWatchersStillActive
{
    iSuccess = false;

    iWatcherOrdered1 = new WatcherOrdered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test8Callback), 0);
    delete iThread;
    iSuccess = true;
}

void SuiteWatchable::Test8Callback(void*)
{
    WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(*iThread);

    watchable->AddWatcher(*iWatcherOrdered1);

    //AssertInvalidOperationException(() => watchable->Dispose());

    watchable->RemoveWatcher(*iWatcherOrdered1);
    watchable->Dispose();
}

//[Test9]
//void SuiteWatchable::WatchableUnorderedThrowsIfDisposedWithWatchersStillActive()
void SuiteWatchable::Test9()
{
    iSuccess = false;
    iWatcherUnordered1 = new WatcherUnordered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test9Callback), 0);
    delete iThread;
    iSuccess = true;
}

void SuiteWatchable::Test9Callback(void*)
{
    WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(*iThread);

    watchable->AddWatcher(*iWatcherUnordered1);

    //AssertInvalidOperationException(() => watchable->Dispose());

    watchable->RemoveWatcher(*iWatcherUnordered1);
    watchable->Dispose();
}
/*

void SuiteWatchable::AssertInvalidOperationException(Action aAction)
{
    TBool asserted = false;

    try
    {
        aAction();
    }
    catch (InvalidOperationException)
    {
        asserted = true;
    }

    ASSERT(asserted);
}
*/

/*
void SuiteWatchable::ReportException(Exception& aException)
{
    ASSERTS();
};
*/


///////////////////////////////////////////////////////////////////////////

TestExceptionReporter::TestExceptionReporter()
    :iCount(0)
    ,iCountStd(0)
{
}


void TestExceptionReporter::Report(Exception& /*aException*/)
{
    iCount++;
}


void TestExceptionReporter::Report(std::exception& /*aException*/)
{
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

///////////////////////////////////////////////////////////////////////////


void TestWatchable()
{
    Runner runner("Watchable tests\n");
    runner.Add(new SuiteWatchable());
    runner.Run();
}



