#include <OpenHome/Topology5.h>

#include <map>
#include <algorithm>


using namespace OpenHome;
using namespace OpenHome::Av;
using namespace std;



MediaPresetExternal::MediaPresetExternal(IWatchableThread& aThread, Topology5Group& aGroup, TUint aIndex, IMediaMetadata* aMetadata, Topology5Source& aSource)
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


void MediaPresetExternal::ItemOpen(const Brx& /*aId*/, ITopology5Source* aValue)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}


void MediaPresetExternal::ItemUpdate(const Brx& /*aId*/, ITopology5Source* aValue, ITopology5Source* /*aPrevious*/)
{
    iBuffering->Update(false);
    iPlaying->Update(aValue == &iSource);
    iSelected->Update(aValue == &iSource);
}


void MediaPresetExternal::ItemClose(const Brx& /*aId*/, ITopology5Source* /*aValue*/)
{
    iBuffering->Update(false);
    iPlaying->Update(false);
    iSelected->Update(false);
}

///////////////////////////////////////////////////////

ITopology5Group& Topology5SourceNull::Group()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


TUint Topology5SourceNull::Index()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}


Brn Topology5SourceNull::Name()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


Brn Topology5SourceNull::Type()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


TBool Topology5SourceNull::Visible()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}


IMediaPreset& Topology5SourceNull::CreatePreset()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


std::vector<ITopology5Group*>& Topology5SourceNull::Volumes()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


IDevice& Topology5SourceNull::Device()
{
    THROW(NotImplementedException);
   // throw new NotImplementedException();
}


TBool Topology5SourceNull::HasInfo()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


TBool Topology5SourceNull::HasTime()
{
    THROW(NotImplementedException);
    //throw new NotImplementedException();
}


///////////////////////////////////////////////////

Topology5Source::Topology5Source(INetwork& aNetwork, Topology5Group& aGroup, ITopology2Source* aSource)
    :iNetwork(aNetwork)
    ,iGroup(aGroup)
    ,iSource(aSource)
    ,iVolumes(NULL)
    ,iDevice(NULL)
{

}


TUint Topology5Source::Index()
{
    return iSource->Index();
}

Brn Topology5Source::Name()
{
    return iSource->Name();
}


ITopology5Group& Topology5Source::Group()
{
    return iGroup;
}


Brn Topology5Source::Type()
{
    return iSource->Type();
}


TBool Topology5Source::Visible()
{
    return iSource->Visible();
}


IMediaPreset& Topology5Source::CreatePreset()
{

    MediaMetadata* metadata = new MediaMetadata();
    metadata->Add(iNetwork.TagManager().Audio().Title(), iSource->Name());
    //metadata.Add(iNetwork.TagManager.Audio.Artwork, Brn("external://") + iSource.Name);

    // get the root group of this group
    Topology5Group* group = &iGroup;
    while (group->Parent() != NULL)
    {
        group = group->Parent();
    }

    return (*(new MediaPresetExternal(iNetwork, *group, iSource->Index(), metadata, *this)));
}


std::vector<ITopology5Group*>& Topology5Source::Volumes()
{
    return *iVolumes;
}


void Topology5Source::SetVolumes(std::vector<ITopology5Group*>* aVolumes)
{
    iVolumes = aVolumes;
}


IDevice& Topology5Source::Device()
{
    ASSERT(iDevice!=NULL);
    return(*iDevice);
}


void Topology5Source::SetDevice(IDevice& aDevice)
{
    iDevice = &aDevice;
}


TBool Topology5Source::HasInfo()
{
    return iHasInfo;
}


void Topology5Source::SetHasInfo(TBool aHasInfo)
{
    iHasInfo = aHasInfo;
}


TBool Topology5Source::HasTime()
{
    return iHasTime;
}


void Topology5Source::SetHasTime(TBool aHasTime)
{
    iHasTime = aHasTime;
}


void Topology5Source::Select()
{
    iGroup.SetSourceIndex(iSource->Index());
}

