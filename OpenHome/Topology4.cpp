#include <OpenHome/Topology4.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



MediaPresetExternal::MediaPresetExternal(IWatchableThread& aThread, Topology4Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology4Source& aSource)
    :iIndex(aIndex)
    ,iMetadata(aMetadata)
    ,iSource(aSource)
    ,iGroup(aGroup)
    ,iBuffering(new Watchable<TBool>(aThread, Brn("Buffering"), false))
    ,iPlaying(new Watchable<TBool>(aThread, Brn("Playing"), false))
    ,iSelected(new Watchable<TBool>(aThread, Brn("Selected"), false))
{

    iGroup.Source().AddWatcher(*this);
}

void MediaPresetExternal::Dispose()
{
    iGroup.Source().RemoveWatcher(*this);
    iBuffering->Dispose();
    iPlaying->Dispose();
    iSelected->Dispose();

}

TUint MediaPresetExternal::Index()
{
    return iIndex;
}

IMediaMetadata& MediaPresetExternal::Metadata()
{
    return *iMetadata;
}

IWatchable<TBool>& MediaPresetExternal::Buffering()
{
    return *iBuffering;
}

IWatchable<TBool>& MediaPresetExternal::Playing()
{
    return *iPlaying;
}

IWatchable<TBool>& MediaPresetExternal::Selected()
{
    return *iSelected;
}

void MediaPresetExternal::Play()
{
    if (!iPlaying->Value())
    {
        iBuffering->Update(true);
    }
    iSource.Select();
}

void MediaPresetExternal::ItemOpen(const Brx& aId, ITopology4Source* aValue)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}

void MediaPresetExternal::ItemUpdate(const Brx& aId, ITopology4Source* aValue, ITopology4Source* aPrevious)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}

void MediaPresetExternal::ItemClose(const Brx& aId, ITopology4Source* aValue)
{
    iBuffering->Update(false);
    iPlaying->Update(false);
    iSelected->Update(false);
}

///////////////////////////////////////////////////////

ITopology4Group& Topology4SourceNull::Group()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

TUint Topology4SourceNull::Index()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}

Brn Topology4SourceNull::Name()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

Brn Topology4SourceNull::Type()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

TBool Topology4SourceNull::Visible()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}

IMediaPreset& Topology4SourceNull::CreatePreset()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

const std::vector<ITopology4Group*> Topology4SourceNull::Volumes()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

IDevice& Topology4SourceNull::Device()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}

TBool Topology4SourceNull::HasInfo()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

TBool Topology4SourceNull::HasTime()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}

///////////////////////////////////////////////////

Topology4Source::Topology4Source(INetwork& aNetwork, Topology4Group& aGroup, ITopology2Source* aSource)
    :iNetwork(aNetwork)
    ,iGroup(aGroup)
    ,iSource(aSource)
    ,iDevice(NULL)
{

}

TUint Topology4Source::Index()
{
    return iSource->Index();
}

Brn Topology4Source::Name()
{
    return iSource->Name();
}

ITopology4Group& Topology4Source::Group()
{
    return iGroup;
}

Brn Topology4Source::Type()
{
    return iSource->Type();
}

TBool Topology4Source::Visible()
{
    return iSource->Visible();
}

IMediaPreset& Topology4Source::CreatePreset()
{

    MediaMetadata* metadata = new MediaMetadata();
    metadata->Add(iNetwork.TagManager().Audio().Title(), iSource->Name());
    //metadata.Add(iNetwork.TagManager.Audio.Artwork, Brn("external://") + iSource.Name);

    // get the root group of this group
    Topology4Group* group = &iGroup;
    while (group->Parent() != NULL)
    {
        group = group->Parent();
    }

    return (*(new MediaPresetExternal(iNetwork, *group, iSource->Index(), metadata, *this)));
}

const std::vector<ITopology4Group*> Topology4Source::Volumes()
{
    return *iVolumes;
}

void Topology4Source::SetVolumes(const std::vector<ITopology4Group*> aVolumes)
{
    iVolumes = &aVolumes;
}

IDevice& Topology4Source::Device()
{
    ASSERT(iDevice!=NULL);
    return(*iDevice);
}

void Topology4Source::SetDevice(IDevice& aDevice)
{
    iDevice = &aDevice;
}

