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


void MediaPresetExternal::ItemOpen(const Brx& /*aId*/, ITopology4Source* aValue)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}


void MediaPresetExternal::ItemUpdate(const Brx& /*aId*/, ITopology4Source* aValue, ITopology4Source* /*aPrevious*/)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}


void MediaPresetExternal::ItemClose(const Brx& /*aId*/, ITopology4Source* /*aValue*/)
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


std::vector<ITopology4Group*>& Topology4SourceNull::Volumes()
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
    ,iVolumes(NULL)
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


std::vector<ITopology4Group*>& Topology4Source::Volumes()
{
    return *iVolumes;
}


void Topology4Source::SetVolumes(std::vector<ITopology4Group*>* aVolumes)
{
    iVolumes = aVolumes;
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

Topology4Group::Topology4Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopologymGroup& aGroup, std::vector<ITopology2Source*> aSources, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iName(aName)
    ,iGroup(&aGroup)
    ,iCurrentSource(unique_ptr<ITopology4Source>(new Topology4SourceNull()))
    ,iLog(aLog)
    ,iDisposed(false)
    ,iParent(NULL)
    ,iSender(NULL)
    ,iVectorSenders(new vector<ITopology4Group*>())
{
    iWatchableSource = new Watchable<ITopology4Source*>(iNetwork, Brn("source"), iCurrentSource.get());

    iSenders = new Watchable<vector<ITopology4Group*>*>(iNetwork, Brn("senders"), iVectorSenders);

    for(TUint i=0; i<aSources.size(); i++)
    {
        Topology4Source* source = new Topology4Source(aNetwork, *this, aSources[i]);
        iSources.push_back(source);
    }

    iGroup->SourceIndex().AddWatcher(*this);

    if (Ascii::Contains(iGroup->Attributes(), Brn("Sender")))
    {
        iGroup->Device().Create(MakeFunctorGeneric(*this, &Topology4Group::CreateCallback), eProxySender);
    }

}


void Topology4Group::CreateCallback(void* aArgs)
{
    ArgsTwo<IDevice*, IProxySender*>* args = (ArgsTwo<IDevice*, IProxySender*>*)aArgs;
    IProxySender* sender = args->Arg2();

    if (!iDisposed)
    {
        iSender = sender;
        iSender->Status().AddWatcher(*this);
    }
    else
    {
        sender->Dispose();
        delete sender;
    }
    delete args;
}


void Topology4Group::Dispose()
{
    iGroup->SourceIndex().RemoveWatcher(*this);
    iGroup = NULL;

    iWatchableSource->Dispose();
    delete iWatchableSource;
    iWatchableSource = NULL;

    if (iSender != NULL)
    {
        iSender->Status().RemoveWatcher(*this);
        iSender->Dispose();
        delete iSender;
        iSender = NULL;
        iHasSender = false;
    }

    iSenders->Dispose();
    delete iSenders;

    for(TUint i=0; i<iSources.size(); i++)
    {
        delete iSources[i];
    }

    delete iVectorSenders;

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
    iCurrentVolumes = unique_ptr<vector<ITopology4Group*>>(new vector<ITopology4Group*>());


    Topology4Group* group = this;

    while (group != NULL)
    {
        if(Ascii::Contains(group->Group().Attributes(), Brn("Volume")))
        {
            iCurrentVolumes->insert(iCurrentVolumes->begin(), group);
        }

        group = group->Parent();
    }

    for(TUint i=0; i<iSources.size(); i++)
    {
        iSources[i]->SetVolumes(iCurrentVolumes.get());
        iSources[i]->SetDevice(iGroup->Device());
        iSources[i]->SetHasInfo(hasInfo);
        iSources[i]->SetHasTime(hasTime);
    }

    for(TUint i=0; i<iChildren.size(); i++)
    {
        iChildren[i]->EvaluateSources();
    }

    for (TUint i= 0; i<iSources.size(); ++i)
    {
        Topology4Source* s = iSources[i];

        TBool expanded = false;

        for(TUint i=0; i<iChildren.size(); i++)
        {
            Topology4Group* g = iChildren[i];

            // if group is connected to source expand source to group sources
            if (s->Name() == g->Name())
            {
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


std::vector<ITopology4Source*>& Topology4Group::Sources()
{
    return iVisibleSources;
}


void Topology4Group::EvaluateSenders()
{
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology4Group* g = iChildren[i];
        g->EvaluateSenders();
    }

    vector<ITopology4Group*>* vectorSenders = new vector<ITopology4Group*>();

    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology4Group* g = iChildren[i];

        vector<ITopology4Group*> v(*(g->Senders().Value()));
        vectorSenders->insert(vectorSenders->begin(), v.begin(), v.end());
    }

    if (iHasSender)
    {
        vectorSenders->insert(vectorSenders->begin(), this);
    }

    delete iVectorSenders;
    iVectorSenders = vectorSenders;
    iSenders->Update(iVectorSenders);
}


IWatchable<std::vector<ITopology4Group*>*>& Topology4Group::Senders()
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


void Topology4Group::ItemOpen(const Brx& /*aId*/, TUint aValue)
{
    iSourceIndex = aValue;
}


void Topology4Group::ItemUpdate(const Brx& /*aId*/, TUint aValue, TUint /*aPrevious*/)
{
   iSourceIndex = aValue;
   EvaluateSourceFromChild();
}


void Topology4Group::ItemClose(const Brx& /*aId*/, TUint /*aValue*/)
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

    iWatchableSource->Update(EvaluateSource());
}


void Topology4Group::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology4Group::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology4Group::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
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
    iGroup.Name().AddWatcher(*this);

    vector<Watchable<ITopology2Source*>*> s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->AddWatcher(*this);
    }
}

