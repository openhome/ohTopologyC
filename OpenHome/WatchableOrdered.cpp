#include <OpenHome/WatchableOrdered.h>
#include<vector>

using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



template <class T>
WatchableOrdered<T>::WatchableOrdered(IWatchableThread& aWatchableThread)
    :WatchableCollection<T>(aWatchableThread)
{
}


/**
    Add an item to the collection at a specified index position

    @param[in] aItem   Reference to the item o be added
    @param[in] aIndex  An integer specifying the index position

 */
template <class T>
void WatchableOrdered<T>::Add(T& aItem, TUint aIndex)
{
    WatchableBase::Assert();

    WatchableCollection<T>::iItems.insert(WatchableCollection<T>::iItems.begin()+aIndex, aItem);

    std::vector<IWatcherOrdered<T>*> watchers(iWatchers);

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->OrderedAdd(aItem, aIndex);
    }
}

/**
    Move an item to a new index in the list

 */
template <class T>
void WatchableOrdered<T>::Move(T& aItem, TUint aNewIndex)
{
    WatchableBase::Assert();

    typename std::vector<IWatcherOrdered<T>*>::iterator item = find(WatchableCollection<T>::iItems.begin(), WatchableCollection<T>::iItems.end(), &aItem);
    ASSERT(item != WatchableCollection<T>::iItems.end())

    TUint oldIndex = item-WatchableCollection<T>::iItems.begin();
    WatchableCollection<T>::iItems.erase(item);
    WatchableCollection<T>::iItems.insert(WatchableCollection<T>::iItems.begin()+aNewIndex, aItem);

    std::vector<IWatcherOrdered<T>*> watchers(iWatchers);

    for(TUint i=0; i<watchers.size(); i++ )
    {
         watchers[i]->OrderedMove(aItem, oldIndex, aNewIndex);

    }
}


/**
    Remove an item from the list

 */
template <class T>
void WatchableOrdered<T>::Remove(T& aItem)
{
    WatchableBase::Assert();

    typename vector<IWatcherOrdered<T>*>::iterator item = find(WatchableCollection<T>::iItems.begin(), WatchableCollection<T>::iItems.end(), &aItem);
    ASSERT(item != WatchableCollection<T>::iItems.end())

    TUint index = item-WatchableCollection<T>::iItems.begin();
    WatchableCollection<T>::iItems.erase(item);

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); // get a fixed copy (because we're calling out to unknown code)
    for(TUint i=0; i<watchers.size(); i++ )
    {
        watchers[i]->OrderedRemove(aItem, index);
    }
}


/**
    Clear the list

 */
template <class T>
void WatchableOrdered<T>::Clear()
{
    WatchableBase::Assert();

    std::vector<IWatcherUnordered<T>*> watchers(iWatchers); // get a fixed copy (because we're calling out to unknown code)
    std::vector<T*> items(WatchableCollection<T>::iItems); // get a fixed copy (because we're calling out to unknown code)
    WatchableCollection<T>::iItems.clear();

    for(TUint i=0; i<watchers.size(); i++ )
    {
        for(TUint x=0; x<items.size(); x++ )
        {
            watchers[i]->OrderedRemove(*(items[x]), x);
        }
    }
}


/**
    Add a Watcher to the list

 */
template <class T>
void WatchableOrdered<T>::AddWatcher(IWatcherOrdered<T>& aWatcher)
{
    WatchableBase::Assert();

    iWatchers.push_back(&aWatcher);

    aWatcher.OrderedOpen();

    for(TUint i=0; i<WatchableCollection<T>::iItems.size(); i++)
    {
        aWatcher.OrderedAdd(*(WatchableCollection<T>::iItems[i]), i);
    }

    aWatcher.OrderedInitialised();
}


/**
    Remove a Watcher from the list

 */
template <class T>
void WatchableOrdered<T>::RemoveWatcher(IWatcherOrdered<T>& aWatcher)
{
    WatchableBase::Assert();

    typename std::vector<T>::iterator watcher = find(iWatchers.begin(), iWatchers.end(), &aWatcher);

    ASSERT(watcher != iWatchers.end())

    if (watcher != iWatchers.end())
    {
        iWatchers.erase(watcher);
    }

    aWatcher.OrderedClose();
}


template <class T>
void WatchableOrdered<T>::Dispose()
{
    FunctorGeneric<void*> action = MakeFunctorGeneric(*this, &WatchableOrdered::DisposeCallback);
    WatchableBase::Execute(action);
}


template <class T>
void WatchableOrdered<T>::DisposeCallback(void*)
{
    ASSERT(iWatchers.size() == 0);
}