//////////////////////////////////////////////////////////////////

Topology5Group::Topology5Group(INetwork& aNetwork, const Brx& aRoom, const Brx& aName, ITopology3Group& aGroup, std::vector<ITopology2Source*> aSources, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iName(aName)
    ,iGroup(&aGroup)
    ,iCurrentSource(unique_ptr<ITopology5Source>(new Topology5SourceNull()))
    ,iLog(aLog)
    ,iDisposed(false)
    ,iParent(NULL)
    ,iSender(NULL)
    ,iVectorSenders(new vector<ITopology5Group*>())
{
    iWatchableSource = new Watchable<ITopology5Source*>(iNetwork, Brn("source"), iCurrentSource.get());

    iSenders = new Watchable<vector<ITopology5Group*>*>(iNetwork, Brn("senders"), iVectorSenders);

    for(TUint i=0; i<aSources.size(); i++)
    {
        Topology5Source* source = new Topology5Source(aNetwork, *this, aSources[i]);
        iSources.push_back(source);
    }

    iGroup->SourceIndex().AddWatcher(*this);

    if (Ascii::Contains(iGroup->Attributes(), Brn("Sender")))
    {
        iGroup->Device().Create(MakeFunctorGeneric(*this, &Topology5Group::CreateCallback), eProxySender);
    }

}


void Topology5Group::CreateCallback(void* aArgs)
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


void Topology5Group::Dispose()
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


Brn Topology5Group::Name()
{
    return iName;
}


IDevice& Topology5Group::Device()
{
    return iGroup->Device();
}


Brn Topology5Group::Room()
{
    return iRoom;
}


Brn Topology5Group::ModelName()
{
    return iGroup->ModelName();
}


Brn Topology5Group::ManufacturerName()
{
    return iGroup->ManufacturerName();
}


Brn Topology5Group::ProductId()
{
    return iGroup->ProductId();
}


IWatchable<ITopology3Sender*>& Topology5Group::Sender()
{
    return iGroup->Sender();
}

IWatchable<ITopology5Source*>& Topology5Group::Source()
{
    return *iWatchableSource;
}


ITopology3Group& Topology5Group::Group()
{
    return(*iGroup);
}


void Topology5Group::EvaluateSources()
{

    TBool hasInfo = Ascii::Contains(iGroup->Attributes() ,Brn("Info"));
    TBool hasTime = (Ascii::Contains(iGroup->Attributes() ,Brn("Time")) && hasInfo);

    // get list of all volume groups
    iCurrentVolumes = unique_ptr<vector<ITopology5Group*>>(new vector<ITopology5Group*>());


    Topology5Group* group = this;

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
        Topology5Source* s = iSources[i];

        TBool expanded = false;

        for(TUint i=0; i<iChildren.size(); i++)
        {
            Topology5Group* g = iChildren[i];

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


    ITopology5Source* source = EvaluateSource();
    iWatchableSource->Update(source);
}


std::vector<ITopology5Source*>& Topology5Group::Sources()
{
    return iVisibleSources;
}


void Topology5Group::EvaluateSenders()
{
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology5Group* g = iChildren[i];
        g->EvaluateSenders();
    }

    vector<ITopology5Group*>* vectorSenders = new vector<ITopology5Group*>();

    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology5Group* g = iChildren[i];

        vector<ITopology5Group*> v(*(g->Senders().Value()));
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


IWatchable<std::vector<ITopology5Group*>*>& Topology5Group::Senders()
{
    return(*iSenders);
}


Topology5Group* Topology5Group::Parent()
{
    return iParent;
}


TBool Topology5Group::AddIfIsChild(Topology5Group& aGroup)
{
    for(TUint i=0; i<iSources.size(); i++)
    {
        Topology5Source* s = iSources[i];
        if (aGroup.Name() == s->Name())
        {
            aGroup.SetParent(*this, s->Index());
            iChildren.push_back(&aGroup);
            return true;
        }
    }

    return false;
}


void Topology5Group::SetParent(Topology5Group& aGroup)
{
    iParent = &aGroup;
}


void Topology5Group::SetParent(Topology5Group& aGroup, TUint aIndex)
{
    SetParent(aGroup);
    iParentSourceIndex = aIndex;
}


void Topology5Group::ItemOpen(const Brx& /*aId*/, TUint aValue)
{
    iSourceIndex = aValue;
}


void Topology5Group::ItemUpdate(const Brx& /*aId*/, TUint aValue, TUint /*aPrevious*/)
{
   iSourceIndex = aValue;
   EvaluateSourceFromChild();
}


void Topology5Group::ItemClose(const Brx& /*aId*/, TUint /*aValue*/)
{
}


ITopology5Source* Topology5Group::EvaluateSource()
{
/*
    if(iSources.Count <= iSourceIndex)
    {
        iLog.Write("EvaluateSource of {0}, iSources.Count={1}, iSourceIndex={2}", iName, iSources.Count(), iSourceIndex);
    }
*/
    // set the source for this group
    Topology5Source* source = iSources[iSourceIndex];

    // check if the group's source is expanded by a child's group's sources
    for(TUint i=0; i<iChildren.size(); i++)
    {
        Topology5Group* g = iChildren[i];
        if (g->Name() == source->Name())
        {
            return g->EvaluateSource();
        }
    }

    return(source);
}


void Topology5Group::EvaluateSourceFromChild()
{
    if (iParent != NULL)
    {
        iParent->EvaluateSourceFromChild();
    }

    iWatchableSource->Update(EvaluateSource());
}


void Topology5Group::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology5Group::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iHasSender = (aValue == Brn("Enabled"));
    EvaluateSendersFromChild();
}


void Topology5Group::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
}


