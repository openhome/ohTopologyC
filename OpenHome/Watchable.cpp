#include <OpenHome/Watchable.h>


using namespace OpenHome;
using namespace OpenHome::Av;

///////////////////////////////////////////////////////

WatchableBase::WatchableBase(IWatchableThread& aWatchableThread)
    :iWatchableThread(aWatchableThread)
{
}


void WatchableBase::Assert()
{
    iWatchableThread.Assert();
}


void WatchableBase::Schedule(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Schedule(aCallback, aObj);
}


void WatchableBase::Execute(FunctorGeneric<void*> aCallback, void* aObj)
{
    iWatchableThread.Execute(aCallback, aObj);
}


TBool WatchableBase::IsWatchableThread()
{
    return(iWatchableThread.IsWatchableThread());
}


void WatchableBase::Lock()
{

}


void WatchableBase::Unlock()
{

}


//////////////////////////////////////////////////////////////////////

template <class T>
Watchable<T>::Watchable(IWatchableThread& aWatchableThread, const Brx& aId, T aValue)
    :WatchableBase(aWatchableThread)
{
    iId = aId;
    iValue = aValue;
    //iWatchers = new List<IWatcher<T>>();
    //iRecentlyRemoved = new List<IWatcher<T>>();
    ASSERT(aValue != NULL);
}


template <class T>
TBool Watchable<T>::Update(T aValue)
{
    ASSERT(aValue != NULL);

    Assert();

    if (iValue.Equals(aValue))
    {
        return (false);
    }

    T previous = iValue;

    iValue = aValue;

    std::vector<IWatcher<T>*> watchers(iWatchers);

    iRecentlyRemoved.clear();


    for(TUint i=0; i<watchers.size(); i++)
    {
        IWatcher<T>& watcher = watchers[i];

        if (!Contains(iRecentlyRemoved, watcher))
        {
            watcher.ItemUpdate(iId, iValue, previous);
        }
    }

    return (true);
}


template <class T>
T Watchable<T>::Value() const
{
    Assert();
    return (iValue);
}


template <class T>
const Brx& Watchable<T>::Id() const
{
    return (iId);
}


template <class T>
void Watchable<T>::AddWatcher(IWatcher<T>& aWatcher)
{
    Assert();
    ASSERT(!Contains(iWatchers, &aWatcher));

    Add(iWatchers, &aWatcher);

    aWatcher.ItemOpen(iId, iValue);
}


template <class T>
void Watchable<T>::RemoveWatcher(IWatcher<T>& aWatcher)
{
    Assert();
    ASSERT(!Contains(iWatchers, &aWatcher));

    Remove(iWatchers, &aWatcher);

    aWatcher.ItemClose(iId, iValue);

    Add(iRecentlyRemoved, &aWatcher);

    //iRecentlyRemoved.push_back(&aWatcher);
}


template <class T>
void Watchable<T>::Dispose()
{
    FunctorGeneric<void*> action = MakeFunctorGeneric(*this, &Watchable::DisposeCallback);
    TUint dummy;
    Execute(action, &dummy);
}

template <class T>
void Watchable<T>::DisposeCallback(void*)
{
    ASSERT(iWatchers.size() == 0);
}




///////////////////////////////////////////////////////////////////////////////
template <class T>
WatchableCollection<T>::WatchableCollection(IWatchableThread& aWatchableThread)
    :WatchableBase(aWatchableThread)
{
}



/*
template <class T>
IEnumerable<T> WatchableCollection<T>::Values() const
{
        return (iValues);
}
*/
///////////////////////////////////////////////////////////////////////////////


/*
template <class T>
void WatchableExtensions<T>::Execute(IWatchableThread& aWatchableThread)
{
    // can't make a functor in a static method (no THIS pointer)

    Action action = MakeFunctorGeneric(*this, &WatchableExtensions::ExecuteCallback);
    TUint dummy
    aWatchableThread.Execute(action, &dummy);
}

template <class T>
void WatchableExtensions<T>::ExecuteCallback(void*)
{

}


template <class T>
void WatchableExtensions<T>::Insert<T>(std::vector<T> aItems, TUint aIndex, T aItem)
{
    aItems.Insert((TInt)aIndex, aItem);
}

template <class T>
void WatchableExtensions<T>::RemoveAt<T>(std::vector<T> aItems, TUint aIndex)
{
    aItems.RemoveAt((TInt)aIndex);
}

template <class T>
T WatchableExtensions<T>::ElementAt<T>(std::vector<T> aItems, TUint aIndex)
{
    return (aItems.At((TInt)aIndex));
}

template <class T>
TUint WatchableExtensions<T>::ElementCount<T>(std::vector<T> aItems)
{
    return ((TUint)aItems.size());
}


template <class T>
TUint WatchableExtensions<T>::ElementCount<K, V>(IDictionary<K, V> aDictionary)
{
    return ((TUint)aDictionary.Count);
}
*/




