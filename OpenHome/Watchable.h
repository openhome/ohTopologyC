#ifndef HEADER_WATCHABLE
#define HEADER_WATCHABLE

#include <OpenHome/WatchableThread.h>
#include <OpenHome/IWatchable.h>
#include <vector>



namespace OpenHome
{

namespace Av
{



/**
    \addtogroup  watchable
    @{
 */

class WatchableBase : public IWatchableThread//, public INonCopyable
{
protected:
    WatchableBase(IWatchableThread& aWatchableThread);

public:
    // IWatchableThread
    virtual void Assert();
    virtual void Schedule(FunctorGeneric<void*> aCallback, void* aObj);
    virtual void Execute(FunctorGeneric<void*> aCallback, void* aObj);
    virtual TBool IsWatchableThread();

    virtual void Lock();
    virtual void Unlock();

private:
    IWatchableThread& iWatchableThread;
};

//////////////////////////////////////////////////////////////////////////////////

template <class T>
class Watchable : public IWatchable<T>, public WatchableBase //, public IDisposable
{
public:
    Watchable(IWatchableThread& aWatchableThread, const Brx& aId, T aValue);
    TBool Update(T aValue);
    T Value() const;

    // IWatchable<T>
    virtual const Brx& Id() const;
    virtual void AddWatcher(IWatcher<T>& aWatcher);
    virtual void RemoveWatcher(IWatcher<T>& aWatcher);

    // IDisposable
    virtual void Dispose();

private:
    void DisposeCallback(void*);

protected:
    Brn iId;
    T iValue;

private :
    std::vector<IWatcher<T>*> iWatchers;
    std::vector<IWatcher<T>*> iRecentlyRemoved;
};

//////////////////////////////////////////////////////////////////////////////////

template <class T>
class WatchableCollection : public WatchableBase
{
public:
    //IEnumerable<T> Values() const;

protected:
    WatchableCollection(IWatchableThread& aWatchableThread);

protected:
    std::vector<T> iItems;
};

//////////////////////////////////////////////////////////////////////////////////

/*
template <class T>
class WatchableExtensions
{
public:
    static void Execute(IWatchableThread& aWatchableThread);
    static void Insert(std::vector<T> aItems, TUint aIndex, T aItem);
    static void RemoveAt(std::vector<T> aItems, TUint aIndex);
    static T ElementAt(std::vector<T> aItems, TUint aIndex);
    static TUint ElementCount(std::vector<T> aItems);
    //static TUint ElementCount<K, V>(this IDictionary<K, V> aDictionary);

private:
    static void ExecuteCallback(void*);

};

*/


/**
    @}
 */




} // namespace Av

} // namespace OpenHome


#endif // HEADER_WATCHABLE