void Topology5Group::EvaluateSendersFromChild()
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


void Topology5Group::SetSourceIndex(TUint aValue)
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

Topology5GroupWatcher::Topology5GroupWatcher(Topology5Room& aRoom, ITopology3Group& aGroup)
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

void Topology5GroupWatcher::Dispose()
{
    iGroup.Name().RemoveWatcher(*this);
    vector<Watchable<ITopology2Source*>*> s = iGroup.Sources();

    for(TUint i=0; i<s.size(); i++)
    {
        s[i]->RemoveWatcher(*this);
    }
}


Brn Topology5GroupWatcher::Room()
{
    return iRoomName;
}


Brn Topology5GroupWatcher::Name()
{
    return iName;
}


std::vector<ITopology2Source*>& Topology5GroupWatcher::Sources()
{
    return iSources;
}


void Topology5GroupWatcher::ItemOpen(const Brx& /*aId*/, Brn aValue)
{
   iName.Set(aValue);
}


void Topology5GroupWatcher::ItemUpdate(const Brx& /*aId*/, Brn aValue, Brn /*aPrevious*/)
{
    iName.Set(aValue);
    iRoom.CreateTree();
}

void Topology5GroupWatcher::ItemClose(const Brx& /*aId*/, Brn /*aValue*/)
{
}


void Topology5GroupWatcher::ItemOpen(const Brx& /*aId*/, ITopology2Source* aValue)
{
    iSources.push_back(aValue);
}


void Topology5GroupWatcher::ItemUpdate(const Brx& /*aId*/, ITopology2Source* aValue, ITopology2Source* aPrevious)
{
    vector<ITopology2Source*>::iterator it = find(iSources.begin(), iSources.end(), aPrevious);
    if (it!=iSources.end())
    {
        iSources[it-iSources.begin()] = aValue;
    }

    iRoom.CreateTree();
}

void Topology5GroupWatcher::ItemClose(const Brx& /*aId*/, ITopology2Source* /*aValue*/)
{
}


///////////////////////////////////////////////////////////////////