TBool Topology4Source::HasInfo()
{
    return iHasInfo;
}

void Topology4Source::SetHasInfo(TBool aHasInfo)
{
    iHasInfo = aHasInfo;
}


TBool Topology4Source::HasTime()
{
    return iHasTime;
}
void Topology4Source::SetHasTime(TBool aHasTime)
{
    iHasTime = aHasTime;
}

void Topology4Source::Select()
{
    iGroup.SetSourceIndex(iSource->Index());
}

//////////////////////////////////////////////////////////////////

Topology4Group::Topology4Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopologymGroup& aGroup, const std::vector<ITopology2Source*> aSources, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iName(aName)
    ,iGroup(&aGroup)
    ,iLog(aLog)
    ,iDisposed(false)
{
    //iChildren = new vector<Topology4Group>();
    //iSources = new vector<Topology4Source>();
    //iVisibleSources = new vector<ITopology4Source>();
    //iWatchableSource = new Watchable<ITopology4Source>(iNetwork, "source", new Topology4SourceNull());

    vector<ITopology4Group*> v;

    iSenders = new Watchable<vector<ITopology4Group*>>(iNetwork, Brn("senders"), v);

    //foreach (ITopology2Source s in aSources)
    for(TUint i=0; i<aSources.size(); i++)
    {
        Topology4Source* source = new Topology4Source(aNetwork, *this, aSources[i]);
        iSources.push_back(source);
    }

    iGroup->SourceIndex().AddWatcher(*this);

    // if (iGroup.Attributes().Contains(Brn("Sender")))
    if (Ascii::Contains(iGroup->Attributes(), Brn("Sender")))
    {

        iGroup->Device().Create(MakeFunctorGeneric(*this, &Topology4Group::CreateCallback), eProxySender);
/*
        iGroup.Device.Create<IProxySender>((sender) =>
        {
            if (!iDisposed)
            {
                iSender = sender;

                iSender.Status.AddWatcher(this);
            }
            else
            {
                sender.Dispose();
            }
        });
*/
    }

}


void Topology4Group::CreateCallback(void* aSender)
{
    IProxySender* sender = (IProxySender*)aSender;

    if (!iDisposed)
    {
        iSender = sender;

        iSender->Status().AddWatcher(*this);
    }
    else
    {
        sender->Dispose();
    }
}


void Topology4Group::Dispose()
{
    iGroup->SourceIndex().RemoveWatcher(*this);
    //iGroup = null;

    iWatchableSource->Dispose();
    iWatchableSource = NULL;

    if (iSender != NULL)
    {
        iSender->Status().RemoveWatcher(*this);
        iSender->Dispose();
        iSender = NULL;
        iHasSender = false;
    }

    iSenders->Dispose();

    iDisposed = true;
}

Brn Topology4Group::Name()
{
    return iName;
}

IDevice& Topology4Group::Device()
{
    return iGroup->Device();
}

Brn Topology4Group::Room()
{
    return iRoom;
}

Brn Topology4Group::ModelName()
{
    return iGroup->ModelName();
}

Brn Topology4Group::ManufacturerName()
{
    return iGroup->ManufacturerName();
}

Brn Topology4Group::ProductId()
{
    return iGroup->ProductId();
}


IWatchable<ITopologymSender*>& Topology4Group::Sender()
{
    return iGroup->Sender();
}

IWatchable<ITopology4Source*>& Topology4Group::Source()
{
    return *iWatchableSource;
}


ITopologymGroup& Topology4Group::Group()
{
    return(*iGroup);
}


