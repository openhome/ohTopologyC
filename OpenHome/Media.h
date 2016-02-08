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
    virtual const std::vector<T>& Data() = 0;
    virtual ~IWatchableFragment() {}

};

////////////////////////////////////////

template <class T>
struct MediaSnapshotCallbackData
{
    FunctorGeneric<IWatchableFragment<T>*> iCallback;
    TUint iIndex;
    std::vector<T>* iValues;
};

/////////////////////////////////////////////////////////

template <class T>
class IWatchableSnapshot
{
public:
    virtual TUint Total() = 0;
    virtual std::vector<TUint>* Alpha() = 0; // null if no alpha map
    virtual void Read(TUint aIndex, TUint aCount, FunctorGeneric<IWatchableFragment<T>*> aCallback) = 0;
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
    virtual void Read(  TUint aIndex,
                        TUint aCount,
                        FunctorGeneric<IWatchableFragment<T>*> aCallback1,
                        FunctorGeneric<MediaSnapshotCallbackData<T>*> aCallback2) = 0;

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
    IWatchableSnapshot<T>* iSnapshotVal;
    Watchable<IWatchableSnapshot<T>*>* iSnapshot;
};

//////////////////////////////////////////////////////////////////////////////////////////////////


template <class T>
class MediaSnapshot : public IWatchableSnapshot<T>, public IDisposable
{
public:
    MediaSnapshot(IMediaClientSnapshot<T>* aSnapshot);
    ~MediaSnapshot();

    TUint Total();
    std::vector<TUint>* Alpha();
    void Read(TUint aIndex, TUint aCount, FunctorGeneric<IWatchableFragment<T>*> aCallback2);

    // IDisposable
    void Dispose();

private:
    void ReadCallback(MediaSnapshotCallbackData<T>* aData);

private:
    IMediaClientSnapshot<T>* iSnapshot;
    TBool iDisposed;
};

////////////////////////////////////////////////////////////////

template <class T>
class WatchableFragment : public IWatchableFragment<T>
{
public:
    WatchableFragment(TUint aIndex, std::vector<T>* aData);
    ~WatchableFragment();

    // IWatchableFragment<T>
    TUint Index();
    const std::vector<T>& Data();

private:
    TUint iIndex;
    std::vector<T>* iData;
};

/////////////////////////////////////////////////////////////////

template <class T>
MediaSupervisor<T>::MediaSupervisor(IWatchableThread& aThread, IMediaClientSnapshot<T>* aClientSnapshot)
    : iSnapshotVal(new MediaSnapshot<T>(aClientSnapshot))
    , iSnapshot(new Watchable<IWatchableSnapshot<T>*>(aThread, Brn("Snapshot"), iSnapshotVal))
{
}

template <class T>
MediaSupervisor<T>::~MediaSupervisor()
{
    delete iSnapshot;
    delete iSnapshotVal;
}

template <class T>
void MediaSupervisor<T>::Dispose()
{
    ((MediaSnapshot<T>*)iSnapshotVal)->Dispose();
    iSnapshot->Dispose();
}


template <class T>
Watchable<IWatchableSnapshot<T>*>& MediaSupervisor<T>::Snapshot()
{
    return (*iSnapshot);
}

template <class T>
void MediaSupervisor<T>::Update(IMediaClientSnapshot<T>* aClientSnapshot)
{
    auto oldSnapshot = iSnapshotVal;

    iSnapshotVal = new MediaSnapshot<T>(aClientSnapshot);
    iSnapshot->Update(iSnapshotVal);

    ((MediaSnapshot<T>*)oldSnapshot)->Dispose();
    delete oldSnapshot;
}

//////////////////////////////////////////////////////////////////////


template <class T>
MediaSnapshot<T>::MediaSnapshot(IMediaClientSnapshot<T>* aSnapshot)
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
void MediaSnapshot<T>::Read(TUint aIndex, TUint aCount, FunctorGeneric<IWatchableFragment<T>*> aCallback)
{
    auto f = MakeFunctorGeneric<MediaSnapshotCallbackData<T>*>(*this, &MediaSnapshot<T>::ReadCallback);
    iSnapshot->Read(aIndex, aCount, aCallback, f);
}

template <class T>
void MediaSnapshot<T>::ReadCallback(MediaSnapshotCallbackData<T>* aData)
{
    FunctorGeneric<IWatchableFragment<T>*> callback = aData->iCallback;
    TUint index = aData->iIndex;
    std::vector<T>* values = aData->iValues;

    if (!iDisposed)
    {
        callback(new WatchableFragment<T>(index, values));
    }
    else
    {
        ASSERTS();
    }

    delete aData;

/*
    iSnapshot.Read(ct.Token, aIndex, aCount, (values) =>
    {
        if (!iDisposed)
        {
            aCallback(new WatchableFragment<T>(aIndex, values));
        }
        else
        {
            Do.Assert(false);
        }
    });

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
WatchableFragment<T>::WatchableFragment(TUint aIndex, std::vector<T>* aData)
    :iIndex(aIndex)
    ,iData(aData)
{
}

// IWatchableFragment<T>

template <class T>
WatchableFragment<T>::~WatchableFragment()
{
    delete iData;
}

template <class T>
TUint WatchableFragment<T>::Index()
{
    return(iIndex);
}

template <class T>
const std::vector<T>& WatchableFragment<T>::Data()
{
    return(*iData);
}

} // Topology
} // OpenHome
