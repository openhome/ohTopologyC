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
#include <OpenHome/Tests/TestExceptionReporter.h>
#include <exception>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace OpenHome::TestFramework;


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

    TBool Check(const Brx& aType, const Brx& aId, T aValue, T aPrevious)
    {
        TBool success = true;

        success &= (iType.Equals(Brn(aType)));
        success &= (iId.Equals(Brn(aId)));
        success &= (iValue.Equals(aValue));
        success &= (iPrevious.Equals(aPrevious));

        success &= (iTypeModified);
        success &= (iIdModified);
        success &= (iValueModified);
        success &= (iPreviousModified);

        return(success);
    }

    TBool Check(const Brx& aType, const Brx& aId, T aValue)
    {
        TBool success = true;

        success &= (iType.Equals(Brn(aType)));
        success &= (iId.Equals(aId));
        success &= (iValue.Equals(aValue));

        success &= (iTypeModified);
        success &= (iIdModified);
        success &= (iValueModified);

        success &= (!iPreviousModified);
        return(success);
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

    TBool Check(const Brx& aType)
    {
        TBool success = true;

        success &= (iType == Brn(aType));
        success &= (iTypeModified);

        success &= (!iIndexModified);
        success &= (!iValueModified);
        success &= (!iFromModified);
        success &= (!iToModified);
        return(success);
    }

    TBool Check(const Brx& aType, T aValue, TUint aIndex)
    {
        TBool success = true;
        success &= (iType == Brn(aType));
        success &= (iValue.Equals(aValue));
        success &= (iIndex == aIndex);

        success &= (iIndexModified);
        success &= (iTypeModified);
        success &= (iValueModified);

        success &= (!iFromModified);
        success &= (!iToModified);
        return(success);
    }

    TBool Check(const Brx& aType, T aValue, TUint aFrom, TUint aTo)
    {
        TBool success = true;
        success &= (iType == Brn(aType));
        success &= (iValue.Equals(aValue));
        success &= (iFrom == aFrom);
        success &= (iTo == aTo);

        success &= (iTypeModified);
        success &= (iValueModified);
        success &= (iFromModified);
        success &= (iToModified);

        success &= (!iIndexModified);
        return(success);
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

    TBool Check(const Brx& aType)
    {
        TBool success = true;
        success &= (iType.Equals(Brn(aType)));
        success &= (iTypeModified);
        success &= (!iValueModified);
        return(success);
    }

    TBool Check(const Brx& aType, T aValue)
    {
        TBool success = true;
        success &= (iType.Equals(Brn(aType)));
        success &= (iValue.Equals(aValue));
        success &= (iTypeModified);
        success &= (iValueModified);
        return(success);
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

    TBool CheckNotEmpty()
    {
        return(iEvents.SlotsUsed() > 0);
    }

    TBool CheckEmpty()
    {
        return(iEvents.SlotsUsed() == 0);
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

    TBool CheckNotEmpty()
    {
        return(iEvents.SlotsUsed() > 0);
    }

    TBool CheckEmpty()
    {
        return(iEvents.SlotsUsed() == 0);
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
    }

    TBool CheckNotEmpty()
    {
        return(iEvents.SlotsUsed() > 0);
    }

    TBool CheckEmpty()
    {
        return(iEvents.SlotsUsed() == 0);
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
    }

    TBool CheckNotEmpty()
    {
        return(iEvents.SlotsUsed() > 0);
    }

    TBool CheckEmpty()
    {
        return(iEvents.SlotsUsed() == 0);
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
    void Test4(); // TwoWatchablesSingleWatcher
    void Test5(); // SingleWatchableWatchableSingleWatcher
    void Test6(); // SingleWatchableOrderedSingleOrderedWatcher
    void Test7(); // SingleWatchableUnorderedSingleWatcherUnordered
    void Test8(); // WatchableOrderedThrowsIfDisposedWithWatchersStillActive
    void Test9(); // WatchableUnorderedThrowsIfDisposedWithWatchersStillActive
    void Test10(); // WatchableThrowsIfDisposedWithWatchersStillActive



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
    void Test10Callback(void*);



    void DoNothing(void*);
    void AssertInvalidOperationException(IDisposable& aDisposable);




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
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test10));
}


void SuiteWatchable::Setup()
{
    iSuccess = false;
    iExceptionReporter = new TestExceptionReporter();
}


void SuiteWatchable::TearDown()
{
    delete iExceptionReporter;
    TEST(iSuccess);
}


void SuiteWatchable::Test1() // SingleWatchableSingleWatcher
{
    iWatcher1 = new Watcher<Brn>();
    iThread = new WatchableThread(*iExceptionReporter);
    iWatchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateBA) , 0);
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1AddWatcher) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);

    TEST(iWatcher1->CheckNotEmpty());

    auto event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateB) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateC) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateDE) , 0);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C")));
    delete event;
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1RemoveWatcher) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("E")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test1UpdateBA) , 0);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckEmpty());

    TEST(iWatcher1->CheckEmpty());

    delete iThread;
    delete iWatcher1;
    delete iWatchable;

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
    iWatcher1 = new Watcher<Brn>();
    iThread = new WatchableThread(*iExceptionReporter);
    iWatchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test2Callback), 0);
    delete iThread;
    TEST(iWatcher1->CheckEmpty());

    delete iWatcher1;
    delete iWatchable;

    iSuccess = true;
}


