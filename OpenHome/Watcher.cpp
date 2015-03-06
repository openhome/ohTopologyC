#include <OpenHome/Watcher.h>


using namespace OpenHome;
using namespace OpenHome::Topology;


template<class T>
Watcher<T>::Watcher(IWatchable<T>& aWatchable, FunctorGeneric<void*> aAction)
    :iWatchable(aWatchable)
    ,iAction(aAction)
{
    iWatchable.Schedule(MakeFunctorGeneric(*this, &Watcher<T>::Watch), this);
}


template<class T>
void Watcher<T>::Watch(void* aObj)
{
    iWatchable.AddWatcher(*((Watcher<T>)aObj));
}


template<class T>
void Watcher<T>::ItemOpen(const Brx& aId, T aValue)
{
    iAction(aValue, true);
}


template<class T>
void Watcher<T>::ItemUpdate(const Brx& aId, T aValue, T aPrevious)
{
    iAction(aValue, false);
}


template<class T>
void Watcher<T>::ItemClose(const Brx& aId, T aValue)
{
}


template<class T>
void Watcher<T>::Dispose()
{
    iWatchable.Execute(MakeFunctorGeneric(*this, &Watcher<T>::DisposeCallback), this);
}


template<class T>
void Watcher<T>::DisposeCallback(void* aObj)
{
    iWatchable.RemoveWatcher(*((Watcher<T>*)aObj));
    delete aObj;
}


////////////////////////////////////////////////////////////////////////////////////

template<class T>
WatcherUnordered<T>::WatcherUnordered(IWatchableUnordered<T>& aWatchable, FunctorGeneric<void*> aAction)
    :iWatchable(aWatchable)
    ,iAction(aAction)
    ,iInitialised(false)
{
    iWatchable.Schedule(MakeFunctorGeneric(*this, &WatcherUnordered<T>::Watch), this);
}


template<class T>
void WatcherUnordered<T>::Watch(void* aObj)
{
    iWatchable.AddWatcher(*((WatcherUnordered<T>)aObj));
}


template<class T>
void WatcherUnordered<T>::UnorderedOpen()
{
}


template<class T>
void WatcherUnordered<T>::UnorderedInitialised()
{
    iInitialised = true;
    iAction(iWatchable.Values);
}


template<class T>
void WatcherUnordered<T>::UnorderedAdd(T aItem)
{
    if (iInitialised)
    {
        iAction(iWatchable.Values);
    }
}


template<class T>
void WatcherUnordered<T>::UnorderedRemove(T aItem)
{
    iAction(iWatchable.Values);
}


template<class T>
void WatcherUnordered<T>::UnorderedClose()
{
}

/*
template<class T>
void WatcherUnordered<T>::Dispose()
{
    iWatchable.Execute(() =>
    {
        iWatchable.RemoveWatcher(this);
    });
}
*/

template<class T>
void WatcherUnordered<T>::Dispose()
{
    iWatchable.Execute(MakeFunctorGeneric(*this, &WatcherUnordered<T>::DisposeCallback), this);
}


template<class T>
void WatcherUnordered<T>::DisposeCallback(void* aObj)
{
    iWatchable.RemoveWatcher(*((WatcherUnordered<T>*)aObj));
}



////////////////////////////////////////////////////////////////////////////////////


template<class T>
WatcherOrdered<T>::WatcherOrdered(IWatchableOrdered<T>& aWatchable, FunctorGeneric<void*> aAction)
    :iWatchable(aWatchable)
    ,iAction(aAction)
    ,iInitialised(false)
{
    iWatchable.Schedule(MakeFunctorGeneric(*this, &WatcherOrdered<T>::Watch), this);
}


template<class T>
void WatcherOrdered<T>::Watch(void* aObj)
{
    iWatchable.AddWatcher(*((WatcherOrdered<T>)aObj));
}

template<class T>
void WatcherOrdered<T>::OrderedOpen()
{
}


template<class T>
void WatcherOrdered<T>::OrderedInitialised()
{
    iInitialised = true;

    iAction(iWatchable.Values);
}


template<class T>
void WatcherOrdered<T>::OrderedAdd(T aItem, TUint aIndex)
{
    if (iInitialised)
    {
        iAction(iWatchable.Values);
    }
}


template<class T>
void WatcherOrdered<T>::OrderedMove(T aItem, TUint aFrom, TUint aTo)
{
    iAction(iWatchable.Values);
}


template<class T>
void WatcherOrdered<T>::OrderedRemove(T aItem, TUint aIndex)
{
    iAction(iWatchable.Values);
}


template<class T>
void OrderedClose()
{
}


template<class T>
void WatcherOrdered<T>::Dispose()
{
    FunctorGeneric<void*> f = MakeFunctorGeneric(*this, &WatcherOrdered<T>::DisposeCallback);
    iWatchable.Execute(f, this);
}


template<class T>
void WatcherOrdered<T>::DisposeCallback(void* aObj)
{
    iWatchable.RemoveWatcher(*((WatcherOrdered<T>)aObj));
}

////////////////////////////////////////////////////////////////////////////////////

/*
template<class T>
static IDisposable WatcherExtensions<T>::CreateWatcher<T>(this IWatchable<T> aWatchable, FunctorGeneric<void*><T> aAction)
{
    return (new Watcher<T>(aWatchable, aAction));
}


template<class T>
static IDisposable WatcherExtensions<T>::CreateWatcher<T>(this IWatchable<T> aWatchable, Action<T, bool> aAction)
{
    return (new Watcher<T>(aWatchable, aAction));
}


template<class T>
static IDisposable WatcherExtensions<T>::CreateWatcher<T>(this IWatchableUnordered<T> aWatchable, Action<IEnumerable<T>> aAction)
{
    return (new WatcherUnordered<T>(aWatchable, aAction));
}


template<class T>
static IDisposable WatcherExtensions<T>::CreateWatcher<T>(this IWatchableOrdered<T> aWatchable, Action<IEnumerable<T>> aAction)
{
    return (new WatcherOrdered<T>(aWatchable, aAction));
}
*/



