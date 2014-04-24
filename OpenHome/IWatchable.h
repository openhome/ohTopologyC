#ifndef HEADER_IWATCHABLE
#define HEADER_IWATCHABLE

#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatcher.h>
#include <vector>

namespace OpenHome
{

namespace Av
{


/**
    \defgroup watchable Watchable
    @{
 */

template <class T>
class IWatchable
{
public:
    virtual T Value() = 0;
    //virtual const Brx& Id() = 0;
    virtual void AddWatcher(IWatcher<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcher<T>& aWatcher) = 0;
    virtual ~IWatchable() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchableUnordered
{
public:
    virtual void AddWatcher(IWatcherUnordered<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcherUnordered<T>& aWatcher) = 0;
    virtual ~IWatchableUnordered() {}
};

//////////////////////////////////////////////////////////////////////

template <class T>
class IWatchableOrdered
{
public:
    virtual void AddWatcher(IWatcherOrdered<T>& aWatcher) = 0;
    virtual void RemoveWatcher(IWatcherOrdered<T>& aWatcher) = 0;
    virtual ~IWatchableOrdered() {}
};

//////////////////////////////////////////////////////////////////////

/**
    @}
 */



} // Av

} // OpenHome


#endif // HEADER_IWATCHABLE