void SuiteWatchable::Test2Callback(void*)
{
    TEST(iWatcher1->CheckEmpty());
    iWatchable->AddWatcher(*iWatcher1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    auto event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    iWatchable->Update(Brn("B"));
    iWatchable->Update(Brn("C"));
    iWatchable->RemoveWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("C")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    iWatchable->Update(Brn("D"));
    iWatchable->AddWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("D")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    iWatchable->RemoveWatcher(*iWatcher1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("D")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
}



void SuiteWatchable::DoNothing(void*)
{
    // do nothing
}


void SuiteWatchable::Test3() // SingleWatchableTwoWatchers
{
    iWatcher1 = new Watcher<Brn>();
    iWatcher2 = new Watcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test3Callback), 0);

    delete iThread;

    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    delete iWatcher1;
    delete iWatcher2;

    iSuccess = true;
}


void SuiteWatchable::Test3Callback(void*)
{
    Watchable<Brn>* watchable = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));

    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    watchable->Update(Brn("B"));
    watchable->Update(Brn("A"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    watchable->AddWatcher(*iWatcher1);
    TEST(iWatcher1->CheckNotEmpty());
    auto event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    watchable->Update(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    watchable->AddWatcher(*iWatcher2);
    TEST(iWatcher2->CheckNotEmpty());
    event = iWatcher2->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("B")));
    delete event;
    TEST(iWatcher2->CheckEmpty());
    TEST(iWatcher1->CheckEmpty());

    watchable->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckNotEmpty());
    event = iWatcher2->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWatcher2->CheckEmpty());

    watchable->RemoveWatcher(*iWatcher1);
    watchable->Update(Brn("D"));
    watchable->Update(Brn("E"));
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("C")));
    delete event;
    TEST(iWatcher1->CheckEmpty());
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher2->CheckNotEmpty());
    event = iWatcher2->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C")));
    delete event;
    TEST(iWatcher2->CheckNotEmpty());
    event = iWatcher2->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D")));
    delete event;
    TEST(iWatcher2->CheckEmpty());

    watchable->RemoveWatcher(*iWatcher2);
    TEST(iWatcher2->CheckNotEmpty());
    event = iWatcher2->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("E")));
    delete event;
    TEST(iWatcher2->CheckEmpty());
    TEST(iWatcher1->CheckEmpty());

    watchable->Update(Brn("B"));
    watchable->Update(Brn("A"));

    TEST(iWatcher1->CheckEmpty());
    TEST(iWatcher2->CheckEmpty());

    delete watchable;
}


void SuiteWatchable::Test4() // TwoWatchablesSingleWatcher
{
    iWatcher1 = new Watcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);

    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test4Callback), 0);

    delete iThread;
    TEST(iWatcher1->CheckEmpty());
    delete iWatcher1;
    iSuccess = true;
}


void SuiteWatchable::Test4Callback(void*)
{
    Watchable<Brn>* watchable1 = new Watchable<Brn>(*iThread, Brn("X"), Brn("A"));
    Watchable<Brn>* watchable2 = new Watchable<Brn>(*iThread, Brn("Y"), Brn("A"));

    TEST(iWatcher1->CheckEmpty());

    watchable1->Update(Brn("B"));
    watchable1->Update(Brn("A"));
    watchable2->Update(Brn("B"));
    watchable2->Update(Brn("A"));
    TEST(iWatcher1->CheckEmpty());

    watchable1->AddWatcher(*iWatcher1);
    TEST(iWatcher1->CheckNotEmpty());
    auto event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("X"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable2->AddWatcher(*iWatcher1);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("Y"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable1->Update(Brn("B"));
    watchable2->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("X"), Brn("B"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("Y"), Brn("C"), Brn("A")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable1->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("X"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable1->RemoveWatcher(*iWatcher1);
    watchable1->Update(Brn("D"));
    watchable1->Update(Brn("E"));
    watchable2->Update(Brn("D"));
    watchable2->Update(Brn("E"));
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("X"), Brn("C")));
    delete event;
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("Y"), Brn("D"), Brn("C")));
    delete event;
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("Y"), Brn("E"), Brn("D")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable2->RemoveWatcher(*iWatcher1);
    TEST(iWatcher1->CheckNotEmpty());
    event = iWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("Y"), Brn("E")));
    delete event;
    TEST(iWatcher1->CheckEmpty());

    watchable1->Update(Brn("B"));
    watchable1->Update(Brn("A"));
    watchable2->Update(Brn("B"));
    watchable2->Update(Brn("A"));
    TEST(iWatcher1->CheckEmpty());

    delete watchable1;
    delete watchable2;

}



