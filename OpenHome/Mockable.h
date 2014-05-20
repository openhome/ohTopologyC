#ifndef HEADER_MOCKABLE
#define HEADER_MOCKABLE


#include <OpenHome/OhNetTypes.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <OpenHome/Private/Ascii.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{


static const TUint kMaxResultBytes = 5000;

///////////////////////////////////////////////

class Mockable : public IMockable
{
public:
    Mockable();

    void Add(const Brx& aId, IMockable& aMockable);
    void Remove(const Brx& aId);

    // IMockable
    void Execute(ICommandTokens& aTokens);

private:
     std::map<Brn, IMockable*, BufferCmp> iMockables;
};

///////////////////////////////////////////////

class MockableScriptRunner
{
private:
    static const TUint kMaxFifoEntries = 1000;
    static const TUint kMaxLineBytes = 5000;


private:
    class AssertError : public Exception
    {
    public :
        AssertError() : Exception("ASSERT") { }
    };

public:
    MockableScriptRunner();
    TBool Run(Functor aWait, IReader& aStream, IMockable& aMockable);
    void Result(Bwh* aValue);

private:
    TBool Test(const Brx& aActual, const Brx& aExpected);
    void Assert(TBool aExpression);

    Fifo<Bwh*> iResultQueue;
    Bws<kMaxLineBytes> iLine;
};

//////////////////////////////////////////////

class ResultWatcherFactory : public IDisposable, public INonCopyable
{
public:
    ResultWatcherFactory(MockableScriptRunner& aRunner);

    template <class T>
    void Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    template <class T>
    void Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    template <class T>
    void Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    void Destroy(const Brx& aId);

    // IDisposable
    void Dispose();

private:
    MockableScriptRunner& iRunner;
    std::map<Brn, std::vector<IDisposable*>, BufferCmp> iWatchers;
};

/////////////////////////////////////////////////////////////


template <class T>
class ResultWatcher : public IWatcher<T>, public IDisposable, public INonCopyable
{
public:
    ResultWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    // IWatcher<T>
    void ItemOpen(const Brx& aId, T aValue);
    void ItemUpdate(const Brx& aId, T aValue, T aPrevious);
    void ItemClose(const Brx& aId, T aValue);

    // IDisposable
    void Dispose();

private:
    void ItemOpenCallback(const Brx& aId);
    void ItemUpdateCallback(const Brx& aId);
    void ItemCloseCallback(const Brx& aId);

private:
    MockableScriptRunner& iRunner;
    Brn iId;
    IWatchable<T>& iWatchable;
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> iAction;

    Bws<kMaxResultBytes> iBuf;
};

//////////////////////////////////////////////////////

template <class T>
class ResultUnorderedWatcher : public IWatcherUnordered<T>, public IDisposable, public INonCopyable
{
public:
    ResultUnorderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    // IUnorderedWatcher<T>
    void UnorderedOpen();
    void UnorderedInitialised();
    void UnorderedAdd(T aItem);
    void UnorderedRemove(T aItem);
    void UnorderedClose();

    // IDisposable
    void Dispose();

private:
    void UnorderedAddCallback(const Brx& aValue);
    void UnorderedRemoveCallback(const Brx& aValue);


private:
    MockableScriptRunner& iRunner;
    Brn iId;
    IWatchableUnordered<T>& iWatchable;
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> iAction;

	Bws<kMaxResultBytes> iBuf;
};

/////////////////////////////////////////////////

template <class T>
class ResultOrderedWatcher : public IWatcherOrdered<T>, public IDisposable, public INonCopyable
{
public:
    ResultOrderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction);

    // IWatcherOrdered<T>
    void OrderedOpen();
    void OrderedInitialised();
    void OrderedAdd(T aItem, TUint aIndex);
    void OrderedMove(T aItem, TUint aFrom, TUint aTo);
    void OrderedRemove(T aItem, TUint aIndex);
    void OrderedClose();

    // IDisposable
    void Dispose();

private:
    void OrderedAddCallback(const Brx& aValue);
    void OrderedMoveCallback(const Brx& aValue);
    void OrderedRemoveCallback(const Brx& aValue);


private:
    MockableScriptRunner& iRunner;
    Brn iId;
    IWatchableOrdered<T>& iWatchable;
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> iAction;

	Bws<kMaxResultBytes> iBuf;
};

//////////////////////////////////////////////


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultWatcher<T>(iRunner, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultUnorderedWatcher<T>(iRunner, aId, aWatchable, aAction));
}


template <class T>
void ResultWatcherFactory::Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
{
    Brn id(aId);

    if (iWatchers.count(id)==0)
    {
        std::vector<IDisposable*> watchers;
        iWatchers[id] = watchers;
    }

    iWatchers[id].push_back(new ResultOrderedWatcher<T>(iRunner, aId, aWatchable, aAction));
}




