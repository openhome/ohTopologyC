#ifndef HEADER_IWATCHABLE
#define HEADER_IWATCHABLE

#include <OpenHome/WatchableThread.h>
#include <vector>

namespace OpenHome
{

namespace Av
{


template <class T>
class IWatcher
{
public:
    virtual void ItemOpen(const Brx& aId, T aValue) = 0;
    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious) = 0;
    virtual void ItemClose(const Brx& aId, T aValue) = 0;
    virtual ~IWatcher() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchable : public IWatchableThread
{
public:
    virtual T Value() const = 0;
    virtual const Brx& Id() const = 0;
    virtual void AddWatcher(IWatcher<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcher<T>& aWatcher) = 0;
    virtual ~IWatchable() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchableCollection : public IWatchableThread
{
public:
    //virtual IEnumerable<T> Values() = 0;
    virtual ~IWatchableCollection() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatcherOrdered
{
public:
    virtual void OrderedOpen() = 0;
    virtual void OrderedInitialised() = 0;
    virtual void OrderedAdd(T aItem, TUint aIndex) = 0;
    virtual void OrderedMove(T aItem, TUint aFrom, TUint aTo) = 0;
    virtual void OrderedRemove(T aItem, TUint aIndex) = 0;
    virtual void OrderedClose() = 0;
    virtual ~IWatcherOrdered() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatcherUnordered
{
public:
    virtual void UnorderedOpen() = 0;
    virtual void UnorderedInitialised() = 0;
    virtual void UnorderedAdd(T aItem) = 0;
    virtual void UnorderedRemove(T aItem) = 0;
    virtual void UnorderedClose() = 0;
    virtual ~IWatcherUnordered() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchableUnordered : public IWatchableCollection<T>
{
public:
    virtual void AddWatcher(IWatcherUnordered<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcherUnordered<T>& aWatcher) = 0;
    virtual ~IWatchableUnordered() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchableOrdered : public IWatchableCollection<T>
{
public:
    virtual void AddWatcher(IWatcherOrdered<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcherOrdered<T>& aWatcher) = 0;
    virtual ~IWatchableOrdered() {}
};

//////////////////////////////////////////////////////////////////////

} // Av

} // OpenHome


#endif // HEADER_WATCHABLE


