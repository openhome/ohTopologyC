#include <OpenHome/Topology4.h>


using namespace OpenHome;
using namespace OpenHome::Topology;
using namespace std;


Topology4Source::Topology4Source(IWatchableThread& aThread, IProxyCredentials& aProxy, ITopology2Source& aSource)
    :iThread(aThread)
    ,iProxy(aProxy)
    ,iSource(aSource)
{
}

void Topology4Source::Dispose()
{
}

TUint Topology4Source::Index()
{
    return iSource.Index();
}

Brn Topology4Source::Name()
{
    return iSource.Name();
}

Brn Topology4Source::Type()
{
        return iSource.Type();
}

TBool Topology4Source::Visible()
{
        return iSource.Visible();
}

void Topology4Source::Create(const Brx& aId, FunctorGeneric<ICredentialsSubscription*> aCallback)
{
//    if (iProxy != NULL)
//    {
//        iProxy.Create(aId, aCallback);
//    }
//    else
//    {
//        aCallback(new CredentialsSubscriptionNull(iThread));
//    }
}


/////////////////////////////////////////////////////////////////////


Topology4Group::Topology4Group(INetwork& aNetwork, ITopology3Group& aGroup, ILog& /*aLog*/)
    :iNetwork(aNetwork)
    ,iGroup(aGroup)
    ,iProxy(NULL)
    //,iLog(aLog)
    ,iGroupWatcher(NULL)
{
    auto v = iGroup.Sources();
    for (TUint i = 0; i<v.size(); i++)
    {
        auto w = v[i];
        w->AddWatcher(*this);
    }
}

Topology4Group::Topology4Group(INetwork& aNetwork, ITopology3Group& aGroup, IProxyCredentials& aProxy, ILog& /*aLog*/)
    :iNetwork(aNetwork)
    ,iGroup(aGroup)
    ,iProxy(&aProxy)
    //,iLog(aLog)
    ,iGroupWatcher(NULL)
{
    auto v = iGroup.Sources();
    for (TUint i = 0; i<v.size(); i++)
    {
        auto w = v[i];
        w->AddWatcher(*this);
    }
}


Topology4Group::~Topology4Group()
{
    delete iGroupWatcher;

    for(auto it=iSourceLookup.begin(); it!=iSourceLookup.end(); it++)
    {
        delete it->second;
    }

    for(auto it=iSources.begin(); it!=iSources.end(); it++)
    {
        delete *it;
    }

    iSourceLookup.clear();
}


void Topology4Group::Dispose()
{
    //iProxy->Dispose();
    //foreach (IWatchable<ITopology2Source> w in iGroup.Sources())

    auto t3GroupSources = iGroup.Sources();
    for (TUint i = 0; i<t3GroupSources.size(); i++)
    {
        auto watchable = t3GroupSources[i];
        watchable->RemoveWatcher(*this);
    }

    for (TUint i = 0; i<iWatchableSources.size(); i++)
    {
        iWatchableSources[i]->Dispose();
    }

    ASSERT(iSourceLookup.size() == 0);
}

Brn Topology4Group::Id()
{
    return iGroup.Id();
}

Brn Topology4Group::Attributes()
{
    return iGroup.Attributes();
}

Brn Topology4Group::ModelName()
{
    return iGroup.ModelName();
}

Brn Topology4Group::ManufacturerName()
{
    return iGroup.ManufacturerName();
}

Brn Topology4Group::ProductId()
{
    return iGroup.ProductId();
}

Brn Topology4Group::ProductImageUri()
{
    return iGroup.ProductImageUri();
}

IDevice& Topology4Group::Device()
{
    return iGroup.Device();
}

IWatchable<Brn>& Topology4Group::RoomName()
{
    return iGroup.RoomName();
}

IWatchable<Brn>& Topology4Group::Name()
{
    return iGroup.Name();
}

IWatchable<TBool>& Topology4Group::Standby()
{
    return iGroup.Standby();
}

IWatchable<TUint>& Topology4Group::SourceIndex()
{
    return iGroup.SourceIndex();
}

vector<Watchable<ITopology4Source*>*>& Topology4Group::Sources()
{
    return iWatchableSources;
}

IWatchable<ISender*>& Topology4Group::Sender()
{
    return iGroup.Sender();
}

void Topology4Group::SetStandby(TBool aValue)
{
    iGroup.SetStandby(aValue);
}

void Topology4Group::SetSourceIndex(TUint aValue)
{
    iGroup.SetSourceIndex(aValue);
}

void Topology4Group::ItemOpen(const Brx& aId, ITopology2Source* aValue)
{
    //auto w = new Watchable<ITopology4Source*>(iNetwork, string.Format("{0}({1})", Id, iWatchableSources.Count.ToString()), new Topology4Source(iNetwork, iProxy, aValue));
    ITopology4Source* t4Source = new Topology4Source(iNetwork, *iProxy, *aValue);
    auto w = new Watchable<ITopology4Source*>(iNetwork, aId, t4Source);
    iWatchableSources.push_back(w);
    iSources.push_back(t4Source);
    iSourceLookup[aValue] = w;
}

