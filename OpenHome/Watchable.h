#ifndef HEADER_WATCHABLE
#define HEADER_WATCHABLE

#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatchable.h>
#include <vector>
#include <algorithm>


/*
    Windows compiler warns (4505) that it has removed certain unreferenced local functions.
    Debugger has been used to proved that these functions _are_ referenced.
    The warning is wrong - so I'm disabling it.
*/
#ifdef _MSC_VER
#pragma warning (disable : 4505)
#endif


namespace OpenHome
{

namespace Topology
{



/**
    \addtogroup  watchable
    @{
 */

class WatchableBase : public IWatchableThread, public INonCopyable
{
protected:
    WatchableBase(IWatchableThread& aWatchableThread);

public:
    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);

private:
    IWatchableThread& iWatchableThread;
};

//////////////////////////////////////////////////////////////////////////////////

template <class T>
class Watchable : public IWatchable<T>, public WatchableBase, public IDisposable
{
public:
    Watchable(IWatchableThread& aWatchableThread, const Brx& aId, T aValue);
    virtual TBool Update(T aValue);

    // IWatchable<T>
    virtual T Value();
    //virtual const Brx& Id();
    virtual void AddWatcher(IWatcher<T>& aWatcher);
    virtual void RemoveWatcher(IWatcher<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

protected:
    Brn iId;
    T iValue;
    TBool iUpdating;

private :
    std::vector<IWatcher<T>*> iWatchers;
    std::vector<IWatcher<T>*> iRecentlyRemoved;
    TUint iCount;
};


//////////////////////////////////////////////////////////////////////

/**

 */
template <class T>
Watchable<T>::Watchable(IWatchableThread& aWatchableThread, const Brx& aId, T aValue)
    :WatchableBase(aWatchableThread)
    ,iId(aId)
    ,iValue(aValue)
    ,iUpdating(false)
    ,iCount(0)
{
    //ASSERT(aValue != NULL);
}


/**

 */
template <class T>
TBool Watchable<T>::Update(T aValue)
{
    // ASSERT(aValue != NULL);

    Assert();

    //if (iValue.Equals(aValue))
    if (iValue == aValue)
    {
        return (false);
    }

    iUpdating = true;
    T previous = iValue;
    iValue = aValue;

    std::vector<IWatcher<T>*> watchers(iWatchers);

    for(TUint i=0; i<watchers.size(); i++)
    {
        IWatcher<T>* watcher = watchers[i];

        auto it = std::find(iRecentlyRemoved.begin(), iRecentlyRemoved.end(), watcher);

        if (it==iRecentlyRemoved.end()) // not found
        {
            watcher->ItemUpdate(iId, iValue, previous);
        }
    }

    iRecentlyRemoved.clear();
    iUpdating = false;

    return (true);
}


/**

 */
template <class T>
T Watchable<T>::Value()
{
    Assert();
    return (iValue);
}


/**

 */
/*
template <class T>
const Brx& Watchable<T>::Id()
{
    return (iId);
}
*/

/**

 */
template <class T>
void Watchable<T>::AddWatcher(IWatcher<T>& aWatcher)
{
    Assert();
    auto it = std::find(iWatchers.begin(), iWatchers.end(), &aWatcher);
    ASSERT(it==iWatchers.end());
    iWatchers.push_back(&aWatcher);
    aWatcher.ItemOpen(iId, iValue);
}


/**

 */
template <class T>
void Watchable<T>::RemoveWatcher(IWatcher<T>& aWatcher)
{
    Assert();

    auto it = std::find(iWatchers.begin(), iWatchers.end(), &aWatcher);
    ASSERT(it!=iWatchers.end());
    iWatchers.erase(it);

    // Assert that it no longer exists
    it = std::find(iWatchers.begin(), iWatchers.end(), &aWatcher);
    ASSERT(it==iWatchers.end());

    aWatcher.ItemClose(iId, iValue);

    if (iUpdating)
    {
        iRecentlyRemoved.push_back(&aWatcher);
    }
}


/**

 */
template <class T>
void Watchable<T>::Dispose()
{
    iCount = 0;
    FunctorGeneric<void*> action = MakeFunctorGeneric(*this, &Watchable::DisposeCallback);
    Execute(action, NULL);
    if (iCount>0)
    {
        ASSERTS();
    }
}


/**

 */
template <class T>
void Watchable<T>::DisposeCallback(void*)
{
    iCount = iWatchers.size();
}



//////////////////////////////////////////////////////////////////////////////////


/**
    @}
 */




} // namespace Topology
} // namespace OpenHome


#endif // HEADER_WATCHABLE