Topology5Room::Topology5Room(INetwork& aNetwork, ITopology4Room& aRoom, ILog& aLog)
    :iNetwork(aNetwork)
    ,iRoom(aRoom)
    ,iLog(aLog)
    ,iName(iRoom.Name())
    ,iStandbyCount(0)
    ,iStandby(eOff)
    ,iCurrentRoots(unique_ptr<vector<ITopology5Root*>>(new vector<ITopology5Root*>()))
    ,iCurrentSources(unique_ptr<vector<ITopology5Source*>>(new vector<ITopology5Source*>()))
    ,iCurrentRegistrations(unique_ptr<vector<ITopology5Registration*>>(new vector<ITopology5Registration*>()))
    ,iWatchableStandby(new Watchable<EStandby>(iNetwork, Brn("standby"), eOff))
    ,iWatchableRoots(new Watchable<vector<ITopology5Root*>*>(iNetwork, Brn("roots"), iCurrentRoots.get()))
    ,iWatchableSources(new Watchable<vector<ITopology5Source*>*>(iNetwork, Brn("sources"), iCurrentSources.get()))
    ,iWatchableRegistrations(new Watchable<vector<ITopology5Registration*>*>(iNetwork, Brn("registration"), iCurrentRegistrations.get()))
{
    iRoom.Groups().AddWatcher(*this);
}


void Topology5Room::Dispose()
{
    iRoom.Groups().RemoveWatcher(*this);
    //iRoom = null;

    map<ITopology3Group*, Topology5GroupWatcher*>::iterator it;

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


Brn Topology5Room::Name()
{
    return iName;
}


IWatchable<EStandby>& Topology5Room::Standby()
{
    return *iWatchableStandby;
}


IWatchable<std::vector<ITopology5Root*>*>& Topology5Room::Roots()
{
    return *iWatchableRoots;
}


IWatchable<std::vector<ITopology5Source*>*>& Topology5Room::Sources()
{
    return *iWatchableSources;
}


IWatchable<std::vector<ITopology5Registration*>*>& Topology5Room::Registrations()
{
    return *iWatchableRegistrations;
}


void Topology5Room::SetStandby(TBool aValue)
{
    iRoom.SetStandby(aValue);
}


void Topology5Room::UnorderedOpen()
{
}


void Topology5Room::UnorderedInitialised()
{
}


void Topology5Room::UnorderedClose()
{
}


void Topology5Room::UnorderedAdd(ITopology3Group* aItem)
{
    iGroupWatcherLookup[aItem] = new Topology5GroupWatcher(*this, *aItem);
    aItem->Standby().AddWatcher(*this);
    CreateTree();
}


void Topology5Room::UnorderedRemove(ITopology3Group* aItem)
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


void Topology5Room::CreateTree()
{
    vector<ITopology5Registration*>* registrations = new vector<ITopology5Registration*>();
    vector<Topology5Group*> oldGroups(iGroups);

    iGroups.clear();
    iRoots.clear();

    map<ITopology3Group*, Topology5GroupWatcher*>::iterator it;
    for(it=iGroupWatcherLookup.begin(); it!=iGroupWatcherLookup.end(); it++)
    {
        Topology5Group* group = new Topology5Group(iNetwork, it->second->Room(), it->second->Name(), *it->first, it->second->Sources(), iLog);
        InsertIntoTree(*group);

        if (!group->ProductId().Equals(Brx::Empty()))
        {
            registrations->push_back(group);
        }
    }

    vector<ITopology5Root*>* roots = new vector<ITopology5Root*>();
    vector<ITopology5Source*>* sources = new vector<ITopology5Source*>();

    for(TUint i=0; i<iRoots.size(); i++)
    {
        Topology5Group* group = iRoots[i];

        group->EvaluateSources();
        group->EvaluateSenders();

        auto s = group->Sources();
        sources->insert(sources->end(), s.begin(), s.end());

        roots->push_back(group);
    }

    iCurrentRoots = unique_ptr<std::vector<ITopology5Root*>>(roots);
    iCurrentRegistrations = unique_ptr<std::vector<ITopology5Registration*>>(registrations);
    iCurrentSources = unique_ptr<std::vector<ITopology5Source*>>(sources);

    iWatchableRoots->Update(roots);
    iWatchableSources->Update(sources);
    iWatchableRegistrations->Update(registrations);

    for(TUint i=0; i<oldGroups.size(); i++)
    {
        oldGroups[i]->Dispose();
        delete oldGroups[i];
    }
}


void Topology5Room::InsertIntoTree(Topology5Group& aGroup)
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


void Topology5Room::ItemOpen(const Brx& /*aId*/, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount++;
    }

    EvaluateStandby();
}


