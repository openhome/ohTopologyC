#ifndef HEADER_IWATCHER
#define HEADER_IWATCHER

#include <OpenHome/WatchableThread.h>
#include <vector>

namespace OpenHome
{

namespace Topology
{

/**
    \addtogroup watcher
    @{
 */


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
class IWatcher
{
public:
    virtual void ItemOpen(const Brx& aId, T aValue) = 0;
    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious) = 0;
    virtual void ItemClose(const Brx& aId, T aValue) = 0;
    virtual ~IWatcher() {}
};

//////////////////////////////////////////////////////////////////////

/**
    @}
 */



} // Av

} // OpenHome


#endif // HEADER_IWATCHER


