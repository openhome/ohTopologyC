#ifndef HEADER_MOCKABLE
#define HEADER_MOCKABLE


#include <OpenHome/OhNetTypes.h>
//#include <OpenHome/WatchableThread.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/IWatchable.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Device.h>
#include <OpenHome/Buffer.h>
#include <vector>
#include <map>


namespace OpenHome
{

namespace Av
{


static const TUint kMaxResultBytes = 2000;

///////////////////////////////////////////////

class IMockable
{
public:
    virtual void Execute(ICommandTokens& aTokens) = 0;
};

///////////////////////////////////////////////

class Mockable : public IMockable
{
public:
    Mockable();

    void Add(const Brx& aId, IMockable& aMockable);
    void Remove(const Brx& aId);
    void Execute(ICommandTokens& aTokens);

private:
     std::map<Brn, IMockable*, BufferCmp> iMockables;
};

///////////////////////////////////////////////

class MockableStream
{
public:
    MockableStream(IReader& aTextReader, IMockable& aMockable);
    void Start();

private:
    IReader& iReader;
    IMockable& iMockable;
};

///////////////////////////////////////////////

class MockableScriptRunner
{
private:
    static const TUint kMaxFifoEntries = 10;
    static const TUint kMaxLineBytes = 2000;


private:
    class AssertError : public Exception
    {
    public :
        AssertError() : Exception("ASSERT") { }
    };

public:
    MockableScriptRunner();
    void Run(FunctorGeneric<void*> aWait, IReader& aStream, IMockable& aMockable);
    void Result(const Brx& aValue);

private:
    void Assert(const Brx& aActual, const Brx& aExpected);
    void Assert(TBool aExpression);


    //Queue<string> iResultQueue;
    Fifo<Brn> iResultQueue;
    Bws<kMaxLineBytes> iLine;
};

//////////////////////////////////////////////

template <class T>
class ResultWatcherFactory : public IDisposable
{
private:
    MockableScriptRunner iRunner;
    std::map<Brn, std::vector<IDisposable*>, BufferCmp> iWatchers;

public:
    ResultWatcherFactory(MockableScriptRunner& aRunner);
    //void Create(const Brx& aId, IWatchable<T>& aWatchable, Action<T, Action<const Brx&>> aAction);
    //void Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, Action<T, Action<const Brx&>> aAction);
    //void Create(const Brx& aId, IWatchableOrdered<T>& aWatchable, Action<T, Action<const Brx&>> aAction);
    void Create(const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction);
    void Create(const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction);
    void Create(const Brx& aId, IWatchableOrdered<T>& aWatchable,FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction);

    void Destroy(const Brx& aId);

    // IDisposable
    void Dispose();
};

/////////////////////////////////////////////////////////////

template <class T>
class ResultWatcher : public IWatcher<T>, public IDisposable
{
public:
    ResultWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchable<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction);

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
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> iAction;

    //static const TUint kMaxResultBytes = 2000;

    Bws<kMaxResultBytes> iResultOpenBuf;
    Bws<kMaxResultBytes> iResultUpdateBuf;
    Bws<kMaxResultBytes> iResultCloseBuf;
};

//////////////////////////////////////////////////////

template <class T>
class ResultUnorderedWatcher : public IWatcherUnordered<T>, public IDisposable
{
public:
    ResultUnorderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableUnordered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> aAction);
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
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> iAction;

    Bws<kMaxResultBytes> iResultAddBuf;
    Bws<kMaxResultBytes> iResultRemoveBuf;
};

/////////////////////////////////////////////////

template <class T>
class ResultOrderedWatcher : public IWatcherOrdered<T>, public IDisposable
{
public:
    ResultOrderedWatcher(MockableScriptRunner& aRunner, const Brx& aId, IWatchableOrdered<T>& aWatchable, FunctorGeneric<ArgsTwo<T, FunctorGeneric<void*>>> aAction);

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
    FunctorGeneric<ArgsTwo<T, FunctorGeneric<const Brx&>>> iAction;

    Bws<kMaxResultBytes> iResultAddBuf;
    Bws<kMaxResultBytes> iResultMoveBuf;
    Bws<kMaxResultBytes> iResultRemoveBuf;
};

//////////////////////////////////////////////

class MocakbleExtensions
{
public:
    static void Execute(IMockable& aMockable, const Brx& aValue);
};

}

}


#endif

