#pragma once

#include <OpenHome/IWatchable.h>
#include <OpenHome/IWatcher.h>

namespace OpenHome
{
namespace Topology
{

/**
    \defgroup watcher Watcher
    @{
 */

template <class T>
class Watcher : public IWatcher<T>//, IDisposable
{
public:
    Watcher(IWatchable<T>& aWatchable, FunctorGeneric<void*> aAction);

    // IWatcher<T>
    virtual void ItemOpen(const Brx& aId, T aValue);
    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious);
    virtual void ItemClose(const Brx& aId, T aValue);

    // IDisposable
    virtual void Dispose();

private:
    void Watch(void*);
    void DisposeCallback(void*);

private:
    IWatchable<T>& iWatchable;
    FunctorGeneric<void*> iAction;
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class WatcherUnordered : public IWatcherUnordered<T>//, public IDisposable
{
public:
    WatcherUnordered(IWatchableUnordered<T>& aWatchable, FunctorGeneric<void*> aAction);

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
    void DisposeCallback(void*);

private:
    IWatchableUnordered<T>& iWatchable;
    FunctorGeneric<void*> iAction;
    TBool iInitialised;
};

/////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class WatcherOrdered : public IWatcherOrdered<T>//, public IDisposable
{
public:
    WatcherOrdered(IWatchableOrdered<T>& aWatchable, FunctorGeneric<void*> aAction);

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
    void DisposeCallback(void*);

private:
    IWatchableOrdered<T>& iWatchable;
    FunctorGeneric<void*> iAction;
    TBool iInitialised;
};

/////////////////////////////////////////////////////////////////////////////////////////

/**
    @}
 */

} // namespace Topology
} // namespace OpenHome