void Topology4Group::EvaluateSources()
{

    TBool hasInfo = Ascii::Contains(iGroup->Attributes() ,Brn("Info"));
    TBool hasTime = (Ascii::Contains(iGroup->Attributes() ,Brn("Time")) && hasInfo);

    // get list of all volume groups
    vector<ITopology4Group*> volumes;// = new vector<ITopology4Group*>();

    Topology4Group* group = this;

    while (group != NULL)
    {
        if(Ascii::Contains(group->Group().Attributes(), Brn("Volume")))
        {
            //volumes.Insert(0, group);
            volumes.insert(volumes.begin(), group);
        }

        group = group->Parent();
    }

    //foreach (Topology4Source s in iSources)
    for(TUint i=0; i<iSources.size(); i++)
    {
        iSources[i]->SetVolumes(volumes);
        iSources[i]->SetDevice(iGroup->Device());
        iSources[i]->SetHasInfo(hasInfo);
        iSources[i]->SetHasTime(hasTime);
    }

    //foreach (Topology4Group g in iChildren)
    for(TUint i=0; i<iChildren.size(); i++)
    {
        iChildren[i]->EvaluateSources();
    }

    for (TUint i= 0; i<iSources.size(); ++i)
    {
        Topology4Source* s = iSources[i];

        TBool expanded = false;

        //foreach (Topology4Group g in iChildren)
        for(TUint i=0; i<iChildren.size(); i++)
        {
            Topology4Group* g = iChildren[i];

            // if group is connected to source expand source to group sources
            if (s->Name() == g->Name())
            {
                //iVisibleSources.AddRange(g->Sources);
                iVisibleSources.insert(iVisibleSources.end(), g->Sources().begin(), g->Sources().end());
                expanded = true;
            }
        }

        if (!expanded)
        {
            // only include if source is visible
            if (s->Visible())
            {
                iVisibleSources.push_back(s);
            }
        }
    }

    ITopology4Source* source = EvaluateSource();
    iWatchableSource->Update(source);
}

const std::vector<ITopology4Source*> Topology4Group::Sources()
{
    return iVisibleSources;
}

void Topology4Group::EvaluateSenders()
{

    //foreach (Topology4Group g in iChildren)
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology4Group* g = iChildren[i];
        g->EvaluateSenders();
    }

    vector<ITopology4Group*> senderDevices;// = new vector<ITopology4Group>();

    //foreach (Topology4Group g in iChildren)
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology4Group* g = iChildren[i];
        //senderDevices.AddRange(g->Senders().Value());

        vector<ITopology4Group*> v(g->Senders().Value());
        senderDevices.insert(senderDevices.begin(), v.begin(), v.end());
    }

    if (iHasSender)
    {
        senderDevices.insert(senderDevices.begin(), this);
    }

    iSenders->Update(senderDevices);
}

IWatchable<std::vector<ITopology4Group*>>& Topology4Group::Senders()
{
    return(*iSenders);
}

Topology4Group* Topology4Group::Parent()
{
    return iParent;
}

TBool Topology4Group::AddIfIsChild(Topology4Group& aGroup)
{
    for(TUint i=0; i<iSources.size(); i++)
    //foreach (Topology4Source s in iSources)
    {
        Topology4Source* s = iSources[i];
        if (aGroup.Name() == s->Name())
        {
            aGroup.SetParent(*this, s->Index());
            iChildren.push_back(&aGroup);

            return true;
        }
    }

    return false;
}

void Topology4Group::SetParent(Topology4Group& aGroup)
{
    iParent = &aGroup;
}

void Topology4Group::SetParent(Topology4Group& aGroup, TUint aIndex)
{
    SetParent(aGroup);
    iParentSourceIndex = aIndex;
}

void Topology4Group::ItemOpen(const Brx& aId, TUint aValue)
{
    iSourceIndex = aValue;
}

void Topology4Group::ItemUpdate(const Brx& aId, TUint aValue, TUint aPrevious)
{
   iSourceIndex = aValue;
   EvaluateSourceFromChild();
}

void Topology4Group::ItemClose(const Brx& aId, TUint aValue)
{
}

ITopology4Source* Topology4Group::EvaluateSource()
{
/*
    if(iSources.Count <= iSourceIndex)
    {
        iLog.Write("EvaluateSource of {0}, iSources.Count={1}, iSourceIndex={2}", iName, iSources.Count(), iSourceIndex);
    }
*/
    // set the source for this group
    Topology4Source* source = iSources[iSourceIndex];

    // check if the group's source is expanded by a child's group's sources

    //foreach (Topology4Group g in iChildren)
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology4Group* g = iChildren[i];
        if (g->Name() == source->Name())
        {
            return g->EvaluateSource();
        }
    }

    return(source);
}

void Topology4Group::EvaluateSourceFromChild()
{
    if (iParent != NULL)
    {
        iParent->EvaluateSourceFromChild();
    }

    ITopology4Source* source = EvaluateSource();
    iWatchableSource->Update(source);

}

void Topology4Group::ItemOpen(const Brx& aId, Brn aValue)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}

