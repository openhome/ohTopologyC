#ifndef HEADER_IDCACHE
#define HEADER_IDCACHE

#include <OpenHome/IdCache.h>
#include <OpenHome/Private/Debug.h>
#include <vector>
#include <map>

using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;

TUint OpenHome::Topology::Hash(const Brx& aBuffer)
{
    TUint hash = 0;
    const TUint bytes = aBuffer.Bytes();
    const TByte* ptr = aBuffer.Ptr();
    for (TUint i=0; i<bytes; i++) {
        hash += *ptr++;
    }
    return hash;
}


const Brn IdCache::kPrefixRadio("Radio");
const Brn IdCache::kPrefixPlaylist("Playlist");

IdCache::IdCache(TUint aMaxCacheEntries)
    :iDisposeHandler(new DisposeHandler())
    ,iMaxCacheEntries(aMaxCacheEntries)
    ,iCacheEntries(0)
    ,iMutexCache("IDCH")
{
}


IdCache::~IdCache()
{
    // delete every IdCacheEntrySession in each map in iCache
    // delete every map in iCache
    for(auto it1 = iCache.begin(); it1 != iCache.end(); ++it1)
    {
        for(auto it2 = it1->second->begin(); it2 != it1->second->end(); ++it2)
        {
            delete it2->second; // delete IdCacheEntrySession
        }
        delete it1->second; // delete map
    }
    delete iDisposeHandler;
}


void IdCache::Dispose()
{
    ASSERT(iSessions.size() == 0);
    iDisposeHandler->Dispose();
    //iCache.clear();
    iLastAccessed.clear();
    iCacheEntries = 0;
}


void IdCache::UnpackIdArray(Brh& aIdArrayBuf, vector<TUint>& aIdArray)
{
    TUint bufBytes = aIdArrayBuf.Bytes();

    ASSERT((bufBytes%4)==0);

    for(TUint i=0; i<bufBytes; i+=4)
    {
        TUint id;
        id = aIdArrayBuf[i+3];
        id |= aIdArrayBuf[i+2]<<8;
        id |= aIdArrayBuf[i+1]<<16;
        id |= aIdArrayBuf[i]<<24;
        aIdArray.push_back(id);
    }
}


IdCacheSession* IdCache::CreateSession(TUint aId, FunctorGeneric<ReadListData*> aFunction)
{
    DisposeLock lock(*iDisposeHandler);

    AutoMutex mutex(iMutexCache);
    iSessions.push_back(aId);
    if (iCache.count(aId)==0)
    {
        iCache[aId] = new map<TUint, IdCacheEntrySession*>();
    }

    return(new IdCacheSession(aId, aFunction, this));
}

TUint IdCache::Hash(const Brx& aPrefix, const Brx& aUdn)
{
    Bws<100> buf;
    buf.Replace(aPrefix);
    buf.Append(Brn("("));
    buf.Append(aUdn);
    buf.Append(Brn(")"));

    std::hash<Brn> hashStr;

    TUint h = hashStr(Brn(buf));
    return(h);
}


void IdCache::NonZeroItems(const vector<TUint>& aItems, vector<TUint>& aNonZeroItems)
{
    for(TUint i=0; i<aItems.size(); i++)
    {
        TUint item = aItems[i];
        if (item!=0)
        {
            aNonZeroItems.push_back(item);
        }
    }
}


void IdCache::Remove(const Brx& aUdn)
{
    DisposeLock lock(*iDisposeHandler);
    AutoMutex mutex(iMutexCache);

    TUint playlistHash = Hash(kPrefixPlaylist, aUdn);
    TUint radioHash = Hash(kPrefixRadio, aUdn);

    vector<TUint>* keys = new vector<TUint>();

    for(auto it=iCache.begin(); it!=iCache.end(); it++)
    {
        keys->push_back(it->first);
    }

    for(TUint i=0; i<keys->size(); i++)
    {
        //if (h == playlistHash || h == radioHash)
        TUint h = (*keys)[i];
        if (h == playlistHash || h == radioHash)
        {
            // erase the map at key 'h' in iCache..

            // but first..

            // delete every entry in map
            for (auto it=iCache[h]->begin(); it!=iCache[h]->end(); it++)
            {
               delete it->second;
            }

            delete iCache[h]; // then delete the map
            iCache.erase(h); // then erase the map from cache
        }
    }
}


void IdCache::DestroySession(TUint aSessionId)
{
    DisposeLock lock(*iDisposeHandler);
    AutoMutex mutex(iMutexCache);

    auto it = find(iSessions.begin(), iSessions.end(), aSessionId);
    if(it!=iSessions.end())
    {
        iSessions.erase(it);
    }
}