void Topology4Group::ItemUpdate(const Brx& aId, ITopology2Source* aValue, ITopology2Source* aPrevious)
{
    auto w = iSourceLookup[aPrevious];
    ITopology4Source* newT4Source = new Topology4Source(iNetwork, *iProxy, *aValue);
    iSources.push_back(newT4Source);

    auto source = w->Value();
    w->Update(newT4Source);

    auto it = find(iSources.begin(), iSources.end(), source);
    ASSERT(it!=iSources.end());
    iSources.erase(it);
    delete source;

    iSourceLookup.erase(aPrevious);
    iSourceLookup[aValue] = w;
}

void Topology4Group::ItemClose(const Brx& aId, ITopology2Source* aValue)
{
    auto w = iSourceLookup[aValue];

    auto it = find(iSources.begin(), iSources.end(), w->Value());
    ASSERT(it!=iSources.end());
    delete *it;
    iSources.erase(it);


    auto it2 = find(iWatchableSources.begin(), iWatchableSources.end(), w);
    ASSERT(it2 != iWatchableSources.end());
    iWatchableSources.erase(it2);

    w->Dispose();
    delete w;
    iSourceLookup.erase(aValue);
}


ITopology4GroupWatcher* Topology4Group::GroupWatcher()
{
    return(iGroupWatcher);
}


void Topology4Group::SetGroupWatcher(ITopology4GroupWatcher* aGroupWatcher)
{
    delete iGroupWatcher;
    iGroupWatcher = aGroupWatcher;
}

/////////////////////////////////////////////////////////////////////


Topology4::Topology4(ITopology3* aTopology3, ILog& aLog)
    :iTopology3(aTopology3)
    ,iLog(aLog)
    ,iNetwork(aTopology3->Network())
    ,iDisposeHandler(new DisposeHandler())
    ,iDisposed(false)
    ,iGroups(new WatchableUnordered<ITopology4Group*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology4::WatchT3Groups), NULL);
}


Topology4::~Topology4()
{
    delete iDisposeHandler;
    delete iGroups;

    for(auto it=iGroupsLookup.begin(); it!=iGroupsLookup.end(); it++)
    {
        delete it->second;
    }

    iGroupsLookup.clear();

    delete iTopology3;
}

void Topology4::WatchT3Groups(void*)
{
    iTopology3->Groups().AddWatcher(*this);
}

void Topology4::Dispose()
{
    iDisposeHandler->Dispose();

    iDisposed = true;

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology4::DisposeCallback), NULL);

    iTopology3->Dispose();

    iGroups->Dispose();
}


void Topology4::DisposeCallback(void*)
{
    iTopology3->Groups().RemoveWatcher(*this);

    for(auto it=iGroupsLookup.begin(); it!=iGroupsLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopology4Group*>& Topology4::Groups()
{
    DisposeLock lock(*iDisposeHandler);
    return *iGroups;
}

INetwork& Topology4::Network()
{
    DisposeLock lock(*iDisposeHandler);
    return iNetwork;
}

void Topology4::UnorderedOpen()
{
}

void Topology4::UnorderedInitialised()
{
}

void Topology4::UnorderedClose()
{
}

void Topology4::UnorderedAdd(ITopology3Group* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology4::UnorderedAddCallback), aItem);
/*
    iDisposeHandler.WhenNotDisposed(() =>
    {
        if (aItem.Attributes.Contains("Credentials"))
        {
            iPendingSubscriptions.Add(aItem);
            aItem.Device.Create<IProxyCredentials>((p) =>
            {
                if (!iDisposed && iPendingSubscriptions.Contains(aItem))
                {
                    CreateGroup(aItem, new Topology4Group(iNetwork, aItem, p, iLog));
                    iPendingSubscriptions.Remove(aItem);
                }
                else
                {
                    p.Dispose();
                }
            });
        }
        else
        {
            CreateGroup(aItem, new Topology4Group(iNetwork, aItem, iLog));
        }
    });
*/
}


void Topology4::UnorderedAddCallback(void* aItem)
{
    auto item = (ITopology3Group*)aItem;
    CreateGroup(*item, new Topology4Group(iNetwork, *item, iLog));
}


void Topology4::CreateGroup(ITopology3Group& aGroup3, Topology4Group* aGroup4)
{
    iGroups->Add(aGroup4);
    iGroupsLookup[&aGroup3] = aGroup4;
}

void Topology4::UnorderedRemove(ITopology3Group* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology4::UnorderedRemoveCallback), aItem);

/*
    iDisposeHandler.WhenNotDisposed(() =>
    {
        if (iPendingSubscriptions.Contains(aItem))
        {
            iPendingSubscriptions.Remove(aItem);
            return;
        }

        // schedule notification of L3 group removal
        Topology4Group group = iGroupsLookup[aItem];
        iGroups.Remove(group);
        iGroupsLookup.Remove(aItem);

        group.Dispose();
    });
*/
}

void Topology4::UnorderedRemoveCallback(void* aItem)
{
    auto item  = (ITopology3Group*)aItem;

    auto it = find(iPendingSubscriptions.begin(), iPendingSubscriptions.end(), item);
    if (it!=iPendingSubscriptions.end())
    {
        iPendingSubscriptions.erase(it);
        return;
    }

    Topology4Group* group = iGroupsLookup[item];
    iGroups->Remove(group);
    iGroupsLookup.erase(item);

    group->Dispose();
    delete group;
}

