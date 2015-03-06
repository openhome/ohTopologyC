#ifndef HEADER_IDCACHE
#define HEADER_IDCACHE


#include <OpenHome/IdCache.h>
#include <vector>
#include <map>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;


IdCache::IdCache(TUint aMaxCacheEntries)
    :iDisposeHandler(new DisposeHandler())
    ,iMaxCacheEntries(aMaxCacheEntries)
    ,iCacheEntries(0)
    ,iMutexCache("IDCH")
{
}


IdCache::~IdCache()
{
    delete iDisposeHandler;
}


void IdCache::Dispose()
{
    if (iSessions.size() > 0)
    {
        //throw new Exception("IdCache disposed with active sessions");
    }

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


vector<TUint> IdCache::NonZeroItems(vector<TUint>& aItems)
{
    vector<TUint> nonZero;

    for(TUint i=0; i<aItems.size(); i++)
    {
        TUint item = aItems[i];
        if (item!=0)
        {
            nonZero.push_back(item);
        }
    }

    ASSERT(nonZero.size()>0);
    return(nonZero);
}


void IdCache::Remove(const Brx& aUdn)
{
    DisposeLock lock(*iDisposeHandler);
    AutoMutex mutex(iMutexCache);

    TUint playlistHash = Hash(kCacheIdPrefixPlaylist, aUdn);
    TUint radioHash = Hash(kCacheIdPrefixRadio, aUdn);

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
    for(auto it = c->begin(); it!=c->end(); it++)
    {
        TUint key = it->first;
        auto itv = find(aValid.begin(), aValid.end(), key);

        if (itv != aValid.end())
        {
            c->erase(key);
            --iCacheEntries;
        }
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

    IdCacheEntrySession* entry = NULL;

    AutoMutex mutex(iMutexCache);

    if (iCache[aSessionId]->count(aId)==0)
    {
        /*IdCacheEntrySession&*/ entry = new IdCacheEntrySession(aSessionId, aId, aEntry);

        if (iCacheEntries == iMaxCacheEntries)
        {
            RemoveEntry();
        }

        (*iCache[aSessionId])[aId] = entry;
        iLastAccessed.push_back(entry);
        ++iCacheEntries;
    }

    return entry;
/*
    using (iDisposeHandler.Lock())
    {
        IdCacheEntrySession entry;

        lock (iCache)
        {
            if (!iCache[aSessionId].TryGetValue(aId, out entry))
            {
                entry = new IdCacheEntrySession(aSessionId, aId, aEntry);

                if (iCacheEntries == iMaxCacheEntries)
                {
                    RemoveEntry();
                }

                iCache[aSessionId].Add(aId, entry);
                iLastAccessed.Add(entry);
                ++iCacheEntries;
            }
        }

        return entry;
    }
*/
}


void IdCache::RemoveEntry()
{
    DisposeLock lock(*iDisposeHandler);

    AutoMutex mutex(iMutexCache);

    IdCacheEntrySession* entry = iLastAccessed[0];
    iCache[entry->SessionId()]->erase(entry->Id());
    iLastAccessed.erase(iLastAccessed.begin());
    --iCacheEntries;
/*
    using (iDisposeHandler.Lock())
    {
        lock (iCache)
        {
            IdCacheEntrySession entry = iLastAccessed[0];
            iCache[entry.SessionId].Remove(entry.Id);
            iLastAccessed.RemoveAt(0);
            --iCacheEntries;
        }
    }
*/
}


IdCacheSession::IdCacheSession(TUint aSessionId, FunctorGeneric<ReadListData*> aFunction, IdCache* aCache)
    :iDisposeHandler(new DisposeHandler())
    ,iSessionId(aSessionId)
    ,iFunction(aFunction)  // radio/playlist ReadList method
    ,iCache(aCache)
    ,iSemaQ("IDCQ", 0)
    ,iFifoHi(10)
    ,iFifoLo(10)
    ,iMutexQueueLow("IDCX")
{

    iThread = new ThreadFunctor("IDCT", MakeFunctor(*this, &IdCacheSession::Run) );
    iThread->Start();

/*
    iTask = Task.Factory.StartNew(() =>
    {
        while (true)
        {
            int result = Semaphore.WaitAny(new WaitHandle[] { iSemaphoreHigh, iSemaphoreLow });
            Task<IEnumerable<IIdCacheEntry>> job = null;
            switch (result)
            {
                case 0:
                    lock (iQueueHigh)
                    {
                        job = iQueueHigh.Dequeue();
                    }
                    break;
                case 1:
                    lock (iQueueLow)
                    {
                        job = iQueueLow.Dequeue();
                    }
                    break;
                default:
                    ASSERTS();
                    //Do.Assert(true);
                    break;
            }

            if (job != null)
            {
                job.Start();
                try
                {
                    job.Wait();
                }
                catch
                {
                    // we just want to ensure task has completed - some other piece of code will handle the exception if required
                }
            }
            else
            {
                break;
            }
        }
    }, TaskCreationOptions.LongRunning);

*/
}


void IdCacheSession::Run()
{
    for(;;)
    {
        iSemaQ.Wait();
        while(iFifoHi.SlotsUsed()>0)
        {
            Job* job = iFifoHi.Read();
            job->Start();
            // wait on Job completion ? (iSemaJob.Wait()??)
        }
        while(iFifoLo.SlotsUsed()>0)
        {
            Job* job = iFifoLo.Read();
            job->Start();
            // wait on Job completion ? (iSemaJob.Wait()??)
        }
    }
}


void IdCacheSession::Dispose()
{
    iDisposeHandler->Dispose();
    iCache->DestroySession(iSessionId);

    iMutexQueueLow.Wait();
    iFifoLo.Write(NULL);
    iMutexQueueLow.Signal();

    iSemaQ.Signal();

    //iTask.Wait();

    delete iDisposeHandler;
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

    Job* job = CreateJob(aReadEntriesData);
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
    return(job);
/*
    create a new job/thread (with CreatJobCallback that does the stuff below, and calls aCallback with result (vector<IIdCacheEntry*>))
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
        readEntriesData->iEntriesCallback(readEntriesData);
    }

    // fetch missing ids
    auto readListData = new ReadListData();
    readListData->iMissingIds = missingIds;
    readListData->iRequiredIds = reqIds;
    readListData->iCallback = MakeFunctorGeneric(*this, &IdCacheSession::GetMissingEntriesCallback);

    iFunction(readListData);
    //iFunction(missingIds, MakeFunctorGeneric(*this, &IdCacheSession::GetMissingEntriesCallback));


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
*/


}


void IdCacheSession::GetMissingEntriesCallback(void* aObj)
{
    auto payload = (ReadListData*)aObj;

    auto missingIds = payload->iMissingIds;
    auto retrievedEntries = payload->iRetrievedEntries;
    auto requiredIds = payload->iRequiredIds;
    auto entries = payload->iEntries;
    auto callback = payload->iCallback;

    for (TUint i=0; i<retrievedEntries->size(); i++)
    {
        TUint id = (*missingIds)[i];
        IIdCacheEntry* entry = iCache->AddEntry(iSessionId, id, (*retrievedEntries)[i]);
        auto it = find(requiredIds->begin(), requiredIds->end(), id);
        ASSERT(it!=requiredIds->end());
        (*entries)[it-requiredIds->begin()] = entry;
    }

    delete payload;
    callback(entries);
}


//////////////////////////////////////////////////////////////////////

IdCacheEntry::IdCacheEntry(IMediaMetadata* aMetadata, const Brx& aUri)
    :iMetadata(aMetadata)
    ,iUri(aUri)
{
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
