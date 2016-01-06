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
    for(auto it1 = iCache.begin(); it1 != iCache.end(); ++it1)
    {
        for(auto it2 = it1->second->begin(); it2 != it1->second->end(); ++it2)
        {
            delete it2->second;
        }
        delete it1->second;
    }
    delete iDisposeHandler;
}


void IdCache::Dispose()
{
    ASSERT(iSessions.size() == 0);

    iDisposeHandler->Dispose();
    iCache.clear();
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


std::unique_ptr<IdCacheSession> IdCache::CreateSession(TUint aId, FunctorGeneric<ReadListData*> aFunction)
{
    DisposeLock lock(*iDisposeHandler);

    AutoMutex mutex(iMutexCache);
    iSessions.push_back(aId);
    if (iCache.count(aId)==0)
    {
        iCache[aId] = new map<TUint, IdCacheEntrySession*>();
    }

    return(std::unique_ptr<IdCacheSession>(new IdCacheSession(aId, aFunction, this)));
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
            iCache.erase(h);
        }
    }

/*
    using (iDisposeHandler.Lock())
    {
        lock (iCache)
        {
            Hash playlistHash = Hash.Create(string.Format(ServicePlaylist.kCacheIdFormat, aUdn));
            Hash radioHash = Hash.Create(string.Format(ServicePlaylist.kCacheIdFormat, aUdn));
            List<Hash> keys = new List<Hash>(iCache.Keys);
            foreach(Hash h in keys)
            {
                if (h == playlistHash || h == radioHash)
                {
                    iCache.Remove(h);
                }
            }
        }
    }
*/
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

    auto c = iCache[aSessionId];
    auto it = c->begin();

    while(it!=c->end())
    {
        TUint key = it->first;
        auto itv = find(aValid.begin(), aValid.end(), key);

        if (itv != aValid.end())
        {
            it++;
            c->erase(key);
            --iCacheEntries;
            continue;
        }
        it++;
    }
/*
    using (iDisposeHandler.Lock())
    {
        lock (iCache)
        {
            Dictionary<uint, IdCacheEntrySession> c = iCache[aSessionId];
            List<uint> keys = new List<uint>(c.Keys);
            foreach (uint k in keys)
            {
                if (!aValid.Contains(k))
                {
                    c.Remove(k);
                    --iCacheEntries;
                }
            }
        }
    }
*/
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
/*
    using (iDisposeHandler.Lock())
    {
        lock (iCache)
        {
            IdCacheEntrySession entry;
            if (iCache[aSessionId].TryGetValue(aId, out entry))
            {
                iLastAccessed.Remove(entry);
                iLastAccessed.Add(entry);
                return entry;
            }
            return null;
        }
    }
*/
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
    iCache[entry->SessionId()]->erase(entry->Id());
    iLastAccessed.erase(iLastAccessed.begin());
    --iCacheEntries;
}


IdCacheSession::IdCacheSession(TUint aSessionId, FunctorGeneric<ReadListData*> aFunction, IdCache* aCache)
    : iDisposeHandler(new DisposeHandler())
    , iSessionId(aSessionId)
    , iFunction(aFunction)  // radio/playlist ReadList method
    , iCache(aCache)
    , iSemaQ("IDCQ", 0)
    , iSemaJob("IDCJ", 0)
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
                Job* job = iFifoHi.Read();
                job->Start();
                iSemaJob.Wait();
            }
            while (iFifoLo.SlotsUsed()>0)
            {
                Job* job = iFifoLo.Read();
				if (job != NULL)
				{
					job->Start();
					iSemaJob.Wait();
				}

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
	iFifoLo.Write(NULL);
	iSemaQ.Signal();

	iThread->Join();
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

            auto v = new vector<TUint>();
            v->push_back(id);
            auto readEntriesData = new ReadEntriesData();
            readEntriesData->iRequestedIds = v;
            FunctorGeneric<ReadEntriesData*> f1; // NULL functor
            readEntriesData->iEntriesCallback = f1;
            FunctorGeneric<std::vector<IMediaPreset*>*> f2; // NULL functor
            readEntriesData->iPresetsCallback = f2;
            readEntriesData->iFunctorsValid = false;

            auto job = CreateJob(readEntriesData);
            ASSERT(job != NULL);
            iFifoLo.Write(job);

            iMutexQueueLow.Signal();
            iSemaQ.Signal();
        }
    }
}