void Topology5Room::ItemUpdate(const Brx& /*aId*/, TBool aValue, TBool /*aPrevious*/)
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


void Topology5Room::ItemClose(const Brx& /*aId*/, TBool aValue)
{
    if (aValue)
    {
        iStandbyCount--;
    }

    EvaluateStandby(iGroupWatcherLookup.size() == 0);
}


void Topology5Room::EvaluateStandby()
{
    EvaluateStandby(false);
}


void Topology5Room::EvaluateStandby(TBool aLastGroup)
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

Topology5::Topology5(ITopology4* aTopology4, ILog& aLog)
    :iTopology4(aTopology4)
    ,iLog(aLog)
    ,iNetwork(aTopology4->Network())
    ,iDisposeHandler(new DisposeHandler())
    ,iRooms(new WatchableUnordered<ITopology5Room*>(iNetwork))
{
    iNetwork.Schedule(MakeFunctorGeneric(*this, &Topology5::ScheduleCallback), NULL);
}


void Topology5::ScheduleCallback(void*)
{
     iTopology4->Rooms().AddWatcher(*this);
}


void Topology5::Dispose()
{
    iDisposeHandler->Dispose();
    delete iDisposeHandler;
    iNetwork.Execute(MakeFunctorGeneric(*this, &Topology5::DisposeCallback), NULL);
    iRoomLookup.clear();
    iRooms->Dispose();
    delete iRooms;
    iRooms = NULL;

    iTopology4->Dispose();
    delete iTopology4;
    iTopology4 = NULL;
}


void Topology5::DisposeCallback(void*)
{
    iTopology4->Rooms().RemoveWatcher(*this);

    map<ITopology4Room*, Topology5Room*>::iterator it;

    for(it=iRoomLookup.begin(); it!=iRoomLookup.end(); it++)
    {
        it->second->Dispose();
        delete it->second;
    }

    iRoomLookup.clear();
}


IWatchableUnordered<ITopology5Room*>& Topology5::Rooms()
{
    DisposeLock lock(*iDisposeHandler);
    return(*iRooms);
}


INetwork& Topology5::Network()
{
    DisposeLock lock(*iDisposeHandler);
    return iNetwork;
}


void Topology5::UnorderedOpen()
{
}

void Topology5::UnorderedInitialised()
{
}


void Topology5::UnorderedClose()
{
}


void Topology5::UnorderedAdd(ITopology4Room* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology5::UnorderedAddCallback), aItem);
}


void Topology5::UnorderedAddCallback(void* aT3Room)
{
    ITopology4Room* t3Room = (ITopology4Room*)aT3Room;
    Topology5Room* t4Room = new Topology5Room(iNetwork, *t3Room, iLog);
    iRooms->Add(t4Room);
    iRoomLookup[t3Room] = t4Room;
}


void Topology5::UnorderedRemove(ITopology4Room* aItem)
{
    iDisposeHandler->WhenNotDisposed(MakeFunctorGeneric(*this, &Topology5::UnorderedRemoveCallback), aItem);
}


void Topology5::UnorderedRemoveCallback(void* aT3Room)
{
    // schedule notification of L4 room removal
    ITopology4Room* t3Room = (ITopology4Room*)aT3Room;
    Topology5Room* t4Room = iRoomLookup[t3Room];
    iRooms->Remove(t4Room);
    iRoomLookup.erase(t3Room);
    t4Room->Dispose();
    delete t4Room;
}






