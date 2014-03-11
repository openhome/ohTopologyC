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

    void Check(const Brx& aType, T aValue, int aIndex)
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
    {
        iEvents = new Fifo<WatcherEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherEvent<T> Pop()
    {
        return (iEvents.Read());
    }

    // IWatcher<T>
    void ItemOpen(Brn aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>("open", aId, aValue));
    }

    void ItemUpdate(Brn aId, T aValue, T aPrevious)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>("update", aId, aValue, aPrevious));
    }

    void ItemClose(Brn aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>("close", aId, aValue));
    }

    // IWatcher<IWatchable<T>>

    void ItemOpen(Brn aId, IWatchable<T>& aValue)
    {
        aValue.AddWatcher(this);
    }

    void ItemUpdate(Brn aId, IWatchable<T>& aValue, IWatchable<T>& aPrevious)
    {
        aPrevious.RemoveWatcher(this);
        aValue.AddWatcher(this);
    }

    void ItemClose(Brn aId, IWatchable<T>& aValue)
    {
        aValue.RemoveWatcher(this);
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
    Watcher() :iMutex("WRMX"), iEvents(10)
    {
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed()== 0);
    }

    WatcherEvent<T>* Pop()
    {
        return (iEvents.Read());
    }

    // IWatcher<T>

    void ItemOpen(const Brx& aId, T aValue)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("open"), aId, aValue));
    }

    void ItemUpdate(const Brx& aId, T aValue, T aPrevious)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherEvent<T>(Brn("update"), aId, aValue, aPrevious));
    }

    void ItemClose(const Brx& aId, T aValue)
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
    {
        iEvents = new Fifo<WatcherOrderedEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherOrderedEvent<T> Pop()
    {
        return (iEvents.Read());
    }

    // IOrderedWatcher<T>

    void OrderedOpen()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("open")));
    }

    void OrderedInitialised()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("initialised")));
    }

    void OrderedAdd(T aItem, TUint aIndex)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("add"), aItem, aIndex));
    }

    void OrderedMove(T aItem, TUint aFrom, TUint aTo)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("move"), aItem, aFrom, aTo));
    }

    void OrderedRemove(T aItem, TUint aIndex)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherOrderedEvent<T>(Brn("remove"), aItem, aIndex));
    }

    void OrderedClose()
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

    WatcherUnordered()
        :iMutex("WOMX")
    {
        iEvents = new Fifo<WatcherUnorderedEvent<T>*>();
    }

    void CheckNotEmpty()
    {
        ASSERT(iEvents.SlotsUsed() > 0);
    }

    void CheckEmpty()
    {
        ASSERT(iEvents.SlotsUsed() == 0);
    }

    WatcherUnorderedEvent<T> Pop()
    {
        return (iEvents.Read());
    }

    // IWatcherUnordered<T>

    void UnorderedOpen()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("open")));
    }

    void UnorderedInitialised()
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("initialised")));
    }

    void UnorderedAdd(T aItem)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("add"), aItem));
    }

    void UnorderedRemove(T aItem)
    {
        AutoMutex mutex(iMutex);
        iEvents.Write(new WatcherUnorderedEvent<T>(Brn("remove"), aItem));
    }

    void UnorderedClose()
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
    //void Test1();
    void Test2();
/*
    void Test3();
    void Test4();
    void Test5();
    void Test6();
    void Test7();
    void Test8();
    void Test9();
*/

    void Test2Schedule(void*);
    void Test2Execute(void*);


private:
    Watcher<Brn>* iWatcherT2;
    WatchableThread* iThreadT2;
};




} // Av

} // OpenHome

/////////////////////////////////////////////////////////////



