#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Watchable.h>
#include <OpenHome/Service.h>
#include <OpenHome/Net/Core/CpDevice.h>
#include <OpenHome/Net/Core/FunctorAsync.h>
#include <OpenHome/MetaData.h>
#include <OpenHome/Private/Printer.h>

#include <vector>
#include <memory>

namespace OpenHome
{
namespace Topology
{

class IMediaPreset : public IDisposable
{
public:
    virtual TUint Index() = 0;
    virtual IMediaMetadata& Metadata() = 0;
    virtual IWatchable<TBool>& Buffering() = 0;
    virtual IWatchable<TBool>& Playing() = 0;
    virtual IWatchable<TBool>& Selected() = 0;
    virtual void Play() = 0;
    virtual ~IMediaPreset() {}
};

/////////////////////////////////////////////////////////
template <class T>
class IWatchableFragment
{
public:
    virtual TUint Index() = 0;
    virtual std::vector<T>& Data() = 0;
    virtual ~IWatchableFragment() {}

};

/////////////////////////////////////////////////////////

template <class T>
class IWatchableSnapshot
{
public:
    virtual TUint Total() = 0;
    virtual std::vector<TUint>* Alpha() = 0; // null if no alpha map
    //virtual void Read(TUint aIndex, TUint aCount, CancellationToken aCancellationToken, Action<IWatchableFragment<T>> aCallback) = 0;
    virtual void Read(TUint aIndex, TUint aCount, /*CancellationToken aCancellationToken,*/ FunctorGeneric<IWatchableFragment<T>*> aCallback) = 0;
    virtual ~IWatchableSnapshot() {}
};

/////////////////////////////////////////////////////////

template <class T>
class IWatchableContainer
{
public:
    virtual IWatchable<IWatchableSnapshot<T>*>& Snapshot() = 0;
    virtual ~IWatchableContainer() {}

};

/////////////////////////////////////////////////////////

template <class T>
class IMediaClientSnapshot
{
public:
    virtual TUint Total() = 0;
    virtual std::vector<TUint>* Alpha() = 0; // null if no alpha map
    //virtual void Read(CancellationToken& aToken, TUint aIndex, TUint aCount, Action<IEnumerable<T>> aCallback) = 0;
    virtual void Read(/*CancellationToken& aToken,*/ TUint aIndex, TUint aCount, FunctorGeneric<std::vector<T>*> aCallback) = 0;
    virtual ~IMediaClientSnapshot() {}

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////

    // all calls into an instance of this class should be called on the watchable thread
template <class T>
class MediaSupervisor : public IDisposable
{
public:
    MediaSupervisor(IWatchableThread& aThread, IMediaClientSnapshot<T>* aClientSnapshot);
    ~MediaSupervisor();
    void Dispose();
    Watchable<IWatchableSnapshot<T>*>& Snapshot();
    void Update(IMediaClientSnapshot<T>* aClientSnapshot);

private:
    //CancellationTokenSource* iCancellationTokenSource;
    IWatchableSnapshot<T>* iSnapshotVal;
    std::unique_ptr<Watchable<IWatchableSnapshot<T>*>> iSnapshot;
};

//////////////////////////////////////////////////////////////////////////////////////////////////


template <class T>
class MediaSnapshot : public IWatchableSnapshot<T>, public IDisposable
{
public:
    MediaSnapshot(/*CancellationToken& aCancellationToken,*/ IMediaClientSnapshot<T>* aSnapshot);
    ~MediaSnapshot();
    // IWatchableSnapshot<IMediaPreset>
    TUint Total();
    std::vector<TUint>* Alpha();
    //void Read(TUint aIndex, TUint aCount, CancellationToken aCancellationToken, Action<IWatchableFragment<T>> aCallback);
    void Read(TUint aIndex, TUint aCount, /*CancellationToken aCancellationToken,*/ FunctorGeneric<IWatchableFragment<T>*> aCallback);

