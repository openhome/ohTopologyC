#ifndef HEADER_WATCHABLE_UNORDERED
#define HEADER_WATCHABLE_UNORDERED


#include <OpenHome/Watchable.h>
#include <vector>




namespace OpenHome
{

namespace Av
{


/**
    \ingroup watchable
    @{
 */

template <class T>
class WatchableUnordered : public IWatchableUnordered<T>, public WatchableCollection<T> //, public IDisposable
{
public:
    WatchableUnordered(IWatchableThread& aWatchableThread);
    void Add(T& aItem);
    void Remove(T& aItem);
    void Clear();

    // IWatchableUnordered<T>
    void AddWatcher(IWatcherUnordered<T>& aWatcher);
    void RemoveWatcher(IWatcherUnordered<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

private:
    std::vector<IWatcherUnordered<T>*> iWatchers;
};


/**
    @}
 */



} // namespace Av
} // namespace OpenHome





#endif // HEADER_WATCHABLE_UNORDERED