/////////////////////////////////////////////////////////////////////////////////////


template <class T>
ResultWatcher<T>::ResultWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultWatcher<T>::ItemOpen(const Brx& /*aId*/, T aValue)
{
    // ignoring aId - we're using iId
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemOpenCallback);
    ArgsTwo<T, FunctorGeneric<const Brx&>>* args = new ArgsTwo<T, FunctorGeneric<const Brx&>>(aValue, f);
    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemOpenCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" open "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}


template <class T>
void ResultWatcher<T>::ItemUpdate(const Brx& /*aId*/, T aValue, T /*aPrevious*/)
{
    // ignoring aId - we're using iId
    // ignoring aPrevious - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemUpdateCallback);
    ArgsTwo<T, FunctorGeneric<const Brx&>>* args = new ArgsTwo<T, FunctorGeneric<const Brx&>>(aValue, f);
    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemUpdateCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" update "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}



template <class T>
void ResultWatcher<T>::ItemClose(const Brx& /*aId*/, T aValue)
{
    // ignoring aId - we're using iId
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultWatcher::ItemCloseCallback);
    ArgsTwo<T, FunctorGeneric<const Brx&>>* args = new ArgsTwo<T, FunctorGeneric<const Brx&>>(aValue, f);
    iAction(args);
}


template <class T>
void ResultWatcher<T>::ItemCloseCallback(const Brx& /*aValue*/)
{

}


template <class T>
void ResultWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

////////////////////////////////////////////////////////////


template <class T>
ResultUnorderedWatcher<T>::ResultUnorderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedOpen()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedInitialised()
{
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedAdd(T aItem)
{
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedAddCallback);
    ArgsTwo<T, FunctorGeneric<const Brx&>>* args = new ArgsTwo<T, FunctorGeneric<const Brx&>>(aItem, f);
    iAction(args);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedAddCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" add "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemove(T aItem)
{
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultUnorderedWatcher::UnorderedRemoveCallback);
    ArgsTwo<T, FunctorGeneric<const Brx&>>* args = new ArgsTwo<T, FunctorGeneric<const Brx&>>(aItem, f);
    iAction(args);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedRemoveCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" remove "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}


template <class T>
void ResultUnorderedWatcher<T>::UnorderedClose()
{
}


template <class T>
void ResultUnorderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

//////////////////////////////////////////////////////////////////////////////


template <class T>
ResultOrderedWatcher<T>::ResultOrderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>*> aAction)
    :iRunner(aRunner)
    ,iId(aId)
    ,iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.AddWatcher(*this);
}


template <class T>
void ResultOrderedWatcher<T>::OrderedOpen()
{
}



template <class T>
void ResultOrderedWatcher<T>::OrderedInitialised()
{
}


template <class T>
void ResultOrderedWatcher<T>::OrderedAdd(T aItem, TUint aIndex)
{
    // Ignoring aIndex - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedAddCallback);
    iAction(aItem, f);

}


template <class T>
void ResultOrderedWatcher<T>::OrderedAddCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" add "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedMove(T aItem, TUint aFrom, TUint aTo)
{
    iResultMoveBuf.Replace(iId);
    iResultMoveBuf.Append(Brn(" moved from "));
    Ascii::AppendDec(iResultMoveBuf, aFrom);
    iResultMoveBuf.Append(Brn(" to "));
    Ascii::AppendDec(iResultMoveBuf, aTo);
    iResultMoveBuf.Append(Brn(" "));
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedMoveCallback);
    iAction(aItem, f);

}



template <class T>
void ResultOrderedWatcher<T>::OrderedMoveCallback(const Brx& aValue)
{
    iResultMoveBuf.Append(aValue);
    Bwh* result = new Bwh(iResultMoveBuf);
    iRunner.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedRemove(T aItem, TUint aIndex)
{
    // Ignoring aIndex - not used
    FunctorGeneric<const Brx&> f = MakeFunctorGeneric(*this, &ResultOrderedWatcher::OrderedRemoveCallback);
    iAction(aItem, f);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedRemoveCallback(const Brx& aValue)
{
    iBuf.Replace(iId);
    iBuf.Append(Brn(" remove "));
    iBuf.Append(aValue);
    Bwh* result = new Bwh(iBuf);
    iRunner.Result(result);
}



template <class T>
void ResultOrderedWatcher<T>::OrderedClose()
{
}



template <class T>
void ResultOrderedWatcher<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}

//////////////////////////////////////////////////////////////////////////////////


} // namespace Av

} // namespace OpenHome

#endif // HEADER_MOCKABLE