void Topology4GroupWatcher::Dispose()
{
    iGroup.Name().RemoveWatcher(*this);
    vector<Watchable<ITopology2Source*>*> s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->RemoveWatcher(*this);
    }
}


Brn Topology4GroupWatcher::Room()
{
    return iRoomName;
}


Brn Topology4GroupWatcher::Name()
{
    return iName;
}


std::vector<ITopology2Source*>& Topology4GroupWatcher::Sources()
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
        iSources[it-iSources.begin()] = aValue;
    }

    iRoom.CreateTree();
}

void Topology4GroupWatcher::ItemClose(const Brx& /*aId*/, ITopology2Source* /*aValue*/)
{
}


///////////////////////////////////////////////////////////////////


Topology4Room::Topology4Room(INetwork& aNetwork, ITopology3Room& aRoom, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iLog(aLog)
    ,iName(iRoom.Name())
    ,iStandbyCount(0)
    ,iStandby(eOff)
    ,iCurrentRoots(unique_ptr<vector<ITopology4Root*>>(new vector<ITopology4Root*>()))
    ,iCurrentSources(unique_ptr<vector<ITopology4Source*>>(new vector<ITopology4Source*>()))
    ,iCurrentRegistrations(unique_ptr<vector<ITopology4Registration*>>(new vector<ITopology4Registration*>()))
    ,iWatchableStandby(new Watchable<EStandby>(iNetwork, Brn("standby"), eOff))
    ,iWatchableRoots(new Watchable<vector<ITopology4Root*>*>(iNetwork, Brn("roots"), iCurrentRoots.get()))
    ,iWatchableSources(new Watchable<vector<ITopology4Source*>*>(iNetwork, Brn("sources"), iCurrentSources.get()))
    ,iWatchableRegistrations(new Watchable<vector<ITopology4Registration*>*>(iNetwork, Brn("registration"), iCurrentRegistrations.get()))
{
    iRoom.Groups().AddWatcher(*this);
}


void Topology4Room::Dispose()
{
    iRoom.Groups().RemoveWatcher(*this);
    //iRoom = null;

    map<ITopologymGroup*, Topology4GroupWatcher*>::iterator it;

    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        it->first->Standby().RemoveWatcher(*this);
        it->second->Dispose();
        delete it->second;
    }

    for(TUint i=0; i<iGroups.size() ;i++)
    {
        iGroups[i]->Dispose();
        delete iGroups[i];
    }
    iGroups.clear();

    iWatchableStandby->Dispose();
    iWatchableRoots->Dispose();
    iWatchableSources->Dispose();
    iWatchableRegistrations->Dispose();

    delete iWatchableRoots;
    delete iWatchableSources;
    delete iWatchableRegistrations;
    delete iWatchableStandby;

    iWatchableStandby = NULL;
    iWatchableRoots = NULL;
    iWatchableSources = NULL;
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


IWatchable<std::vector<ITopology4Root*>*>& Topology4Room::Roots()
{
    return *iWatchableRoots;
}


IWatchable<std::vector<ITopology4Source*>*>& Topology4Room::Sources()
{
    return *iWatchableSources;
}


IWatchable<std::vector<ITopology4Registration*>*>& Topology4Room::Registrations()
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
    iGroupWatcherLookup[aItem] = new Topology4GroupWatcher(*this, *aItem);
    aItem->Standby().AddWatcher(*this);
    CreateTree();
}


void Topology4Room::UnorderedRemove(ITopologymGroup* aItem)
{
    iGroupWatcherLookup[aItem]->Dispose();
    delete iGroupWatcherLookup[aItem];
    iGroupWatcherLookup.erase(aItem);
    aItem->Standby().RemoveWatcher(*this);

    if (iGroupWatcherLookup.size() > 0)
    {
        CreateTree();
    }
}


