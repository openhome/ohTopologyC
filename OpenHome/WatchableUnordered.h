#ifndef HEADER_WATCHABLE_UNORDERED
#define HEADER_WATCHABLE_UNORDERED


#include <OpenHome/IWatchable.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/OhTopologyC.h>
#include <vector>
#include <algorithm>




namespace OpenHome
{

namespace Av
{


//class WatchableBase;

/**
    \ingroup watchable
    @{
 */
template <class T>
class WatchableUnordered : public WatchableBase, public IWatchableUnordered<T>, public IDisposable //, public WatchableCollection<T>
{
public:
    WatchableUnordered(IWatchableThread& aWatchableThread);
    virtual void Add(T aItem);
    virtual void Remove(T aItem);
    virtual void Clear();

    // IWatchableUnordered<T>
    virtual void AddWatcher(IWatcherUnordered<T>& aWatcher);
    virtual void RemoveWatcher(IWatcherUnordered<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

private:
    std::vector<T> iWatchables;
    std::vector<IWatcherUnordered<T>*> iWatchers;
    TUint iCount;
};




/**
    Constructor

    @param[in] aWatchableThread   Reference to a watchable thread object
 */
template <class T>
WatchableUnordered<T>::WatchableUnordered(IWatchableThread& aWatchableThread)
    :WatchableBase(aWatchableThread)
{
}

/**
    Add a watchable to the list of watchables

    @param[in] aWatchable   Reference to the watchable being added
 */
template <class T>
void WatchableUnordered<T>::Add(T aWatchable)
{
    //LOG(kTrace, "WatchableUnordered<T>::Add \n");
    Assert(); /// must be on watchable thread

    iWatchables.push_back(aWatchable); /// add aWatchable to iWatchables
    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); /// get snapshot of iWatchers (can't mutex when calling out to unknown code)

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->UnorderedAdd(aWatchable); /// add aWatchable to every watcher's list
    }
}


/**
    Remove an existing watchable from the list of watchables

    @param[in] aWatchable   Reference to the watchable being removed
 */
template <class T>
void WatchableUnordered<T>::Remove(T aWatchable)
{
    //Log::Print("\nWatchableUnordered<T>::Remove ");
    //Log::Print(typeid(this).name());

    //LOG(kTrace, "WatchableUnordered<T>::Remove \n");
    Assert(); /// must be on watchable thread

    typename std::vector<T>::iterator it = std::find(iWatchables.begin(), iWatchables.end(), aWatchable);
    ASSERT(it != iWatchables.end()); /// aWatchable must exist in iWatchables
    iWatchables.erase(it); /// remove aWatchable from iWatchables

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); /// get snapshot (can't mutex when calling out to unknown code)
    for(TUint i=0; i<watchers.size(); i++ )
    {
        watchers[i]->UnorderedRemove(aWatchable); /// remove aWatchable from every watcher's list
    }
}


/**
    Clear the list of watchables

 */
template <class T>
void WatchableUnordered<T>::Clear()
{
    Assert(); /// must be on watchable thread

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); /// get snapshot (can't mutex when calling out to unknown code)
    std::vector<T> watchables(iWatchables); /// get snapshot (can't mutex when calling out to unknown code)
    iWatchables.clear(); /// clear list of watchables, iWatchables

    for(TUint i=0; i<watchers.size(); i++ )
    {
        for(TUint x=0; x<watchables.size(); x++ )
        {
            watchers[i]->UnorderedRemove(watchables[x]); /// remove every watchable from every watcher's list
        }
    }
}


/**
    Add a watcher to the list of watchers

    @param[in] aWatcher   Reference to the watcher being added
 */
template <class T>
void WatchableUnordered<T>::AddWatcher(IWatcherUnordered<T>& aWatcher)
{
    //LOG(kTrace, "WatchableUnordered<T>::AddWatcher  iWatchables.size()=%d\n", iWatchables.size());
    Assert(); /// must be on watchable thread
    iWatchers.push_back(&aWatcher); /// add aWatcher to iWatchers
    aWatcher.UnorderedOpen(); /// set aWatcher status to Open

    for (TUint i=0; i<iWatchables.size(); i++)
    {
        //LOG(kTrace, "WatchableUnordered<T>::AddWatcher  adding watchables...\n");
        aWatcher.UnorderedAdd(iWatchables[i]); /// add all watchables to aWatcher
    }

    aWatcher.UnorderedInitialised(); /// set aWatcher status to Initialised
}


/**
    Remove a Watcher from the list of watchers

    @param[in] aWatcher   Reference to the watcher being removed
 */
template <class T>
void WatchableUnordered<T>::RemoveWatcher(IWatcherUnordered<T>& aWatcher)
{
    Assert(); /// must be on watchable thread
    typename std::vector<IWatcherUnordered<T>*>::iterator it = std::find(iWatchers.begin(), iWatchers.end(), &aWatcher);

    if (it != iWatchers.end())  /// check aWatcher exists
    {
        iWatchers.erase(it); /// remove aWatcher from iWatchers
    }

    aWatcher.UnorderedClose(); /// set aWatcher status to Closed
}


template <class T>
void WatchableUnordered<T>::Dispose()
{
    iCount = 0;
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &WatchableUnordered::DisposeCallback);
    Execute(f, 0);
    ASSERT(iCount==0);
}


template <class T>
void WatchableUnordered<T>::DisposeCallback(void*)
{
    iCount = iWatchers.size();
}




/**
    @}
 */



} // namespace Av
} // namespace OpenHome





#endif // HEADER_WATCHABLE_UNORDERED