void SuiteWatchable::Test5() // SingleWatchableWatchableSingleWatcher
{
    iWWatcher1 = new WWatcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test5Callback), 0);

    delete iThread;
    TEST(iWWatcher1->CheckEmpty());

    delete iWWatcher1;
    iSuccess = true;
}


void SuiteWatchable::Test5Callback(void*)
{
    Watchable<Brn>* w = new Watchable<Brn>(*iThread, Brn("test"), Brn("A"));
    Watchable<IWatchable<Brn>*>* ww = new Watchable<IWatchable<Brn>*>(*iThread, Brn("WW"), w);

    TEST(iWWatcher1->CheckEmpty());

    ww->AddWatcher(*iWWatcher1);
    TEST(iWWatcher1->CheckNotEmpty());
    auto event = iWWatcher1->Pop();
    TEST(event->Check(Brn("open"), Brn("test"), Brn("A")));
    delete event;
    TEST(iWWatcher1->CheckEmpty());

    w->Update(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWWatcher1->CheckNotEmpty());
    event = iWWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A")));
    delete event;
    TEST(iWWatcher1->CheckEmpty());

    w->Update(Brn("C"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWWatcher1->CheckNotEmpty());
    event = iWWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B")));
    delete event;
    TEST(iWWatcher1->CheckEmpty());

    w->Update(Brn("D"));
    w->Update(Brn("E"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWWatcher1->CheckNotEmpty());
    event = iWWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("D"), Brn("C")));
    delete event;
    TEST(iWWatcher1->CheckNotEmpty());
    event = iWWatcher1->Pop();
    TEST(event->Check(Brn("update"), Brn("test"), Brn("E"), Brn("D")));
    delete event;
    TEST(iWWatcher1->CheckEmpty());

    ww->RemoveWatcher(*iWWatcher1);
    TEST(iWWatcher1->CheckNotEmpty());
    event = iWWatcher1->Pop();
    TEST(event->Check(Brn("close"), Brn("test"), Brn("E")));
    delete event;
    TEST(iWWatcher1->CheckEmpty());

    w->Update(Brn("B"));
    w->Update(Brn("A"));
    TEST(iWWatcher1->CheckEmpty());

    delete w;
    delete ww;
}


void SuiteWatchable::Test6() // SingleWatchableOrderedSingleOrderedWatcher
{
    iWatcherOrdered1 = new WatcherOrdered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test6Callback), 0);

    delete iThread;
    TEST(iWatcherOrdered1->CheckEmpty());
    delete iWatcherOrdered1;
    iSuccess = true;
}


void SuiteWatchable::Test6Callback(void*)
{
    WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(*iThread);

    TEST(iWatcherOrdered1->CheckEmpty());

    watchable->Add(Brn("A"), 0);
    watchable->Add(Brn("B"), 1);
    TEST(iWatcherOrdered1->CheckEmpty());

    watchable->AddWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherOrdered1->CheckNotEmpty());
    auto event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("open")));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("A"), 0));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("B"), 1));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("initialised")));
    delete event;
    TEST(iWatcherOrdered1->CheckEmpty());

    watchable->Add(Brn("C"), 0);
    watchable->Add(Brn("D"), 1);
    watchable->Move(Brn("C"), 2);
    watchable->Remove(Brn("B"));
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("C"), 0));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("D"), 1));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("move"), Brn("C"), 0, 2));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("remove"), Brn("B"), 3));
    delete event;
    TEST(iWatcherOrdered1->CheckEmpty());

    watchable->RemoveWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("close")));
    delete event;

    watchable->AddWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("open")));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("D"), 0));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("A"), 1));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("add"), Brn("C"), 2));
    delete event;
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("initialised")));
    delete event;
    TEST(iWatcherOrdered1->CheckEmpty());

    watchable->RemoveWatcher(*iWatcherOrdered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherOrdered1->CheckNotEmpty());
    event = iWatcherOrdered1->Pop();
    TEST(event->Check(Brn("close")));
    delete event;

    watchable->Dispose();

    delete watchable;
}


