#ifndef HEADER_WATCHABLE_ORDERED
#define HEADER_WATCHABLE_ORDERED

#include <OpenHome/Watchable.h>
#include<vector>




namespace OpenHome
{
namespace Av
{



/**
    \ingroup watchable
    @{
 */


template <class T>
class WatchableOrdered : public WatchableCollection<T>, public IWatchableOrdered<T>//, public IDisposable,
{
public:
    WatchableOrdered(IWatchableThread& aWatchableThread);

    virtual void Add(T& aValue, TUint aIndex);
    virtual void Move(T& aValue, TUint aIndex);
    virtual void Remove(T& aValue);
    virtual void Clear();

    // IWatchableOrdered<T>
    virtual void AddWatcher(IWatcherOrdered<T>& aWatcher);
    virtual void RemoveWatcher(IWatcherOrdered<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

private:
    std::vector<IWatcherOrdered<T>*> iWatchers;
};


/**
    @}
 */


}  // Av
}  // OpenHome




#endif // HEADER_WATCHABLE_ORDERED