    // IDisposable
    void Dispose();

private:
    void ReadCallback(std::vector<T>*);

private:
    //CancellationToken& iCancellationToken;
    IMediaClientSnapshot<T>* iSnapshot;
    TBool iDisposed;
};

////////////////////////////////////////////////////////////////

template <class T>
class WatchableFragment : public IWatchableFragment<T>
{
public:
    WatchableFragment(TUint aIndex, std::vector<T>& aData);
    ~WatchableFragment();
    // IWatchableFragment<T>
    TUint Index();
    std::vector<T>& Data();

private:
    TUint iIndex;
    std::vector<T>& iData;
};

/////////////////////////////////////////////////////////////////

template <class T>
MediaSupervisor<T>::MediaSupervisor(IWatchableThread& aThread, IMediaClientSnapshot<T>* aClientSnapshot)
    //:iCancellationTokenSource(new CancellationTokenSource())
    : iSnapshotVal(new MediaSnapshot<T>(/*iCancellationTokenSource.Token,*/ aClientSnapshot))
    , iSnapshot(new Watchable<IWatchableSnapshot<T>*>(aThread, Brn("Snapshot"), iSnapshotVal))
{
}

template <class T>
MediaSupervisor<T>::~MediaSupervisor()
{
    delete iSnapshotVal;
}

template <class T>
void MediaSupervisor<T>::Dispose()
{
    //iCancellationTokenSource.Cancel();
    MediaSnapshot<T>* snapshot = (MediaSnapshot<T>*)iSnapshotVal;
    snapshot->Dispose();
    iSnapshot->Dispose();
    //iCancellationTokenSource.Dispose();
}


template <class T>
Watchable<IWatchableSnapshot<T>*>& MediaSupervisor<T>::Snapshot()
{
    return (*iSnapshot);
}

template <class T>
void MediaSupervisor<T>::Update(IMediaClientSnapshot<T>* aClientSnapshot)
{
//    MediaSnapshot<T> snapshot = iSnapshot.Value as MediaSnapshot<T>;
    MediaSnapshot<T>* snapshot = (MediaSnapshot<T>*)iSnapshot->Value();
    //iCancellationTokenSource.Cancel();
    //iCancellationTokenSource.Dispose();
    //iCancellationTokenSource = new CancellationTokenSource();
    iSnapshot->Update(new MediaSnapshot<T>(/*iCancellationTokenSource.Token, */aClientSnapshot));
    snapshot->Dispose();
}

//////////////////////////////////////////////////////////////////////

template <class T>
MediaSnapshot<T>::MediaSnapshot(/*CancellationToken& aCancellationToken,*/ IMediaClientSnapshot<T>* aSnapshot)
    //:iCancellationToken(aCancellationToken)
    :iSnapshot(aSnapshot)
    ,iDisposed(false)
{
}

// IWatchableSnapshot<IMediaPreset>
template <class T>
MediaSnapshot<T>::~MediaSnapshot()
{
    delete iSnapshot;
}

template <class T>
TUint MediaSnapshot<T>::Total()
{
    return (iSnapshot->Total());
}


template <class T>
std::vector<TUint>* MediaSnapshot<T>::Alpha()
{
    return (iSnapshot->Alpha());
}


template <class T>
void MediaSnapshot<T>::Read(TUint aIndex, TUint aCount, /*CancellationToken aCancellationToken,*/ FunctorGeneric<IWatchableFragment<T>*> /*aCallback*/)
{


//    CancellationTokenLink ct = new CancellationTokenLink(iCancellationToken, aCancellationToken);

    auto f = MakeFunctorGeneric<std::vector<T>*>(*this, &MediaSnapshot<T>::ReadCallback);
    iSnapshot->Read(aIndex, aCount, f);

    // Need to pass aCallback and aIndex into functor (aValues comes from the consumer of the functor)


/*
    iSnapshot.Read(ct.Token, aIndex, aCount, (values) =>
    {
        if (!iDisposed)
        {
            aCallback(new WatchableFragment<T>(aIndex, values));
        }
        else
        {
            ASSERT(false);
        }
    });
*/
}

template <class T>
void MediaSnapshot<T>::ReadCallback(std::vector<T>*)
{
/*
    if (!iDisposed)
    {
        aCallback(new WatchableFragment<T>(aIndex, values));
    }
    else
    {
        ASSERT(false);
    }
*/
}


// IDisposable

template <class T>
void MediaSnapshot<T>::Dispose()
{
    iDisposed = true;
}

//////////////////////////////////////////////////////////////////////////

template <class T>
WatchableFragment<T>::WatchableFragment(TUint aIndex, std::vector<T>& aData)
    :iIndex(aIndex)
    ,iData(aData)
{
}

// IWatchableFragment<T>

template <class T>
WatchableFragment<T>::~WatchableFragment()
{

}

template <class T>
TUint WatchableFragment<T>::Index()
{
    return(iIndex);
}

template <class T>
std::vector<T>& WatchableFragment<T>::Data()
{
    return(iData);
}

} // Topology
} // OpenHome