void Topology4Group::ItemUpdate(const Brx& aId, Brn aValue, Brn aPrevious)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}

void Topology4Group::ItemClose(const Brx& aId, Brn aValue)
{
}

void Topology4Group::EvaluateSendersFromChild()
{
    if (iParent != NULL)
    {
        iParent->EvaluateSendersFromChild();
    }
    else
    {
        EvaluateSenders();
    }
}

void Topology4Group::SetSourceIndex(TUint aValue)
{
    if (iGroup != NULL)
    {
        if (iParent != NULL)
        {
            iParent->SetSourceIndex(iParentSourceIndex);
        }

        iGroup->SetSourceIndex(aValue);
    }
}


//////////////////////////////////////////////////////////////////////////////

Topology4GroupWatcher::Topology4GroupWatcher(Topology4Room& aRoom, ITopologymGroup& aGroup)
    :iRoom(aRoom)
    ,iGroup(aGroup)
    ,iRoomName(iRoom.Name())
{
    //iSources = new vector<ITopology2Source>();

    iGroup.Name().AddWatcher(*this);
    //foreach (IWatchable<ITopology2Source> s in iGroup.Sources)

    vector<Watchable<ITopology2Source*>*> s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->AddWatcher(*this);
    }
}

void Topology4GroupWatcher::Dispose()
{
    iGroup.Name().RemoveWatcher(*this);
    //foreach (IWatchable<ITopology2Source> s in iGroup.Sources)

    vector<Watchable<ITopology2Source*>*> s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->RemoveWatcher(*this);
    }

    //iGroup = null;
    //iRoom = null;
}

Brn Topology4GroupWatcher::Room()
{
    return iRoomName;
}

Brn Topology4GroupWatcher::Name()
{
    return iName;
}

const std::vector<ITopology2Source*> Topology4GroupWatcher::Sources()
{
    return iSources;
}

void Topology4GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
   iName.Set(aValue);
}

void Topology4GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iName.Set(aValue);
    iRoom.CreateTree();
}

void Topology4GroupWatcher::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
}

void Topology4GroupWatcher::ItemOpen(const Brx& /*aId*/, ITopology2Source* aValue)
{
    iSources.push_back(aValue);
}

void Topology4GroupWatcher::ItemUpdate(const Brx& /*aId*/, ITopology2Source* aValue, ITopology2Source* aPrevious)
{
    vector<ITopology2Source*>::iterator it = find(iSources.begin(), iSources.end(), aPrevious);
    if (it!=iSources.end())
    {
        iSources[it-iSources.end()] = aValue;
        //it->first = aValue;
    }

    iRoom.CreateTree();
}

void Topology4GroupWatcher::ItemClose(const Brx& aId, ITopology2Source* aValue)
{
}


///////////////////////////////////////////////////////////////////