void Topology4Room::CreateTree()
{
    vector<ITopology4Registration*>* registrations = new vector<ITopology4Registration*>();
    vector<Topology4Group*> oldGroups(iGroups);

    iGroups.clear();
    iRoots.clear();

    map<ITopologymGroup*, Topology4GroupWatcher*>::iterator it;
    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        Topology4Group* group = new Topology4Group(iNetwork, it->second->Room(), it->second->Name(), *it->first, it->second->Sources(), iLog);
        InsertIntoTree(*group);

        if (!group->ProductId().Equals(Brx::Empty()))
        {
            registrations->push_back(group);
        }
    }

    vector<ITopology4Root*>* roots = new vector<ITopology4Root*>();
    vector<ITopology4Source*>* sources = new vector<ITopology4Source*>();

    for(TUint i=0; i<iRoots.size(); i++)
    {
        Topology4Group* group = iRoots[i];

        group->EvaluateSources();
        group->EvaluateSenders();

        auto s = group->Sources();
        sources->insert(sources->end(), s.begin(), s.end());

        roots->push_back(group);
    }

    iCurrentRoots = unique_ptr<std::vector<ITopology4Root*>>(roots);
    iCurrentRegistrations = unique_ptr<std::vector<ITopology4Registration*>>(registrations);
    iCurrentSources = unique_ptr<std::vector<ITopology4Source*>>(sources);

    iWatchableRoots->Update(roots);
    iWatchableSources->Update(sources);
    iWatchableRegistrations->Update(registrations);

    for(TUint i=0; i<oldGroups.size(); i++)
    {
        oldGroups[i]->Dispose();
        delete oldGroups[i];
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
    for(TUint i=0; i<iGroups.size(); i++)
    {
        if (iGroups[i]->AddIfIsChild(aGroup))
        {
            iGroups.push_back(&aGroup);
            return;
        }
    }

    // check for parent of an existing root
    for(TUint i=0; i<iRoots.size(); i++)
    {
        if (aGroup.AddIfIsChild(*iRoots[i]))
        {
            iRoots.erase(iRoots.begin()+i);
            break;
        }
    }

    iGroups.push_back(&aGroup);
    iRoots.push_back(&aGroup);

}


void Topology4Room::ItemOpen(const Brx& /*aId*/, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount++;
    }

    EvaluateStandby();
}


void Topology4Room::ItemUpdate(const Brx& /*aId*/, TBool aValue, TBool /*aPrevious*/)
{
    if (aValue)
    {
        iStandbyCount++;
    }
    else
    {
        iStandbyCount--;
    }

    EvaluateStandby();
}


void Topology4Room::ItemClose(const Brx& /*aId*/, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount--;
    }

    EvaluateStandby(iGroupWatcherLookup.size() == 0);
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

            if (iStandbyCount == iGroupWatcherLookup.size())
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
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology4::ScheduleCallback), NULL);
}


void Topology4::ScheduleCallback(void*)
{
     iTopology3->Rooms().AddWatcher(*this);
}


void Topology4::Dispose()
{
    iDisposeHandler->Dispose();
    delete iDisposeHandler;
    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology4::DisposeCallback), NULL);
    iRoomLookup.clear();
    iRooms->Dispose();
    delete iRooms;
    iRooms = NULL;

    iTopology3->Dispose();
    delete iTopology3;
    iTopology3 = NULL;
}


void Topology4::DisposeCallback(void*)
{
    iTopology3->Rooms().RemoveWatcher(*this);

    map<ITopology3Room*, Topology4Room*>::iterator it;

    for(it=iRoomLookup.begin(); it!=iRoomLookup.end(); it++)
    {
        it->second->Dispose();
        delete it->second;
    }

    iRoomLookup.clear();
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
}


void Topology4::UnorderedAddCallback(void* aT3Room)
{
    ITopology3Room* t3Room = (ITopology3Room*)aT3Room;
    Topology4Room* t4Room = new Topology4Room(iNetwork, *t3Room, iLog);
    iRooms->Add(t4Room);
    iRoomLookup[t3Room] = t4Room;
}


void Topology4::UnorderedRemove(ITopology3Room* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology4::UnorderedRemoveCallback), aItem);
}


void Topology4::UnorderedRemoveCallback(void* aT3Room)
{
    // schedule notification of L4 room removal
    ITopology3Room* t3Room = (ITopology3Room*)aT3Room;
    Topology4Room* t4Room = iRoomLookup[t3Room];
    iRooms->Remove(t4Room);
    iRoomLookup.erase(t3Room);
    t4Room->Dispose();
    delete t4Room;
}







