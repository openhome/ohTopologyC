#include<OpenHome/WatcherControl.h>


using namespace OpenHome;


template<class T>
WatcherControl<T>::WatcherControl(IWatchable<T>& aWatchable)
    :iWatchable(aWatchable)
{
}


template<class T>
void WatcherControl<T>::Open()
{
    iWatchable.AddWatcher(this);
}


template<class T>
void WatcherControl<T>::ItemOpen(const Brx& aId, T aItem)
{
    iControl = CreateControl(aItem);
}


template<class T>
void WatcherControl<T>::ItemUpdate(const Brx& aId, T aItem, T aPrevious)
{
    UpdateControl(aItem, aPrevious);
}


template<class T>
void WatcherControl<T>::ItemClose(const Brx& aId, T aItem)
{
    //iControl.Dispose();
}


template<class T>
const Brx& WatcherControl<T>::Id()
{
    return (iControl.Id());
}


template<class T>
void WatcherControl<T>::Dispose()
{
    iWatchable.RemoveWatcher(*this);
}