Topology4Room::Topology4Room(INetwork& aNetwork, ITopology3Room& aRoom, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iLog(aLog)
    ,iName(iRoom.Name())
    ,iStandbyCount(0)
    ,iStandby(eOn)
{
    vector<ITopology4Root*> vectorRoot;
    vector<ITopology4Source*> vectorSource;
    vector<ITopology4Registration*> vectorRegistration;

    iWatchableRoots = new Watchable<std::vector<ITopology4Root*>>(iNetwork, Brn("roots"), vectorRoot);
    iWatchableSources = new Watchable<std::vector<ITopology4Source*>>(iNetwork, Brn("sources"), vectorSource);
    iWatchableRegistrations = new Watchable<std::vector<ITopology4Registration*>>(iNetwork, Brn("registration"), vectorRegistration);

    iRoom.Groups().AddWatcher(*this);

}

void Topology4Room::Dispose()
{
    iRoom.Groups().RemoveWatcher(*this);
    //iRoom = null;

    //foreach (var kvp in iGroupLookup)
    map<ITopologymGroup*, Topology4GroupWatcher*>::iterator it;

    for(it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
    {
        it->first->Standby().RemoveWatcher(*this);
        it->second->Dispose();

    }
    //iGroupLookup = null;

    //foreach (Topology4Group g in iGroups)
    for(TUint i=0; i<iGroups.size() ;i++)
    {
        iGroups[i]->Dispose();
    }
    iGroups.clear();
    //iGroups = null;

    iWatchableStandby->Dispose();
    iWatchableStandby = NULL;

    iWatchableRoots->Dispose();
    iWatchableRoots = NULL;

    iWatchableSources->Dispose();
    iWatchableSources = NULL;

    iWatchableRegistrations->Dispose();
    iWatchableRegistrations = NULL;

  //  iRoots = null;

}

Brn Topology4Room::Name()
{
    return iName;
}

IWatchable<EStandby>& Topology4Room::Standby()
{
    return *iWatchableStandby;
}

IWatchable<std::vector<ITopology4Root*>>& Topology4Room::Roots()
{
    return *iWatchableRoots;
}

IWatchable<std::vector<ITopology4Source*>>& Topology4Room::Sources()
{
    return *iWatchableSources;
}

IWatchable<std::vector<ITopology4Registration*>>& Topology4Room::Registrations()
{
    return *iWatchableRegistrations;
}

void Topology4Room::SetStandby(TBool aValue)
{
    iRoom.SetStandby(aValue);
}

void Topology4Room::UnorderedOpen()
{
}

void Topology4Room::UnorderedInitialised()
{
}

void Topology4Room::UnorderedClose()
{
}

void Topology4Room::UnorderedAdd(ITopologymGroup* aItem)
{
    iGroupLookup[aItem] = new Topology4GroupWatcher(*this, *aItem);
    aItem->Standby().AddWatcher(*this);
    CreateTree();

}

void Topology4Room::UnorderedRemove(ITopologymGroup* aItem)
{
    iGroupLookup[aItem]->Dispose();
    iGroupLookup.erase(aItem);
    aItem->Standby().RemoveWatcher(*this);

    if (iGroupLookup.size() > 0)
    {
        CreateTree();
    }
}

void Topology4Room::CreateTree()
{
    //vector<ITopology4Registration> registrations = new vector<ITopology4Registration>();
    //vector<Topology4Group> oldGroups = new vector<Topology4Group>(iGroups);

    vector<ITopology4Registration*> registrations;
    vector<Topology4Group*> oldGroups(iGroups);

    iGroups.clear();
    iRoots.clear();

    //foreach (var kvp in iGroupLookup)

    map<ITopologymGroup*, Topology4GroupWatcher*>::iterator it;
    for(it=iGroupLookup.begin(); it!=iGroupLookup.end(); it++)
    {
        Topology4Group* group = new Topology4Group(iNetwork, it->second->Room(), it->second->Name(), *it->first, it->second->Sources(), iLog);
        InsertIntoTree(*group);
        //if (!string.IsNullOrEmpty(group->ProductId()))
        if (!group->ProductId().Equals(Brx::Empty()))
        {
            registrations.push_back(group);
        }
    }

    vector<ITopology4Root*> roots;
    vector<ITopology4Source*> sources;
    //foreach (Topology4Group g in iRoots)
    for(TUint i=0; i<iRoots.size(); i++)
    {
        Topology4Group* group = iRoots[i];

        group->EvaluateSources();
        group->EvaluateSenders();

        //sources.AddRange(group->Sources());
        sources.insert(sources.end(), group->Sources().begin(), group->Sources().end());
        roots.push_back(group);
    }

    iWatchableRoots->Update(roots);
    iWatchableSources->Update(sources);
    iWatchableRegistrations->Update(registrations);

    //foreach (Topology4Group g in oldGroups)
    for(TUint i=0; i<oldGroups.size(); i++)
    {
        oldGroups[i]->Dispose();
    }

}

void Topology4Room::InsertIntoTree(Topology4Group& aGroup)
{
    // if group is the first group found
    if (iGroups.size() == 0)
    {
        iGroups.push_back(&aGroup);
        iRoots.push_back(&aGroup);
        return;
    }

    // check for an existing parent
    //foreach (Topology4Group g in iGroups)
    for(TUint i=0; i<iGroups.size(); i++)
    {
        if (iGroups[i]->AddIfIsChild(aGroup))
        {
            iGroups.push_back(&aGroup);
            return;
        }
    }

    // check for parent of an existing root
    //foreach (Topology4Group g in iRoots)
    for(TUint i=0; i<iRoots.size(); i++)
    {
        if (aGroup.AddIfIsChild(*iRoots[i]))
        {
            //iRoots.Remove(g);
            iRoots.erase(iRoots.begin()+i);
            break;
        }
    }

    iGroups.push_back(&aGroup);
    iRoots.push_back(&aGroup);
}

void Topology4Room::ItemOpen(const Brx& aId, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount++;
    }

    EvaluateStandby();
}

void Topology4Room::ItemUpdate(const Brx& aId, TBool aValue, TBool aPrevious)
{
    if (aValue)
    {
        ++iStandbyCount;
    }
    else
    {
        --iStandbyCount;
    }

    EvaluateStandby();
}

void Topology4Room::ItemClose(const Brx& aId, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount--;
    }

    EvaluateStandby(iGroupLookup.size() == 0);
}