void IdCache::SetValid(TUint aSessionId, vector<TUint>& aValid)
{
    DisposeLock lock(*iDisposeHandler);

    AutoMutex mutex(iMutexCache);

    auto c = iCache[aSessionId];  // this gets a  std::map<TUint, IdCacheEntrySession*>*

    // iterate through the map
    for (auto it = c->begin(); it!=c->end(); it++)
    {
        // for each key in the map...find the key in the aValid vector
        TUint key = it->first;
        auto itv = find(aValid.begin(), aValid.end(), key);

        if (itv == aValid.end())
        {
            // if the key exists in the vector...
            IdCacheEntrySession* ces = (*c)[key];
            delete ces; // delete the session assoc with that key
            c->erase(key);  // erase the session from the map
            --iCacheEntries; // dec the entry count
        }
    }
}


IIdCacheEntry* IdCache::Entry(TUint aSessionId, TUint aId)
{
    DisposeLock lock(*iDisposeHandler);

    AutoMutex mutex(iMutexCache);

    if (iCache[aSessionId]->count(aId)>0)
    {
        IdCacheEntrySession* entry = (*iCache[aSessionId])[aId];

        auto it = find(iLastAccessed.begin(), iLastAccessed.end(), entry);
        if (it != iLastAccessed.end())
        {
            iLastAccessed.erase(it);
        }

        iLastAccessed.push_back(entry);
        return(entry);
    }
    else
    {
        // THROW here ?
        return NULL;
    }
}


IIdCacheEntry* IdCache::AddEntry(TUint aSessionId, TUint aId, IIdCacheEntry* aEntry)
{
    DisposeLock lock(*iDisposeHandler);
    AutoMutex mutex(iMutexCache);

    IdCacheEntrySession* entry = NULL;
    if (iCache[aSessionId]->count(aId) == 0)
    {
        entry = new IdCacheEntrySession(aSessionId, aId, aEntry);

        if (iCacheEntries == iMaxCacheEntries)
        {
            RemoveEntry();
        }

        (*iCache[aSessionId])[aId] = entry;
        iLastAccessed.push_back(entry);
        ++iCacheEntries;
    }

    return entry;
}


void IdCache::RemoveEntry()
{
    // must be called with iDisposeHandler, iMutexCache held
    IdCacheEntrySession* entry = iLastAccessed[0];
    auto mp = iCache[entry->SessionId()];
    delete (*mp)[entry->Id()];
    mp->erase(entry->Id());
    iLastAccessed.erase(iLastAccessed.begin());
    --iCacheEntries;
}


IdCacheSession::IdCacheSession(TUint aSessionId, FunctorGeneric<ReadListData*> aFunction, IdCache* aCache)
    : iDisposeHandler(new DisposeHandler())
    , iSessionId(aSessionId)
    , iFunction(aFunction)  // radio/playlist ReadList method
    , iCache(aCache)
    , iSemaQ("IDCQ", 0)
    , iFifoHi(10)
    , iFifoLo(10)
    , iMutexQueueLow("IDCX")
{

    iThread = new ThreadFunctor("IdCacheSession", MakeFunctor(*this, &IdCacheSession::Run) );
    iThread->Start();
}

IdCacheSession::~IdCacheSession()
{
    delete iDisposeHandler;
}

void IdCacheSession::Run()
{
    try
    {
        for (;;)
        {
            iSemaQ.Wait();
            while (iFifoHi.SlotsUsed()>0)
            {

                auto job = iFifoHi.Read();
                job->Execute();
                delete job;
            }
            while (iFifoLo.SlotsUsed()>0)
            {
                auto job = iFifoLo.Read();
                job->Execute();
                delete job;
            }
        }
    }
    catch (FifoReadError&) {}
}


void IdCacheSession::Dispose()
{
    iDisposeHandler->Dispose();
    iCache->DestroySession(iSessionId);

    iFifoHi.ReadInterrupt();
    iFifoLo.ReadInterrupt();

    iSemaQ.Signal();

    while (iFifoHi.SlotsUsed()>0)
    {
        auto job = iFifoHi.Read();
        delete job;
    }
    while (iFifoLo.SlotsUsed()>0)
    {
        auto job = iFifoLo.Read();
        delete job;
    }

    delete iThread;
}


void IdCacheSession::SetValid(vector<TUint>& aValid)
{
    DisposeLock lock(*iDisposeHandler);

    if (aValid.size() > 0)
    {
        iCache->SetValid(iSessionId, aValid);
        for(TUint i=0; i<aValid.size(); i++)
        {
            TUint id = aValid[i];

            iMutexQueueLow.Wait();

            auto readEntriesData = new ReadEntriesData();
            readEntriesData->iRequestedIds.push_back(id);
            FunctorGeneric<ReadEntriesData*> f1; // NULL functor
            readEntriesData->iEntriesCallback = f1;
            FunctorGeneric<std::vector<IMediaPreset*>*> f2; // NULL functor
            readEntriesData->iPresetsCallback = f2;
            readEntriesData->iFunctorsValid = false;

            auto job = CreateJob(readEntriesData);
            iFifoLo.Write(job);

            iMutexQueueLow.Signal();
            iSemaQ.Signal();
        }
    }
}