/*

//[Test1]
//void SuiteWatchable::SingleWatchableSingleWatcher()
void SuiteWatchable::Test1()
{
    Watcher<Brn>* watcher = new Watcher<Brn>();

    //using (WatchableThread* thread = new WatchableThread(ReportException))
    //{

        WatchableThread* thread = new WatchableThread(ReportException)

        Watchable<Brn>* watchable = new Watchable<Brn>(thread, "test", "A");

        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->Update("B");
            watchable->Update("A");
        });

        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->AddWatcher(watcher);
        });

        thread->Execute();

        watcher->CheckNotEmpty();
        watcher->Pop()->Check("open", "test", "A");
        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->Update("B");
        });

        thread->Execute();
        watcher->CheckNotEmpty();
        watcher->Pop()->Check("update", "test", "B", "A");
        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->Update("C");
        });

        thread->Execute();
        watcher->CheckNotEmpty();
        watcher->Pop()->Check("update", "test", "C", "B");
        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->Update("D");
            watchable->Update("E");
        });

        thread->Execute();
        watcher->CheckNotEmpty();
        watcher->Pop()->Check("update", "test", "D", "C");
        watcher->CheckNotEmpty();
        watcher->Pop()->Check("update", "test", "E", "D");
        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->RemoveWatcher(watcher);
        });

        thread->Execute();
        watcher->CheckNotEmpty();
        watcher->Pop()->Check("close", "test", "E");
        watcher->CheckEmpty();

        thread->Schedule(() =>
        {
            watchable->Update("B");
            watchable->Update("A");
        });

        thread->Execute();
        watcher->CheckEmpty();

        watcher->CheckEmpty();
    //}
}

*/


//[Test2]
//void SuiteWatchable::RemoveWatcherFlushesCallbacks()
void SuiteWatchable::Test2()
{
    iWatcherT2 = new Watcher<Brn>();

//    using (WatchableThread* thread = new WatchableThread(ReportException))
//    {

    TestExceptionReporter* xReporter = new TestExceptionReporter();
    iThreadT2 =  new WatchableThread(*xReporter);

    FunctorGeneric<void*> f = MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test2Schedule);
    iThreadT2->Schedule(f, 0);

    iWatcherT2->CheckEmpty();
}


void SuiteWatchable::Test2Schedule(void*)
{
    ASSERTS();

    Watchable<Brn>* watchable = new Watchable<Brn>(*iThreadT2, Brn("test"), Brn("A"));

    iWatcherT2->CheckEmpty();

    watchable->AddWatcher(*iWatcherT2);

    FunctorGeneric<void*> f = MakeFunctorGeneric<void*>(*this, &SuiteWatchable::Test2Execute);

    iThreadT2->Execute(f, 0);
    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("open"), Brn("test"), Brn("A"));
    iWatcherT2->CheckEmpty();

    watchable->Update(Brn("B"));
    watchable->Update(Brn("C"));
    watchable->RemoveWatcher(*iWatcherT2);

    iThreadT2->Execute(f, 0);

    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("update"), Brn("test"), Brn("B"), Brn("A"));
    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("update"), Brn("test"), Brn("C"), Brn("B"));
    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("close"), Brn("test"), Brn("C"));
    iWatcherT2->CheckEmpty();

    watchable->Update(Brn("D"));
    watchable->AddWatcher(*iWatcherT2);
    iThreadT2->Execute(f, 0);
    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("open"), Brn("test"), Brn("D"));
    iWatcherT2->CheckEmpty();

    watchable->RemoveWatcher(*iWatcherT2);
    iThreadT2->Execute(f, 0);
    iWatcherT2->CheckNotEmpty();
    iWatcherT2->Pop()->Check(Brn("close"), Brn("test"), Brn("D"));
    iWatcherT2->CheckEmpty();
}



void SuiteWatchable::Test2Execute(void*)
{
    // do nothing
}