void IdCacheSession::Entries(ReadEntriesData* aReadEntriesData)
{
    DisposeLock lock(*iDisposeHandler);

    Job* job = CreateJob(aReadEntriesData);
    ASSERT(job != NULL);
    iFifoHi.Write(job);
    iSemaQ.Signal();
}


//Task<IEnumerable<IIdCacheEntry>> CreateJob(IEnumerable<uint> aIds)
Job* IdCacheSession::CreateJob(ReadEntriesData* aReadEntriesData)
{
    // I think this method should be renamed with something more meaningful
    // perhaps something with "Entries" in the title
    // I propose "CreateEntriesJob"

    Job* job = new Job(MakeFunctorGeneric(*this, &IdCacheSession::CreateJobCallback), aReadEntriesData);
    ASSERT(job != NULL)
    return(job);
/*
    create a new job/thread (with CreateJobCallback that does the stuff below, and calls aCallback with result (vector<IIdCacheEntry*>))
*/

/*
    Task<IEnumerable<IIdCacheEntry>> task = new Task<IEnumerable<IIdCacheEntry>>(() =>
    {
        List<IIdCacheEntry> entries = new List<IIdCacheEntry>();
        List<uint> ids = new List<uint>();

        // find all entries currently in cache and build a list of ids required to be fetched
        foreach (uint id in aIds)
        {
            IIdCacheEntry entry = iCache.Entry(iSessionId, id);
            if (entry == null)
            {
                ids.Add(id);
            }
            entries.Add(entry);
        }

        if (ids.Count == 0) // found them all
        {
            return entries;
        }

        // fetch missing ids
        IEnumerable<IIdCacheEntry> result = iFunction(ids).Result;

        // add retrieved ids to cache
        uint index = 0;
        foreach (IIdCacheEntry e in result)
        {
            uint id = ids.ElementAt((int)index);
            IIdCacheEntry entry = iCache.AddEntry(iSessionId, id, e);
            entries[aIds.ToList().IndexOf(ids[(int)index])] = entry;
            ++index;
        }

        return entries;
    });
    return task;
*/
}


void IdCacheSession::CreateJobCallback(void* aReadEntriesData)
{
    // I propose renaming this method : "CreateEntriesJobCallback"
    ReadEntriesData* readEntriesData = (ReadEntriesData*)aReadEntriesData;
    vector<TUint>* reqIds = readEntriesData->iRequestedIds;

    auto entries = new vector<IIdCacheEntry*>();
    auto missingIds = new vector<TUint>();

    readEntriesData->iRetrievedEntries = entries;

    // find all entries currently in cache and build a list of ids required to be fetched
    for (TUint i=0; i<reqIds->size(); i++)
    {
        auto id = (*reqIds)[i];
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
        if (readEntriesData->iFunctorsValid)
        {
            readEntriesData->iEntriesCallback(readEntriesData);
        }
        iSemaJob.Signal();
        return;
    }

    // fetch missing ids
    auto readListData = new ReadListData();
    readListData->iMissingIds = missingIds;
    readListData->iRequiredIds = reqIds;
    readListData->iEntries = entries;

    readListData->iCallback = MakeFunctorGeneric(*this, &IdCacheSession::GetMissingEntries);

    iFunction(readListData);  // this calls ServicePlaylistNetwork::ReadList or  ServiceRadioNetwork::ReadList

    if (readEntriesData->iFunctorsValid)
    {
        readEntriesData->iEntriesCallback(readEntriesData);
    }

    iSemaJob.Signal();
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
            auto it = find(requiredIds->begin(), requiredIds->end(), id);
            ASSERT(it != requiredIds->end());
            (*entries)[it - requiredIds->begin()] = entry;
        }
    }

    delete payload;


/*
        // add retrieved ids to cache
        uint index = 0;
        foreach (IIdCacheEntry e in result)  // result = IEnumerable<IIdCacheEntry>
        {
            uint id = ids.ElementAt((int)index);
            IIdCacheEntry entry = iCache.AddEntry(iSessionId, id, e);
            entries[aIds.ToList().IndexOf(ids[(int)index])] = entry;
            ++index;
        }
*/


}


//////////////////////////////////////////////////////////////////////

IdCacheEntry::IdCacheEntry(IMediaMetadata* aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
    LOG(kApplication7, "IdCacheEntry()\n");
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



#endif // HEADER_IDCACHE