void IdCacheSession::Entries(ReadEntriesData* aReadEntriesData)
{
    DisposeLock lock(*iDisposeHandler);

    ReadEntriesJob* job = CreateJob(aReadEntriesData);
    iFifoHi.Write(job);
    iSemaQ.Signal();
}


ReadEntriesJob* IdCacheSession::CreateJob(ReadEntriesData* aReadEntriesData)
{
    ReadEntriesJob* job = new ReadEntriesJob(MakeFunctorGeneric(*this, &IdCacheSession::ReadEntriesCallback), aReadEntriesData);
    return(job);
}


void IdCacheSession::ReadEntriesCallback(ReadEntriesData* aReadEntriesData)
{

    //vector<TUint> reqIds = aReadEntriesData->iRequestedIds;

    auto entries = new vector<IIdCacheEntry*>();
    auto missingIds = new vector<TUint>();

    aReadEntriesData->iRetrievedEntries = entries;

    // find all entries currently in cache and build a list of ids required to be fetched
    for (TUint i=0; i<aReadEntriesData->iRequestedIds.size(); i++)
    {
        auto id = aReadEntriesData->iRequestedIds[i];
        IIdCacheEntry* entry = iCache->Entry(iSessionId, id);
        if (entry == NULL)
        {
            missingIds->push_back(id);
        }
        entries->push_back(entry);
    }

    if (missingIds->size() == 0) // found them all
    {
        delete missingIds;
        delete entries;
        if (aReadEntriesData->iFunctorsValid)
        {
            aReadEntriesData->iEntriesCallback(aReadEntriesData);
        }
        else
        {
            delete aReadEntriesData;
        }
        return;
    }

    // fetch missing ids
    auto readListData = new ReadListData();
    readListData->iMissingIds = missingIds;
    readListData->iRequiredIds = aReadEntriesData->iRequestedIds;
    readListData->iEntries = entries;

    readListData->iCallback = MakeFunctorGeneric(*this, &IdCacheSession::GetMissingEntries);

    iFunction(readListData);  // this calls ServicePlaylistNetwork::ReadList or  ServiceRadioNetwork::ReadList

    if (aReadEntriesData->iFunctorsValid)
    {
        aReadEntriesData->iEntriesCallback(aReadEntriesData);
    }
    else
    {
        delete aReadEntriesData;
    }
}


void IdCacheSession::GetMissingEntries(void* aObj)
{
    auto payload = (ReadListData*)aObj;
    auto missingIds = payload->iMissingIds;
    auto retrievedEntries = payload->iRetrievedEntries;
    auto requiredIds = payload->iRequiredIds;
    auto entries = payload->iEntries;


    if (retrievedEntries != NULL)
    {
        for (TUint i = 0; i < retrievedEntries->size(); i++)
        {
            TUint id = (*missingIds)[i];
            IIdCacheEntry* entry = iCache->AddEntry(iSessionId, id, (*retrievedEntries)[i]);
            auto it = find(requiredIds.begin(), requiredIds.end(), id);
            ASSERT(it != requiredIds.end());
            (*entries)[it - requiredIds.begin()] = entry;
        }
    }

    delete payload;
}


//////////////////////////////////////////////////////////////////////

IdCacheEntry::IdCacheEntry(IMediaMetadata* aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
    LOG(kApplication7, "IdCacheEntry()\n");
}

IdCacheEntry::~IdCacheEntry()
{
    delete iMetadata;
}

IMediaMetadata& IdCacheEntry::Metadata()
{
    return(*iMetadata);
}


const Brx& IdCacheEntry::Uri()
{
    return(iUri);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

IdCacheEntrySession::IdCacheEntrySession(TUint aSessionId, TUint aId, IIdCacheEntry* aCacheEntry)
    :iSessionId(aSessionId)
    ,iId(aId)
    ,iCacheEntry(aCacheEntry)
{
}

IdCacheEntrySession::~IdCacheEntrySession()
{
    delete iCacheEntry;
}
TUint IdCacheEntrySession::SessionId()
{
    return(iSessionId);
}


TUint IdCacheEntrySession::Id()
{
    return(iId);
}


IMediaMetadata& IdCacheEntrySession::Metadata()
{
    return(iCacheEntry->Metadata());
}


const Brx& IdCacheEntrySession::Uri()
{
    return(iCacheEntry->Uri());
}

/////////////////////////////////////////////////

ReadEntriesJob::ReadEntriesJob(FunctorGeneric<ReadEntriesData*> aCallback, ReadEntriesData* aArg)
    :iCallback(aCallback)
    ,iArg(aArg)
{
}


void ReadEntriesJob::Execute()
{
    iCallback(iArg);
}



#endif // HEADER_IDCACHE