void SuiteWatchable::Test7() // SingleWatchableUnorderedSingleWatcherUnordered
{
    iWatcherUnordered1 = new WatcherUnordered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test7Callback), 0);

    delete iThread;
    TEST(iWatcherUnordered1->CheckEmpty());
    delete iWatcherUnordered1;
    iSuccess = true;
}


void SuiteWatchable::Test7Callback(void*)
{
    WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(*iThread);

    TEST(iWatcherUnordered1->CheckEmpty());

    watchable->Add(Brn("A"));
    watchable->Add(Brn("B"));
    TEST(iWatcherUnordered1->CheckEmpty());

    watchable->AddWatcher(*iWatcherUnordered1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherUnordered1->CheckNotEmpty());
    auto event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("open")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("A")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("B")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("initialised")));
    delete event;
    TEST(iWatcherUnordered1->CheckEmpty());

    watchable->Add(Brn("C"));
    watchable->Add(Brn("D"));
    watchable->Remove(Brn("A"));
    watchable->Remove(Brn("C"));

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("C")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("D")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("remove"), Brn("A")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("remove"), Brn("C")));
    delete event;

    watchable->RemoveWatcher(*iWatcherUnordered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("close")));
    delete event;

    watchable->AddWatcher(*iWatcherUnordered1);

    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("open")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("B")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("add"), Brn("D")));
    delete event;
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("initialised")));
    delete event;
    TEST(iWatcherUnordered1->CheckEmpty());

    watchable->RemoveWatcher(*iWatcherUnordered1);
    iThread->Execute(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::DoNothing), 0);
    TEST(iWatcherUnordered1->CheckNotEmpty());
    event = iWatcherUnordered1->Pop();
    TEST(event->Check(Brn("close")));
    delete event;

    watchable->Dispose();
    delete watchable;
}


void SuiteWatchable::Test8() // WatchableOrderedThrowsIfDisposedWithWatchersStillActive
{
    iWatcherOrdered1 = new WatcherOrdered<Brn>();
    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test8Callback), 0);
    delete iThread;
    delete iWatcherOrdered1;
    iSuccess = true;
}

void SuiteWatchable::Test8Callback(void*)
{
    WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(*iThread);
    watchable->AddWatcher(*iWatcherOrdered1);

    TEST_THROWS(watchable->Dispose(), AssertionFailed);

    watchable->RemoveWatcher(*iWatcherOrdered1);

    // clean up
    auto event = iWatcherOrdered1->Pop();
    delete event;
    event = iWatcherOrdered1->Pop();
    delete event;
    event = iWatcherOrdered1->Pop();
    delete event;

    watchable->Dispose();
    delete watchable;
}



void SuiteWatchable::Test9() // WatchableUnorderedThrowsIfDisposedWithWatchersStillActive
{
    iWatcherUnordered1 = new WatcherUnordered<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test9Callback), 0);
    delete iThread;
    delete iWatcherUnordered1;
    iSuccess = true;
}

void SuiteWatchable::Test9Callback(void*)
{

    WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(*iThread);
    watchable->AddWatcher(*iWatcherUnordered1);

    TEST_THROWS(watchable->Dispose(), AssertionFailed);

    watchable->RemoveWatcher(*iWatcherUnordered1);

    // clean up
    auto event = iWatcherUnordered1->Pop();
    delete event;
    event = iWatcherUnordered1->Pop();
    delete event;
    event = iWatcherUnordered1->Pop();
    delete event;

    watchable->Dispose();

    delete watchable;

}



void SuiteWatchable::Test10() // WatchableThrowsIfDisposedWithWatchersStillActive
{
    iWatcher1 = new Watcher<Brn>();

    iThread = new WatchableThread(*iExceptionReporter);
    iThread->Schedule(MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test10Callback), 0);
    delete iThread;
    delete iWatcher1;
    iSuccess = true;

}

void SuiteWatchable::Test10Callback(void*)
{

    Watchable<Brn>* watchable = new Watchable<Brn>(*iThread, Brn("TestId"), Brn("TestValue"));
    watchable->AddWatcher(*iWatcher1);

    TEST_THROWS(watchable->Dispose(), AssertionFailed);

    watchable->RemoveWatcher(*iWatcher1);

    // clean up
    auto event = iWatcher1->Pop();
    delete event;
    event = iWatcher1->Pop();
    delete event;

    watchable->Dispose();
    delete watchable;

}

///////////////////////////////////////////////////////////////////////////

void TestWatchable()
{
    Runner runner("Watchable tests\n");
    runner.Add(new SuiteWatchable());
    runner.Run();
}



