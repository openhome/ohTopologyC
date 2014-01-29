#include <OpenHome/WatchableUnordered.h>

#include<vector>

using namespace OpenHome;
using namespace OpenHome::Av;



template <class T>
WatchableUnordered<T>::WatchableUnordered(IWatchableThread& aWatchableThread)
    :WatchableCollection<T>(aWatchableThread)
{
}


template <class T>
void WatchableUnordered<T>::Add(T& aItem)
{
    WatchableBase::Assert();

    WatchableCollection<T>::iItems.push_back(&aItem);
    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); // get a fixed copy (because we're calling out to unknown code)

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->UnorderedAdd(aItem);

    }
}


template <class T>
void WatchableUnordered<T>::Remove(T& aItem)
{
    WatchableBase::Assert();

    typename std::vector<T>::iterator item = find(WatchableCollection<T>::iItems.begin(), WatchableCollection<T>::iItems.end(), &aItem);
    ASSERT(item != WatchableCollection<T>::iItems.end());
    WatchableCollection<T>::iItems.erase(item);

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); // get a fixed copy (because we're calling out to unknown code)
    for(TUint i=0; i<watchers.size(); i++ )
    {
        watchers[i]->UnorderedRemove(aItem);
    }
}


template <class T>
void WatchableUnordered<T>::Clear()
{
    WatchableBase::Assert();

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); // get a fixed copy (because we're calling out to unknown code)
    std::vector<T*> items(WatchableCollection<T>::iItems); // get a fixed copy (because we're calling out to unknown code)
    WatchableCollection<T>::iItems.clear();

    for(TUint i=0; i<watchers.size(); i++ )
    {
        for(TUint x=0; x<items.size(); x++ )
        {
            watchers[i]->UnorderedRemove(items[x]);
        }
    }
}


template <class T>
void WatchableUnordered<T>::AddWatcher(IWatcherUnordered<T>& aWatcher)
{
    WatchableBase::Assert();
    iWatchers.push_back(&aWatcher);

    aWatcher.UnorderedOpen();

    for (TUint i=0; i<WatchableCollection<T>::iItems.size(); i++)
    {
        aWatcher.UnorderedAdd(WatchableCollection<T>::iItems[i]);
    }

    aWatcher.UnorderedInitialised();
}


template <class T>
void WatchableUnordered<T>::RemoveWatcher(IWatcherUnordered<T>& aWatcher)
{
    WatchableBase::Assert();
    typename std::vector<T>::iterator watcher = find(iWatchers.begin(), iWatchers.end(), &aWatcher);
    ASSERT(watcher != iWatchers.end())

    iWatchers.erase(watcher);

    aWatcher.UnorderedClose();
}


template <class T>
void WatchableUnordered<T>::Dispose()
{
    Action action = MakeFunctorGeneric(*this, &WatchableUnordered::DisposeCB);
    WatchableBase::Execute(action);
}


template <class T>
void WatchableUnordered<T>::DisposeCB(void*)
{
    ASSERT(iWatchers.size() == 0);
}




