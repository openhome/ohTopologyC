#pragma once

#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Fifo.h>
#include <OpenHome/Exception.h>
#include <OpenHome/OhTopologyC.h>
#include <OpenHome/Media.h>
#include <OpenHome/Job.h>

#include <vector>
#include <map>

namespace OpenHome
{
namespace Topology
{

TUint Hash(const Brx& aBuffer);

class IdCacheEntrySession;
class IIdCacheEntry;

struct ReadEntriesData
{
    FunctorGeneric<ReadEntriesData*> iEntriesCallback;
    FunctorGeneric<std::vector<IMediaPreset*>*> iPresetsCallback;
    std::vector<TUint>* iRequestedIds;
    std::vector<IIdCacheEntry*>* iRetrievedEntries;
    TUint iIndex;
    TBool iFunctorsValid = false;
};

///////////////////

struct ReadListData
{
    std::vector<TUint>* iMissingIds;
    std::vector<TUint>* iRequiredIds;
    std::vector<IIdCacheEntry*>* iEntries;
    std::vector<IIdCacheEntry*>* iRetrievedEntries;
    FunctorGeneric<void*> iCallback;
};

///////////////////

class IIdCacheEntry
{
public:
    virtual IMediaMetadata& Metadata() = 0;
    virtual const Brx& Uri() = 0;
};

//////////////////////////////////////////

class IIdCacheSession : public IDisposable
{
public:
    virtual void SetValid(std::vector<TUint>& aValid) = 0;
    virtual void Entries(ReadEntriesData* aReadEntriesData) = 0;

};

//////////////////////////////////////////////////////

class IdCacheSession;

class IIdCache
{
public:
    virtual IdCacheSession* CreateSession(TUint aId, FunctorGeneric<ReadListData*> aFunction) = 0;
    virtual ~IIdCache() {}
};

//////////////////////////////////////////////////////

class IdCache : public IIdCache, public IDisposable
{
public:
    static const Brn kPrefixRadio;
    static const Brn kPrefixPlaylist;
public:
    IdCache(TUint aMaxCacheEntries);
    virtual ~IdCache();

    virtual IdCacheSession* CreateSession(TUint aId, FunctorGeneric<ReadListData*> aFunction);

    // IDisposable
    virtual void Dispose();

    void DestroySession(TUint aSessionId);
    void SetValid(TUint aSessionId, std::vector<TUint>& aValid);
    void Remove(const Brx& aUdn);
    IIdCacheEntry* Entry(TUint aSessionId, TUint aId);
    IIdCacheEntry* AddEntry(TUint aSessionId, TUint aId, IIdCacheEntry* aEntry);

    static void UnpackIdArray(Brh& aIdArrayBuf, std::vector<TUint>& aIdArray);
    static TUint Hash(const Brx& aPrefix, const Brx& aUdn);
    static void NonZeroItems(const std::vector<TUint>& aItems, std::vector<TUint>& aNonZeroItems);


private: // (internal)
    void RemoveEntry();

private:
    DisposeHandler* iDisposeHandler;
    TUint iMaxCacheEntries;
    std::map<TUint, std::map<TUint, IdCacheEntrySession*>*> iCache;
    std::vector<IdCacheEntrySession*> iLastAccessed;
    std::vector<TUint> iSessions;
    TUint iCacheEntries;
    Mutex iMutexCache;
};

//////////////////////////////////////////////////////

class IdCacheSession : public IIdCacheSession
{
public:
    IdCacheSession(TUint aSessionId, FunctorGeneric<ReadListData*> aFunction, IdCache* aCache);
    virtual void Dispose();
    virtual void SetValid(std::vector<TUint>& aValid);
    virtual void Entries(ReadEntriesData* aReadEntriesData);

private:
    Job* CreateJob(ReadEntriesData* aReadEntriesData);
    void CreateJobCallback(void* aReadEntriesData);
    //Task<std::vector<IIdCacheEntry*>> CreateJob(std::vector<TUint>& aIds);
    void Run();

private:
    void GetMissingEntries(void* aObj);

private:
    DisposeHandler* iDisposeHandler;
    TUint iSessionId;
    FunctorGeneric<ReadListData*> iFunction;

    IdCache* iCache;
    Semaphore iSemaQ;
    Fifo<Job*> iFifoHi;
    Fifo<Job*> iFifoLo;
    //Task iTask;
    Mutex iMutexQueueLow;
    ThreadFunctor* iThread;
};

//////////////////////////////////////////////////////

class IdCacheEntry : public IIdCacheEntry
{
public:
    IdCacheEntry(IMediaMetadata* aMetadata, const Brx& aUri);
    virtual IMediaMetadata& Metadata();
    virtual const Brx& Uri();

private:
    IMediaMetadata* iMetadata;
    Bws<1000> iUri;
};

//////////////////////////////////////////////////////

class IdCacheEntrySession : public IIdCacheEntry
{
public:
    IdCacheEntrySession(TUint aSessionId, TUint aId, IIdCacheEntry* aCacheEntry);
    virtual TUint SessionId();
    virtual TUint Id();
    virtual IMediaMetadata& Metadata();
    virtual const Brx& Uri();

private:
    TUint iSessionId;
    TUint iId;
    IIdCacheEntry* iCacheEntry;
};

} // namespace Topology
} // namespace OpenHome


namespace std
{
    template <> struct hash<OpenHome::Brn>
    {
        size_t operator()(const OpenHome::Brn& aX) const
        {
            return OpenHome::Topology::Hash(aX);
        }
    };
}
