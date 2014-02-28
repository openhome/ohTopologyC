#ifndef WATCHER_CONTROL
#define WATCHER_CONTROL


#include<OpenHome/IWatchable.h>


namespace OpenHome
{

namespace Av
{


class IControl //: IDisposable
{
public:
    virtual const Brx& Id() = 0;
};

/**
    \addtogroup watcher
    @{
 */

template<class T>
class WatcherControl : public IWatcher<T>, public IControl
{

public:
    // IWatcher<T>
    virtual void ItemOpen(const Brx& aId, T aValue);
    virtual void ItemUpdate(const Brx& aId, T aValue, T aPrevious);
    virtual void ItemClose(const Brx& aId, T aValue);

protected:
    WatcherControl(IWatchable<T>& aWatchable);
    void Open();
    virtual IControl CreateControl(T aValue) = 0;
    virtual void UpdateControl(T aValue, T aPrevious) = 0;

    // IControl
    virtual const Brx& Id();
    virtual void Dispose();

protected:
    IWatchable<T>& iWatchable;

private:
    IControl iControl;
};


/**
    @}
 */

} // Av

} // OpenHome


#endif // WATCHER_CONTROL