void Topology4Room::EvaluateStandby()
{
    EvaluateStandby(false);
}

void Topology4Room::EvaluateStandby(TBool aLastGroup)
{
    if (!aLastGroup)
    {
        EStandby standby = eOff;

        if (iStandbyCount > 0)
        {
            standby = eMixed;

            if (iStandbyCount == iGroupLookup.size())
            {
                standby = eOn;
            }
        }

        if (standby != iStandby)
        {
            iStandby = standby;
            iWatchableStandby->Update(standby);
        }
    }

}

///////////////////////////////////////////////////////////////////////////////////

Topology4::Topology4(ITopology3* aTopology3, ILog& aLog)
    :iTopology3(aTopology3)
    ,iLog(aLog)
    ,iNetwork(aTopology3->Network())
    ,iDisposeHandler(new DisposeHandler())
    ,iRooms(new WatchableUnordered<ITopology4Room*>(iNetwork))
{
    //iRoomLookup = new map<ITopology3Room*, Topology4Room*>();
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology4::ScheduleCallback), NULL);
/*
    iNetwork.Schedule(() =>
    {
        iTopology3.Rooms.AddWatcher(this);
    });
*/
}


void Topology4::ScheduleCallback(void*)
{
     iTopology3->Rooms().AddWatcher(*this);
}


void Topology4::Dispose()
{
    iDisposeHandler->Dispose();

    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology4::DisposeCallback), NULL);
/*
    iNetwork.Execute(() =>
    {
        iTopology3.Rooms.RemoveWatcher(this);

        foreach (Topology4Room r in iRoomLookup.Values)
        {
            r.Dispose();
        }
    });
*/
    iRoomLookup.clear();

    iRooms->Dispose();

}


void Topology4::DisposeCallback(void*)
{
    iTopology3->Rooms().RemoveWatcher(*this);

    map<ITopology3Room*, Topology4Room*>::iterator it;

    for(it=iRoomLookup.begin(); it!=iRoomLookup.end(); it++)
    {
        it->second->Dispose();
    }
}


IWatchableUnordered<ITopology4Room*>& Topology4::Rooms()
{
    DisposeLock lock(*iDisposeHandler);
    return(*iRooms);
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

void Topology4::UnorderedAdd(ITopology3Room* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology4::UnorderedAddCallback), aItem);
/*
    iDisposeHandler.WhenNotDisposed(() =>
    {
        Topology4Room room = new Topology4Room(iNetwork, aItem, iLog);
        iRooms.Add(room);
        iRoomLookup.Add(aItem, room);
    });
*/
}


void Topology4::UnorderedAddCallback(void* aT3Room)
{
    ITopology3Room* t3Room = (ITopology3Room*)aT3Room;
    Topology4Room* room = new Topology4Room(iNetwork, *t3Room, iLog);
    iRooms->Add(room);
    iRoomLookup[t3Room] = room;
}


void Topology4::UnorderedRemove(ITopology3Room* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology4::UnorderedRemoveCallback), aItem);
/*
    iDisposeHandler.WhenNotDisposed(() =>
    {
        // schedule notification of L4 room removal
        Topology4Room room = iRoomLookup[aItem];
        iRooms.Remove(room);
        iRoomLookup.Remove(aItem);

        room.Dispose();
    });
*/
}


void Topology4::UnorderedRemoveCallback(void* aT3Room)
{
    // schedule notification of L4 room removal
    ITopology3Room* t3Room = (ITopology3Room*)aT3Room;
    Topology4Room* room = iRoomLookup[t3Room];
    iRooms->Remove(room);
    iRoomLookup.erase(t3Room);
    room->Dispose();
}







