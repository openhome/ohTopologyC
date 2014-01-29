#ifndef HEADER_WATCHER
#define HEADER_WATCHER

#include <OpenHome/IWatchable.h>
#include <OpenHome/Watcher.h>


namespace OpenHome
{

namespace Av
{


template <class T>
class Watcher : public IWatcher<T>//, IDisposable
{
public:
    Watcher(IWatchable<T>& aWatchable, Action aAction);
    //Watcher(IWatchable<T> aWatchable, Action aAction);

    // IWatcher<T>
    virtual void ItemOpen(const Brx& aId, T aValue);
    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious);
    virtual void ItemClose(const Brx& aId, T aValue);

    // IDisposable
    virtual void Dispose();

private:
    void Watch(void*);
    void DisposeCB(void*);

private:
    IWatchable<T>& iWatchable;
    Action iAction;
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class WatcherUnordered : public IWatcherUnordered<T>//, public IDisposable
{
public:
    WatcherUnordered(IWatchableUnordered<T>& aWatchable, Action aAction);

    // IUnorderedWatcher<T>
    virtual void UnorderedOpen();
    virtual void UnorderedInitialised();
    virtual void UnorderedAdd(T aItem);
    virtual void UnorderedRemove(T aItem);
    virtual void UnorderedClose();

    // IDisposable
    virtual void Dispose();

private:
    void Watch(void*);
    void DisposeCB(void*);

private:
    IWatchableUnordered<T>& iWatchable;
    Action iAction;
    TBool iInitialised;
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class WatcherOrdered : public IWatcherOrdered<T>//, public IDisposable
{
public:
    WatcherOrdered(IWatchableOrdered<T>& aWatchable, Action aAction);

    // IOrderedWatcher<T>
    virtual void OrderedOpen();
    virtual void OrderedInitialised();
    virtual void OrderedAdd(T aItem, TUint aIndex);
    virtual void OrderedMove(T aItem, TUint aFrom, TUint aTo);
    virtual void OrderedRemove(T aItem, TUint aIndex);
    virtual void OrderedClose();

    // IDisposable
    virtual void Dispose();

private:
    void Watch(void*);
    void DisposeCB(void*);

private:
    IWatchableOrdered<T>& iWatchable;
    Action iAction;
    TBool iInitialised;
};

/////////////////////////////////////////////////////////////////////////////////////////

/*
class WatcherExtensions
{
public:
    static IDisposable CreateWatcher<T>(this IWatchable<T> aWatchable, Action<T> aAction);
    static IDisposable CreateWatcher<T>(this IWatchable<T> aWatchable, Action<T, TBool> aAction);
    static IDisposable CreateWatcher<T>(this IWatchableUnordered<T> aWatchable, Action<IEnumerable<T>> aAction);
    static IDisposable CreateWatcher<T>(this IWatchableOrdered<T> aWatchable, Action<IEnumerable<T>> aAction);
};
*/

} // namespace Av

} // namespace OpenHome

#endif // HEADER_WATCHER