/*

//[Test3]
//void SuiteWatchable::SingleWatchableTwoWatchers()
void SuiteWatchable::Test3()
{
    Watcher<Brn>* watcher1 = new Watcher<Brn>();
    Watcher<Brn>* watcher2 = new Watcher<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            Watchable<Brn>* watchable = new Watchable<Brn>(thread, "test", "A");

            watcher1.CheckEmpty();
            watcher2.CheckEmpty();

            watchable->Update("B");
            watchable->Update("A");
            thread->Execute();
            watcher1.CheckEmpty();
            watcher2.CheckEmpty();

            watchable->AddWatcher(watcher1);
            watcher1.CheckNotEmpty();
            watcher1.Pop()->Check("open", "test", "A");
            watcher1.CheckEmpty();
            watcher2.CheckEmpty();

            watchable->Update("B");
            thread->Execute();
            watcher1.CheckNotEmpty();
            watcher1.Pop()->Check("update", "test", "B", "A");
            watcher1.CheckEmpty();
            watcher2.CheckEmpty();

            watchable->AddWatcher(watcher2);
            watcher2.CheckNotEmpty();
            watcher2.Pop()->Check("open", "test", "B");
            watcher2.CheckEmpty();
            watcher1.CheckEmpty();

            watchable->Update("C");
            thread->Execute();
            watcher1.CheckNotEmpty();
            watcher1.Pop()->Check("update", "test", "C", "B");
            watcher1.CheckEmpty();
            watcher2.CheckNotEmpty();
            watcher2.Pop()->Check("update", "test", "C", "B");
            watcher2.CheckEmpty();

            watchable->RemoveWatcher(watcher1);
            watchable->Update("D");
            watchable->Update("E");
            watcher1.CheckNotEmpty();
            watcher1.Pop()->Check("close", "test", "C");
            watcher1.CheckEmpty();
            thread->Execute();
            watcher2.CheckNotEmpty();
            watcher2.Pop()->Check("update", "test", "D", "C");
            watcher2.CheckNotEmpty();
            watcher2.Pop()->Check("update", "test", "E", "D");
            watcher2.CheckEmpty();

            watchable->RemoveWatcher(watcher2);
            watcher2.CheckNotEmpty();
            watcher2.Pop()->Check("close", "test", "E");
            watcher2.CheckEmpty();
            watcher1.CheckEmpty();

            watchable->Update("B");
            watchable->Update("A");
            watcher1.CheckEmpty();
            watcher2.CheckEmpty();
        });
    }

    watcher1.CheckEmpty();
    watcher2.CheckEmpty();
}

//[Test4]
//void SuiteWatchable::TwoWatchablesSingleWatcher()
void SuiteWatchable::Test4()
{
    Watcher<Brn>* watcher = new Watcher<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            Watchable<Brn>* watchable1 = new Watchable<Brn>(thread, "X", "A");
            Watchable<Brn>* watchable2 = new Watchable<Brn>(thread, "Y", "A");

            watcher->CheckEmpty();

            watchable1.Update("B");
            watchable1.Update("A");
            watchable2.Update("B");
            watchable2.Update("A");
            watcher->CheckEmpty();

            watchable1.AddWatcher(watcher);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open", "X", "A");
            watcher->CheckEmpty();

            watchable2.AddWatcher(watcher);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open", "Y", "A");
            watcher->CheckEmpty();

            watchable1.Update("B");
            watchable2.Update("C");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "X", "B", "A");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "Y", "C", "A");
            watcher->CheckEmpty();

            watchable1.Update("C");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "X", "C", "B");
            watcher->CheckEmpty();

            watchable1.RemoveWatcher(watcher);
            watchable1.Update("D");
            watchable1.Update("E");
            watchable2.Update("D");
            watchable2.Update("E");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close", "X", "C");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "Y", "D", "C");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "Y", "E", "D");
            watcher->CheckEmpty();

            watchable2.RemoveWatcher(watcher);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close", "Y", "E");
            watcher->CheckEmpty();

            watchable1.Update("B");
            watchable1.Update("A");
            watchable2.Update("B");
            watchable2.Update("A");
            watcher->CheckEmpty();

        });
    }

    watcher->CheckEmpty();
}

//[Test5]
//void SuiteWatchable::SingleWatchableWatchableSingleWatcher()
void SuiteWatchable::Test5()
{
    WWatcher<Brn>* watcher = new WWatcher<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            Watchable<Brn>* w = new Watchable<Brn>(thread, "test", "A");
            Watchable<IWatchable<Brn>*>* ww = new Watchable<IWatchable<Brn>*>(thread, "WW", w);

            watcher->CheckEmpty();

            ww.AddWatcher(watcher);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open", "test", "A");
            watcher->CheckEmpty();

            w.Update("B");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "test", "B", "A");
            watcher->CheckEmpty();

            w.Update("C");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "test", "C", "B");
            watcher->CheckEmpty();

            w.Update("D");
            w.Update("E");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "test", "D", "C");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("update", "test", "E", "D");
            watcher->CheckEmpty();

            ww.RemoveWatcher(watcher);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close", "test", "E");
            watcher->CheckEmpty();

            w.Update("B");
            w.Update("A");
            watcher->CheckEmpty();
        });
    }

    watcher->CheckEmpty();
}

//[Test6]
//void SuiteWatchable::SingleWatchableOrderedSingleOrderedWatcher()
void SuiteWatchable::Test6()
{
    var watcher = new OrderedWatcher<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(thread);

            watcher->CheckEmpty();

            watchable->Add("A", 0);
            watchable->Add("B", 1);
            watcher->CheckEmpty();

            watchable->AddWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "A", 0);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "B", 1);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("initialised");
            watcher->CheckEmpty();

            watchable->Add("C", 0);
            watchable->Add("D", 1);
            watchable->Move("C", 2);
            watchable->Remove("B");
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "C", 0);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "D", 1);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("move", "C", 0, 2);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("remove", "B", 3);
            watcher->CheckEmpty();

            watchable->RemoveWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close");

            watchable->AddWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "D", 0);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "A", 1);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "C", 2);
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("initialised");
            watcher->CheckEmpty();

            watchable->RemoveWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close");

            watchable->Dispose();
        });
    }

    watcher->CheckEmpty();
}

//[Test7]
//void SuiteWatchable::SingleWatchableUnorderedSingleWatcherUnordered()
void SuiteWatchable::Test7()
{
    WatcherUnordered<Brn>* watcher = new WatcherUnordered<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(thread);

            watcher->CheckEmpty();

            watchable->Add("A");
            watchable->Add("B");
            watcher->CheckEmpty();

            watchable->AddWatcher(watcher);

            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "A");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "B");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("initialised");
            watcher->CheckEmpty();

            watchable->Add("C");
            watchable->Add("D");
            watchable->Remove("A");
            watchable->Remove("C");

            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "C");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "D");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("remove", "A");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("remove", "C");

            watchable->RemoveWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close");

            watchable->AddWatcher(watcher);

            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("open");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "B");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("add", "D");
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("initialised");
            watcher->CheckEmpty();

            watchable->RemoveWatcher(watcher);
            thread->Execute();
            watcher->CheckNotEmpty();
            watcher->Pop()->Check("close");

            watchable->Dispose();
        });
    }

    watcher->CheckEmpty();
}

//[Test8]
//void SuiteWatchable::WatchableOrderedThrowsIfDisposedWithWatchersStillActive()
void SuiteWatchable::Test8()
{
    OrderedWatcher<Brn>* watcher = new OrderedWatcher<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {

            WatchableOrdered<Brn>* watchable = new WatchableOrdered<Brn>(thread);

            watchable->AddWatcher(watcher);

            AssertInvalidOperationException(() => watchable->Dispose());

            watchable->RemoveWatcher(watcher);
            watchable->Dispose();
        });
    }
}

//[Test9]
//void SuiteWatchable::WatchableUnorderedThrowsIfDisposedWithWatchersStillActive()
void SuiteWatchable::Test9()
{
    WatcherUnordered<Brn>* watcher = new WatcherUnordered<Brn>();

    using (WatchableThread* thread = new WatchableThread(ReportException))
    {
        thread->Schedule(() =>
        {
            WatchableUnordered<Brn>* watchable = new WatchableUnordered<Brn>(thread);

            watchable->AddWatcher(watcher);

            AssertInvalidOperationException(() => watchable->Dispose());

            watchable->RemoveWatcher(watcher);
            watchable->Dispose();
        });
    }
}

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


void SuiteWatchable::ReportException(Exception& aException)
{
    ASSERTS();
};
*/

SuiteWatchable::SuiteWatchable()
    :SuiteUnitTest("SuiteWatchable")
{
    AddTest(MakeFunctor(*this, &SuiteWatchable::Test2));
}


void SuiteWatchable::Setup()
{

}


void SuiteWatchable::TearDown()
{

}

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



